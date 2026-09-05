// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsPreferredVideoDecoderFactory.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "api/video_codecs/video_decoder_software_fallback_wrapper.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "VideoCodecTimingTelemetry.h"

namespace remote {
namespace {

class FrameTimedVideoDecoder final
    : public webrtc::VideoDecoder,
      public webrtc::DecodedImageCallback {
public:
    explicit FrameTimedVideoDecoder(
        std::unique_ptr<webrtc::VideoDecoder> decoder)
        : decoder_(std::move(decoder)),
          timingInstanceId_(
              VideoCodecTimingRegistry::Instance().Register(
                  VideoCodecTimingDirection::kDecode))
    {}

    ~FrameTimedVideoDecoder() override
    {
        VideoCodecTimingRegistry::Instance().Unregister(
            timingInstanceId_);
    }

    bool Configure(const Settings& settings) override
    {
        ClearPending();
        return decoder_->Configure(settings);
    }

    int32_t Decode(
        const webrtc::EncodedImage& inputImage,
        int64_t renderTimeMs) override
    {
        const std::uint32_t timestamp = inputImage.RtpTimestamp();
        {
            std::lock_guard lock(mutex_);
            if (pendingStartsUs_.size() >= 512) {
                pendingStartsUs_.erase(pendingStartsUs_.begin());
            }
            pendingStartsUs_[timestamp] =
                VideoCodecTimingRegistry::SteadyNowUs();
        }
        const int32_t result =
            decoder_->Decode(inputImage, renderTimeMs);
        if (result != WEBRTC_VIDEO_CODEC_OK &&
            result != WEBRTC_VIDEO_CODEC_NO_OUTPUT) {
            std::lock_guard lock(mutex_);
            pendingStartsUs_.erase(timestamp);
            VideoCodecTimingRegistry::Instance().RecordDropped(
                timingInstanceId_);
        }
        return result;
    }

    int32_t RegisterDecodeCompleteCallback(
        webrtc::DecodedImageCallback* callback) override
    {
        {
            std::lock_guard lock(mutex_);
            callback_ = callback;
        }
        return decoder_->RegisterDecodeCompleteCallback(
            callback ? this : nullptr);
    }

    int32_t Release() override
    {
        ClearPending();
        {
            std::lock_guard lock(mutex_);
            callback_ = nullptr;
        }
        return decoder_->Release();
    }

    DecoderInfo GetDecoderInfo() const override
    {
        auto info = decoder_->GetDecoderInfo();
        info.implementation_name =
            VideoCodecTimingRegistry::TaggedImplementation(
                std::move(info.implementation_name),
                timingInstanceId_);
        return info;
    }

    const char* ImplementationName() const override
    {
        implementationName_ =
            GetDecoderInfo().implementation_name;
        return implementationName_.c_str();
    }

    int32_t Decoded(webrtc::VideoFrame& decodedImage) override
    {
        RecordCompleted(decodedImage, std::nullopt);
        webrtc::DecodedImageCallback* callback = Callback();
        return callback
                   ? callback->Decoded(decodedImage)
                   : WEBRTC_VIDEO_CODEC_OK;
    }

    int32_t Decoded(
        webrtc::VideoFrame& decodedImage,
        int64_t decodeTimeMs) override
    {
        RecordCompleted(decodedImage, std::nullopt);
        webrtc::DecodedImageCallback* callback = Callback();
        return callback
                   ? callback->Decoded(decodedImage, decodeTimeMs)
                   : WEBRTC_VIDEO_CODEC_OK;
    }

    void Decoded(
        webrtc::VideoFrame& decodedImage,
        std::optional<int32_t> decodeTimeMs,
        std::optional<uint8_t> qp) override
    {
        RecordCompleted(
            decodedImage,
            qp ? std::make_optional<std::int32_t>(*qp)
               : std::nullopt);
        if (webrtc::DecodedImageCallback* callback = Callback()) {
            callback->Decoded(decodedImage, decodeTimeMs, qp);
        }
    }

private:
    webrtc::DecodedImageCallback* Callback() const
    {
        std::lock_guard lock(mutex_);
        return callback_;
    }

    void ClearPending()
    {
        std::lock_guard lock(mutex_);
        pendingStartsUs_.clear();
    }

    void RecordCompleted(
        const webrtc::VideoFrame& decodedImage,
        std::optional<std::int32_t> qp)
    {
        const std::uint32_t timestamp =
            decodedImage.rtp_timestamp();
        std::int64_t startUs = 0;
        {
            std::lock_guard lock(mutex_);
            const auto found = pendingStartsUs_.find(timestamp);
            if (found == pendingStartsUs_.end()) {
                return;
            }
            startUs = found->second;
            pendingStartsUs_.erase(found);
        }
        const std::int64_t nowUs =
            VideoCodecTimingRegistry::SteadyNowUs();
        if (nowUs < startUs) {
            return;
        }
        VideoCodecTimingRegistry::Instance().RecordCompleted(
            timingInstanceId_,
            VideoCodecTimingRegistry::UntaggedImplementation(
                decoder_->GetDecoderInfo().implementation_name),
            timestamp,
            static_cast<std::uint64_t>(nowUs - startUs),
            static_cast<std::uint32_t>(decodedImage.width()),
            static_cast<std::uint32_t>(decodedImage.height()),
            0,
            qp);
    }

    std::unique_ptr<webrtc::VideoDecoder> decoder_;
    const std::uint64_t timingInstanceId_;
    mutable std::mutex mutex_;
    std::unordered_map<std::uint32_t, std::int64_t> pendingStartsUs_;
    webrtc::DecodedImageCallback* callback_ = nullptr;
    mutable std::string implementationName_;
};

void AppendUniqueFormats(
    std::vector<webrtc::SdpVideoFormat>& destination,
    const std::vector<webrtc::SdpVideoFormat>& source)
{
    for (const auto& format : source) {
        if (!format.IsCodecInList(destination)) {
            destination.push_back(format);
        }
    }
}

}  // namespace

class WindowsPreferredVideoDecoderFactoryState final {
public:
    WindowsPreferredVideoDecoderFactoryState(
        std::unique_ptr<webrtc::VideoDecoderFactory> softwareFactory,
        std::unique_ptr<webrtc::VideoDecoderFactory> hardwareFactory,
        bool preferSoftware)
        : softwareFactory_(std::move(softwareFactory)),
          hardwareFactory_(std::move(hardwareFactory)),
          preferSoftware_(preferSoftware)
    {}

    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const
    {
        std::lock_guard lock(mutex_);
        std::vector<webrtc::SdpVideoFormat> formats;
        if (preferSoftware_ && softwareFactory_) {
            AppendUniqueFormats(
                formats, softwareFactory_->GetSupportedFormats());
        }
        if (hardwareFactory_) {
            AppendUniqueFormats(
                formats, hardwareFactory_->GetSupportedFormats());
        }
        if (!preferSoftware_ && softwareFactory_) {
            AppendUniqueFormats(
                formats, softwareFactory_->GetSupportedFormats());
        }
        return formats;
    }

    webrtc::VideoDecoderFactory::CodecSupport QueryCodecSupport(
        const webrtc::SdpVideoFormat& format,
        bool referenceScaling,
        std::optional<webrtc::Resolution> resolution) const
    {
        std::lock_guard lock(mutex_);
        if (preferSoftware_ && softwareFactory_) {
            const auto support = softwareFactory_->QueryCodecSupport(
                format, referenceScaling, resolution);
            if (support.is_supported) {
                return support;
            }
        }
        if (hardwareFactory_) {
            const auto support = hardwareFactory_->QueryCodecSupport(
                format, referenceScaling, resolution);
            if (support.is_supported) {
                return support;
            }
        }
        return !preferSoftware_ && softwareFactory_
                   ? softwareFactory_->QueryCodecSupport(
                         format, referenceScaling, resolution)
                   : webrtc::VideoDecoderFactory::CodecSupport{};
    }

    std::unique_ptr<webrtc::VideoDecoder> CreateSelected(
        const webrtc::Environment& environment,
        const webrtc::SdpVideoFormat& format,
        std::uint64_t* revision) const
    {
        std::lock_guard lock(mutex_);
        std::unique_ptr<webrtc::VideoDecoder> hardware;
        if (hardwareFactory_) {
            hardware = hardwareFactory_->Create(environment, format);
        }
        std::unique_ptr<webrtc::VideoDecoder> software;
        if (softwareFactory_) {
            software = softwareFactory_->Create(environment, format);
        }
        if (revision) {
            *revision = revision_;
        }
        if (preferSoftware_ && software) {
            return software;
        }
        if (hardware && software) {
            return webrtc::CreateVideoDecoderSoftwareFallbackWrapper(
                environment, std::move(software), std::move(hardware));
        }
        return hardware ? std::move(hardware) : std::move(software);
    }

    std::uint64_t Revision() const
    {
        std::lock_guard lock(mutex_);
        return revision_;
    }

    bool HasHardwareDecoderFor(
        const webrtc::SdpVideoFormat& format) const
    {
        std::lock_guard lock(mutex_);
        return hardwareFactory_ && format.IsCodecInList(
            hardwareFactory_->GetSupportedFormats());
    }

    bool HasSoftwareDecoderFor(
        const webrtc::SdpVideoFormat& format) const
    {
        std::lock_guard lock(mutex_);
        return softwareFactory_ && format.IsCodecInList(
            softwareFactory_->GetSupportedFormats());
    }

    void SetPreferSoftware(bool preferSoftware)
    {
        std::lock_guard lock(mutex_);
        if (preferSoftware_ != preferSoftware) {
            preferSoftware_ = preferSoftware;
            ++revision_;
        }
    }

    void NotifyConfigurationChanged()
    {
        std::lock_guard lock(mutex_);
        ++revision_;
    }

private:
    mutable std::mutex mutex_;
    std::unique_ptr<webrtc::VideoDecoderFactory> softwareFactory_;
    std::unique_ptr<webrtc::VideoDecoderFactory> hardwareFactory_;
    bool preferSoftware_ = false;
    std::uint64_t revision_ = 1;
};

namespace {

class ReconfigurableVideoDecoder final : public webrtc::VideoDecoder {
public:
    ReconfigurableVideoDecoder(
        std::shared_ptr<WindowsPreferredVideoDecoderFactoryState> state,
        webrtc::Environment environment,
        webrtc::SdpVideoFormat format)
        : state_(std::move(state)),
          environment_(std::move(environment)),
          format_(std::move(format)),
          decoder_(state_->CreateSelected(
              environment_, format_, &revision_))
    {}

    bool IsValid() const noexcept { return decoder_ != nullptr; }

    bool Configure(const Settings& settings) override
    {
        if (!EnsureCurrent(false)) {
            return false;
        }
        settings_ = settings;
        configured_ = decoder_->Configure(settings);
        return configured_;
    }

    int32_t Decode(
        const webrtc::EncodedImage& inputImage,
        int64_t renderTimeMs) override
    {
        return EnsureCurrent(true)
                   ? decoder_->Decode(inputImage, renderTimeMs)
                   : WEBRTC_VIDEO_CODEC_ERROR;
    }

    int32_t RegisterDecodeCompleteCallback(
        webrtc::DecodedImageCallback* callback) override
    {
        callback_ = callback;
        return decoder_
                   ? decoder_->RegisterDecodeCompleteCallback(callback)
                   : WEBRTC_VIDEO_CODEC_ERROR;
    }

    int32_t Release() override
    {
        configured_ = false;
        callback_ = nullptr;
        return decoder_ ? decoder_->Release() : WEBRTC_VIDEO_CODEC_OK;
    }

    DecoderInfo GetDecoderInfo() const override
    {
        return decoder_ ? decoder_->GetDecoderInfo() : DecoderInfo{};
    }

    const char* ImplementationName() const override
    {
        implementationName_ = GetDecoderInfo().implementation_name;
        return implementationName_.c_str();
    }

private:
    bool EnsureCurrent(bool configure)
    {
        if (!state_) {
            return false;
        }
        const std::uint64_t currentRevision = state_->Revision();
        if (decoder_ && currentRevision == revision_) {
            return true;
        }
        std::uint64_t newRevision = 0;
        auto replacement = state_->CreateSelected(
            environment_, format_, &newRevision);
        if (!replacement) {
            return false;
        }
        if (configure && configured_ && settings_) {
            if (!replacement->Configure(*settings_)) {
                replacement->Release();
                return false;
            }
            if (callback_ &&
                replacement->RegisterDecodeCompleteCallback(callback_) !=
                    WEBRTC_VIDEO_CODEC_OK) {
                replacement->Release();
                return false;
            }
        }
        if (decoder_) {
            decoder_->Release();
        }
        decoder_ = std::move(replacement);
        revision_ = newRevision;
        return true;
    }

    std::shared_ptr<WindowsPreferredVideoDecoderFactoryState> state_;
    webrtc::Environment environment_;
    webrtc::SdpVideoFormat format_;
    std::unique_ptr<webrtc::VideoDecoder> decoder_;
    std::uint64_t revision_ = 0;
    webrtc::DecodedImageCallback* callback_ = nullptr;
    bool configured_ = false;
    std::optional<Settings> settings_;
    mutable std::string implementationName_;
};

}  // namespace

WindowsPreferredVideoDecoderFactory::WindowsPreferredVideoDecoderFactory(
    std::unique_ptr<webrtc::VideoDecoderFactory> softwareFactory,
    std::unique_ptr<webrtc::VideoDecoderFactory> hardwareFactory,
    bool preferSoftware)
    : state_(std::make_shared<WindowsPreferredVideoDecoderFactoryState>(
          std::move(softwareFactory), std::move(hardwareFactory),
          preferSoftware))
{}

WindowsPreferredVideoDecoderFactory::~WindowsPreferredVideoDecoderFactory() =
    default;

std::vector<webrtc::SdpVideoFormat>
WindowsPreferredVideoDecoderFactory::GetSupportedFormats() const
{
    return state_->GetSupportedFormats();
}

webrtc::VideoDecoderFactory::CodecSupport
WindowsPreferredVideoDecoderFactory::QueryCodecSupport(
    const webrtc::SdpVideoFormat& format,
    bool referenceScaling,
    std::optional<webrtc::Resolution> resolution) const
{
    return state_->QueryCodecSupport(
        format, referenceScaling, resolution);
}

std::unique_ptr<webrtc::VideoDecoder>
WindowsPreferredVideoDecoderFactory::Create(
    const webrtc::Environment& environment,
    const webrtc::SdpVideoFormat& format)
{
    auto selected = std::make_unique<ReconfigurableVideoDecoder>(
        state_, environment, format);
    return selected->IsValid()
               ? std::make_unique<FrameTimedVideoDecoder>(
                     std::move(selected))
               : nullptr;
}

void WindowsPreferredVideoDecoderFactory::SetPreferSoftware(
    bool preferSoftware) noexcept
{
    state_->SetPreferSoftware(preferSoftware);
}

void WindowsPreferredVideoDecoderFactory::NotifyConfigurationChanged()
    noexcept
{
    state_->NotifyConfigurationChanged();
}

bool WindowsPreferredVideoDecoderFactory::HasHardwareDecoderFor(
    const webrtc::SdpVideoFormat& format) const
{
    return state_->HasHardwareDecoderFor(format);
}

bool WindowsPreferredVideoDecoderFactory::HasSoftwareDecoderFor(
    const webrtc::SdpVideoFormat& format) const
{
    return state_->HasSoftwareDecoderFor(format);
}

}  // namespace remote
