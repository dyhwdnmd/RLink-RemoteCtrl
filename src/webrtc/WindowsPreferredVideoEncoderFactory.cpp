// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsPreferredVideoEncoderFactory.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include "api/environment/environment_factory.h"
#include "api/video_codecs/video_encoder_software_fallback_wrapper.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "src/core/ScreenStreamPolicy.h"
#include "VideoCodecTimingTelemetry.h"

namespace remote {
namespace {

// libwebrtc's generic software-fallback wrapper may be configured through
// WebRTC-Video-EncoderFallbackSettings to force software below a pixel-count
// threshold. That policy is useful for some camera/mobile products, but it is
// wrong for a Windows remote desktop: changing 1080p -> 720p must not replace
// a healthy NVENC/QSV/AMF encoder with libx264. Preserve every other field
// trial and disable only the resolution-based forced fallback for the wrapper
// we construct here. Real InitEncode/Encode failures still use the normal
// software fallback path.
class HardwareFailureOnlyFieldTrials final
    : public webrtc::FieldTrialsView {
public:
    explicit HardwareFailureOnlyFieldTrials(
        const webrtc::FieldTrialsView& base)
        : base_(base)
    {}

    std::string Lookup(absl::string_view key) const override
    {
        if (key == "WebRTC-Video-EncoderFallbackSettings") {
            return {};
        }
        return base_.Lookup(key);
    }

private:
    const webrtc::FieldTrialsView& base_;
};

// Resolution changes make VideoStreamEncoder release and immediately
// initialize the same hardware encoder wrapper with the new dimensions.
// Some Windows hardware drivers finish destroying the previous session a few
// milliseconds after Release(), so the first NVENC/QSV/AMF initialization may
// fail even though an immediate retry succeeds. libwebrtc's stock fallback
// wrapper permanently selects software after that first failure. Absorb only
// this short transient here; repeated failures are still returned to the
// stock wrapper and therefore retain the normal software safety net.
class TransientFailureRetryVideoEncoder final
    : public webrtc::VideoEncoder {
public:
    explicit TransientFailureRetryVideoEncoder(
        std::unique_ptr<webrtc::VideoEncoder> encoder)
        : encoder_(std::move(encoder))
    {}

    void SetFecControllerOverride(
        webrtc::FecControllerOverride* fecControllerOverride) override
    {
        fecControllerOverride_ = fecControllerOverride;
        encoder_->SetFecControllerOverride(fecControllerOverride);
    }

    int InitEncode(
        const webrtc::VideoCodec* codecSettings,
        const Settings& settings) override
    {
        if (!codecSettings) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        codecSettings_ = *codecSettings;
        settings_ = settings;
        rates_.reset();
        initialized_ = false;
        const int result = InitializeWithRetry(/*attempts=*/3);
        initialized_ = result == WEBRTC_VIDEO_CODEC_OK;
        return result;
    }

    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override
    {
        callback_ = callback;
        return encoder_->RegisterEncodeCompleteCallback(callback);
    }

    int32_t Release() override
    {
        initialized_ = false;
        callback_ = nullptr;
        rates_.reset();
        return encoder_->Release();
    }

    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frameTypes) override
    {
        int32_t result = encoder_->Encode(frame, frameTypes);
        if (result != WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE ||
            !initialized_ || !codecSettings_ || !settings_) {
            return result;
        }

        // A resolution reconfiguration can also expose the delayed driver
        // teardown on the first submitted frame. Recreate the hardware path
        // once before allowing the outer wrapper to switch to software.
        encoder_->Release();
        initialized_ = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        result = InitializeWithRetry(/*attempts=*/2);
        if (result != WEBRTC_VIDEO_CODEC_OK) {
            return result;
        }
        initialized_ = true;
        PrimeEncoder();
        return encoder_->Encode(frame, frameTypes);
    }

    void OnPacketLossRateUpdate(float packetLossRate) override
    {
        packetLossRate_ = packetLossRate;
        encoder_->OnPacketLossRateUpdate(packetLossRate);
    }

    void OnRttUpdate(int64_t rttMs) override
    {
        rttMs_ = rttMs;
        encoder_->OnRttUpdate(rttMs);
    }

    void OnLossNotification(
        const LossNotification& lossNotification) override
    {
        lossNotification_ = lossNotification;
        encoder_->OnLossNotification(lossNotification);
    }

    void SetRates(const RateControlParameters& parameters) override
    {
        rates_ = parameters;
        encoder_->SetRates(parameters);
    }

    EncoderInfo GetEncoderInfo() const override
    {
        return encoder_->GetEncoderInfo();
    }

private:
    int InitializeWithRetry(int attempts)
    {
        int result = WEBRTC_VIDEO_CODEC_ERROR;
        for (int attempt = 0; attempt < attempts; ++attempt) {
            if (attempt > 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(20 * attempt));
            }
            result = encoder_->InitEncode(
                &*codecSettings_, *settings_);
            if (result == WEBRTC_VIDEO_CODEC_OK ||
                result ==
                    WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED) {
                return result;
            }
            encoder_->Release();
        }
        return result;
    }

    void PrimeEncoder()
    {
        encoder_->SetFecControllerOverride(fecControllerOverride_);
        if (callback_) {
            encoder_->RegisterEncodeCompleteCallback(callback_);
        }
        if (rates_) {
            encoder_->SetRates(*rates_);
        }
        if (rttMs_) {
            encoder_->OnRttUpdate(*rttMs_);
        }
        if (packetLossRate_) {
            encoder_->OnPacketLossRateUpdate(*packetLossRate_);
        }
        if (lossNotification_) {
            encoder_->OnLossNotification(*lossNotification_);
        }
    }

    std::unique_ptr<webrtc::VideoEncoder> encoder_;
    webrtc::FecControllerOverride* fecControllerOverride_ = nullptr;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    bool initialized_ = false;
    std::optional<webrtc::VideoCodec> codecSettings_;
    std::optional<Settings> settings_;
    std::optional<RateControlParameters> rates_;
    std::optional<float> packetLossRate_;
    std::optional<int64_t> rttMs_;
    std::optional<LossNotification> lossNotification_;
};

class RuntimeTrackedSoftwareEncoder final
    : public webrtc::VideoEncoder,
      public webrtc::EncodedImageCallback {
public:
    RuntimeTrackedSoftwareEncoder(
        std::unique_ptr<webrtc::VideoEncoder> encoder,
        std::shared_ptr<VideoEncoderRuntimeState> runtimeState)
        : encoder_(std::move(encoder))
        , runtimeState_(std::move(runtimeState))
    {}

    ~RuntimeTrackedSoftwareEncoder() override
    {
        Unregister();
    }

    int InitEncode(
        const webrtc::VideoCodec* codecSettings,
        const Settings& settings) override
    {
        Unregister();
        {
            std::lock_guard lock(frameStatsMutex_);
            inputRateWindowStart_ = {};
            outputRateWindowStart_ = {};
            inputRateWindowFrames_ = 0;
            outputRateWindowFrames_ = 0;
            observedInputFrameRate_ = 0;
            observedOutputFrameRate_ = 0;
            totalInputFrames_ = 0;
            totalOutputFrames_ = 0;
            totalDroppedFrames_ = 0;
        }
        const int result =
            encoder_->InitEncode(codecSettings, settings);
        if (result == WEBRTC_VIDEO_CODEC_OK &&
            runtimeState_ && codecSettings) {
            const auto info = encoder_->GetEncoderInfo();
            std::uint32_t effectiveStartBitrateBps =
                codecSettings->startBitrate * 1000;
            if (codecSettings->mode ==
                    webrtc::VideoCodecMode::kScreensharing &&
                info.implementation_name.starts_with(
                    "FFmpeg/libx264")) {
                const auto screenPolicy = ResolveScreenStreamPolicy(
                    codecSettings->width,
                    codecSettings->height,
                    {codecSettings->width,
                     codecSettings->height,
                     (std::max<std::uint32_t>)(
                         codecSettings->maxFramerate, 1)});
                effectiveStartBitrateBps = (std::min)(
                    (std::max)(effectiveStartBitrateBps,
                               screenPolicy.startBitrateBps),
                    codecSettings->maxBitrate * 1000);
            }
            runtimeInstanceId_ =
                runtimeState_->RegisterSoftwareEncoder(
                    info.implementation_name);
            runtimeState_->MarkSoftwareInitialized(
                runtimeInstanceId_,
                info.implementation_name,
                codecSettings->width,
                codecSettings->height,
                (std::max<std::uint32_t>)(
                    codecSettings->maxFramerate, 1),
                codecSettings->minBitrate * 1000,
                effectiveStartBitrateBps,
                codecSettings->maxBitrate * 1000);
        }
        return result;
    }

    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override
    {
        {
            std::lock_guard lock(frameStatsMutex_);
            callback_ = callback;
        }
        return encoder_->RegisterEncodeCompleteCallback(
            callback ? this : nullptr);
    }

    int32_t Release() override
    {
        const int32_t result = encoder_->Release();
        {
            std::lock_guard lock(frameStatsMutex_);
            callback_ = nullptr;
        }
        Unregister();
        return result;
    }

    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frameTypes) override
    {
        ReportInputFormat(frame.video_frame_buffer());
        RecordInputFrame();
        const int32_t result = encoder_->Encode(frame, frameTypes);
        if (result != WEBRTC_VIDEO_CODEC_OK) {
            RecordDroppedFrame();
        }
        return result;
    }

    void SetRates(
        const RateControlParameters& parameters) override
    {
        encoder_->SetRates(parameters);
        if (runtimeState_ && runtimeInstanceId_ != 0) {
            runtimeState_->MarkRates(
                runtimeInstanceId_,
                parameters.framerate_fps > 0.0
                    ? static_cast<std::uint32_t>(
                          parameters.framerate_fps)
                    : 0,
                parameters.bitrate.get_sum_bps());
        }
    }

    EncoderInfo GetEncoderInfo() const override
    {
        return encoder_->GetEncoderInfo();
    }

    Result OnEncodedImage(
        const webrtc::EncodedImage& encodedImage,
        const webrtc::CodecSpecificInfo* codecSpecificInfo) override
    {
        RecordOutputFrame();
        webrtc::EncodedImageCallback* callback = nullptr;
        {
            std::lock_guard lock(frameStatsMutex_);
            callback = callback_;
        }
        return callback
                   ? callback->OnEncodedImage(
                         encodedImage, codecSpecificInfo)
                   : Result(Result::ERROR_SEND_FAILED);
    }

    void OnFrameDropped(
        uint32_t rtpTimestamp,
        int spatialId,
        bool isEndOfTemporalUnit) override
    {
        RecordDroppedFrame();
        webrtc::EncodedImageCallback* callback = nullptr;
        {
            std::lock_guard lock(frameStatsMutex_);
            callback = callback_;
        }
        if (callback) {
            callback->OnFrameDropped(
                rtpTimestamp, spatialId, isEndOfTemporalUnit);
        }
    }

private:
    void PublishFramePipeline()
    {
        if (!runtimeState_ || runtimeInstanceId_ == 0) {
            return;
        }
        std::uint32_t inputRate = 0;
        std::uint32_t outputRate = 0;
        std::uint64_t inputFrames = 0;
        std::uint64_t outputFrames = 0;
        std::uint64_t droppedFrames = 0;
        {
            std::lock_guard lock(frameStatsMutex_);
            inputRate = observedInputFrameRate_;
            outputRate = observedOutputFrameRate_;
            inputFrames = totalInputFrames_;
            outputFrames = totalOutputFrames_;
            droppedFrames = totalDroppedFrames_;
        }
        runtimeState_->MarkFramePipeline(
            runtimeInstanceId_, inputRate, outputRate,
            inputFrames, outputFrames, droppedFrames);
    }

    void RecordInputFrame()
    {
        bool publish = false;
        {
            std::lock_guard lock(frameStatsMutex_);
            const auto now = std::chrono::steady_clock::now();
            if (inputRateWindowFrames_ == 0) {
                inputRateWindowStart_ = now;
            }
            ++inputRateWindowFrames_;
            ++totalInputFrames_;
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - inputRateWindowStart_);
            if (elapsed >= std::chrono::milliseconds(1000)) {
                observedInputFrameRate_ =
                    static_cast<std::uint32_t>((std::max<long>)(
                        1L, std::lround(
                            static_cast<double>(
                                inputRateWindowFrames_) *
                            1000.0 /
                            static_cast<double>((std::max<std::int64_t>)(
                                elapsed.count(), 1)))));
                inputRateWindowFrames_ = 0;
                inputRateWindowStart_ = now;
                publish = true;
            }
        }
        if (publish) {
            PublishFramePipeline();
        }
    }

    void RecordOutputFrame()
    {
        bool publish = false;
        {
            std::lock_guard lock(frameStatsMutex_);
            const auto now = std::chrono::steady_clock::now();
            if (outputRateWindowFrames_ == 0) {
                outputRateWindowStart_ = now;
            }
            ++outputRateWindowFrames_;
            ++totalOutputFrames_;
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - outputRateWindowStart_);
            if (elapsed >= std::chrono::milliseconds(1000)) {
                observedOutputFrameRate_ =
                    static_cast<std::uint32_t>((std::max<long>)(
                        1L, std::lround(
                            static_cast<double>(
                                outputRateWindowFrames_) *
                            1000.0 /
                            static_cast<double>((std::max<std::int64_t>)(
                                elapsed.count(), 1)))));
                outputRateWindowFrames_ = 0;
                outputRateWindowStart_ = now;
                publish = true;
            }
        }
        if (publish) {
            PublishFramePipeline();
        }
    }

    void RecordDroppedFrame()
    {
        {
            std::lock_guard lock(frameStatsMutex_);
            ++totalDroppedFrames_;
        }
        PublishFramePipeline();
    }

    void Unregister()
    {
        if (runtimeState_ && runtimeInstanceId_ != 0) {
            runtimeState_->UnregisterEncoder(runtimeInstanceId_);
            runtimeInstanceId_ = 0;
        }
    }

    void ReportInputFormat(
        const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer)
    {
        if (!runtimeState_ || runtimeInstanceId_ == 0 || !buffer) {
            return;
        }
        std::string inputFormat;
        if (buffer->type() ==
                webrtc::VideoFrameBuffer::Type::kNative &&
            buffer->storage_representation() ==
                "D3D11_BGRA_DESKTOP") {
            inputFormat = "D3D11 BGRA desktop texture";
        } else if (buffer->type() ==
                webrtc::VideoFrameBuffer::Type::kNative &&
            buffer->storage_representation() ==
                "CPU_BGRA_DESKTOP") {
            inputFormat = "CPU BGRA desktop";
        } else if (
            buffer->type() ==
            webrtc::VideoFrameBuffer::Type::kI420) {
            inputFormat = "CPU I420";
        } else if (
            buffer->type() ==
            webrtc::VideoFrameBuffer::Type::kNV12) {
            inputFormat = "CPU NV12";
        } else {
            inputFormat = buffer->storage_representation();
        }
        if (inputFormat != lastInputFormat_) {
            lastInputFormat_ = inputFormat;
            runtimeState_->MarkInputFormat(
                runtimeInstanceId_, std::move(inputFormat));
        }
    }

    std::unique_ptr<webrtc::VideoEncoder> encoder_;
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState_;
    std::uint64_t runtimeInstanceId_ = 0;
    std::string lastInputFormat_;
    std::mutex frameStatsMutex_;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    std::chrono::steady_clock::time_point inputRateWindowStart_{};
    std::chrono::steady_clock::time_point outputRateWindowStart_{};
    std::uint32_t inputRateWindowFrames_ = 0;
    std::uint32_t outputRateWindowFrames_ = 0;
    std::uint32_t observedInputFrameRate_ = 0;
    std::uint32_t observedOutputFrameRate_ = 0;
    std::uint64_t totalInputFrames_ = 0;
    std::uint64_t totalOutputFrames_ = 0;
    std::uint64_t totalDroppedFrames_ = 0;
};

class FrameTimedVideoEncoder final
    : public webrtc::VideoEncoder,
      public webrtc::EncodedImageCallback {
public:
    explicit FrameTimedVideoEncoder(
        std::unique_ptr<webrtc::VideoEncoder> encoder)
        : encoder_(std::move(encoder)),
          timingInstanceId_(
              VideoCodecTimingRegistry::Instance().Register(
                  VideoCodecTimingDirection::kEncode))
    {}

    ~FrameTimedVideoEncoder() override
    {
        VideoCodecTimingRegistry::Instance().Unregister(
            timingInstanceId_);
    }

    void SetFecControllerOverride(
        webrtc::FecControllerOverride* fecControllerOverride) override
    {
        encoder_->SetFecControllerOverride(fecControllerOverride);
    }

    int InitEncode(
        const webrtc::VideoCodec* codecSettings,
        const Settings& settings) override
    {
        ClearPending();
        lowLatencyScreenshare_.store(
            codecSettings &&
                codecSettings->mode ==
                    webrtc::VideoCodecMode::kScreensharing,
            std::memory_order_release);
        return encoder_->InitEncode(codecSettings, settings);
    }

    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override
    {
        {
            std::lock_guard lock(mutex_);
            callback_ = callback;
        }
        return encoder_->RegisterEncodeCompleteCallback(
            callback ? this : nullptr);
    }

    int32_t Release() override
    {
        ClearPending();
        lowLatencyScreenshare_.store(false, std::memory_order_release);
        {
            std::lock_guard lock(mutex_);
            callback_ = nullptr;
        }
        return encoder_->Release();
    }

    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frameTypes) override
    {
        const std::uint32_t timestamp = frame.rtp_timestamp();
        {
            std::lock_guard lock(mutex_);
            if (pendingStartsUs_.size() >= 512) {
                pendingStartsUs_.erase(pendingStartsUs_.begin());
            }
            pendingStartsUs_[timestamp] =
                VideoCodecTimingRegistry::SteadyNowUs();
        }
        const int32_t result = encoder_->Encode(frame, frameTypes);
        if (result != WEBRTC_VIDEO_CODEC_OK) {
            std::lock_guard lock(mutex_);
            pendingStartsUs_.erase(timestamp);
        }
        return result;
    }

    void SetRates(
        const RateControlParameters& parameters) override
    {
        encoder_->SetRates(parameters);
    }

    void OnPacketLossRateUpdate(float packetLossRate) override
    {
        encoder_->OnPacketLossRateUpdate(packetLossRate);
    }

    void OnRttUpdate(int64_t rttMs) override
    {
        encoder_->OnRttUpdate(rttMs);
    }

    void OnLossNotification(
        const LossNotification& lossNotification) override
    {
        encoder_->OnLossNotification(lossNotification);
    }

    EncoderInfo GetEncoderInfo() const override
    {
        auto info = encoder_->GetEncoderInfo();
        info.implementation_name =
            VideoCodecTimingRegistry::TaggedImplementation(
                std::move(info.implementation_name),
                timingInstanceId_);
        return info;
    }

    Result OnEncodedImage(
        const webrtc::EncodedImage& encodedImage,
        const webrtc::CodecSpecificInfo* codecSpecificInfo) override
    {
        const std::uint32_t timestamp = encodedImage.RtpTimestamp();
        RecordCompleted(encodedImage);

        webrtc::EncodedImageCallback* callback = nullptr;
        {
            std::lock_guard lock(mutex_);
            callback = callback_;
        }
        if (!callback) {
            return Result(Result::ERROR_SEND_FAILED);
        }

        if (!lowLatencyScreenshare_.load(std::memory_order_acquire)) {
            return callback->OnEncodedImage(
                encodedImage, codecSpecificInfo);
        }

        // Desktop control values freshness over replaying every old frame.
        // Advertising a zero playout-delay range activates libwebrtc's
        // low-latency receive path. If a hardware decoder temporarily blocks,
        // the frame buffer then fast-forwards to the newest decodable frame
        // instead of growing a seconds-long queue. Camera video keeps the
        // normal adaptive jitter buffer because only screenshare encoders set
        // this flag during InitEncode().
        webrtc::EncodedImage lowLatencyImage = encodedImage;
        lowLatencyImage.SetPlayoutDelay(
            webrtc::VideoPlayoutDelay::Minimal());
        return callback->OnEncodedImage(
            lowLatencyImage, codecSpecificInfo);
    }

    void OnFrameDropped(
        uint32_t rtpTimestamp,
        int spatialId,
        bool isEndOfTemporalUnit) override
    {
        if (isEndOfTemporalUnit) {
            std::lock_guard lock(mutex_);
            pendingStartsUs_.erase(rtpTimestamp);
            VideoCodecTimingRegistry::Instance().RecordDropped(
                timingInstanceId_);
        }

        webrtc::EncodedImageCallback* callback = nullptr;
        {
            std::lock_guard lock(mutex_);
            callback = callback_;
        }
        if (callback) {
            callback->OnFrameDropped(
                rtpTimestamp, spatialId, isEndOfTemporalUnit);
        }
    }

private:
    void ClearPending()
    {
        std::lock_guard lock(mutex_);
        pendingStartsUs_.clear();
    }

    void RecordCompleted(const webrtc::EncodedImage& encodedImage)
    {
        const std::uint32_t timestamp =
            encodedImage.RtpTimestamp();
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
                encoder_->GetEncoderInfo().implementation_name),
            timestamp,
            static_cast<std::uint64_t>(nowUs - startUs),
            encodedImage._encodedWidth,
            encodedImage._encodedHeight,
            encodedImage.size(),
            encodedImage.qp_ >= 0
                ? std::make_optional<std::int32_t>(
                      encodedImage.qp_)
                : std::nullopt);
    }

    std::unique_ptr<webrtc::VideoEncoder> encoder_;
    const std::uint64_t timingInstanceId_;
    mutable std::mutex mutex_;
    std::unordered_map<std::uint32_t, std::int64_t> pendingStartsUs_;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    std::atomic_bool lowLatencyScreenshare_{false};
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

class WindowsPreferredVideoEncoderFactoryState final {
public:
    WindowsPreferredVideoEncoderFactoryState(
        std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
        std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory,
        std::shared_ptr<VideoEncoderRuntimeState> runtimeState)
        : softwareFactory_(std::move(softwareFactory)),
          hardwareFactory_(std::move(hardwareFactory)),
          runtimeState_(std::move(runtimeState))
    {}

    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const
    {
        std::lock_guard lock(mutex_);
        std::vector<webrtc::SdpVideoFormat> formats;
        if (hardwareFactory_) {
            AppendUniqueFormats(
                formats, hardwareFactory_->GetSupportedFormats());
        }
        if (softwareFactory_) {
            AppendUniqueFormats(
                formats, softwareFactory_->GetSupportedFormats());
        }
        return formats;
    }

    webrtc::VideoEncoderFactory::CodecSupport QueryCodecSupport(
        const webrtc::SdpVideoFormat& format,
        std::optional<std::string> scalabilityMode,
        std::optional<webrtc::Resolution> resolution) const
    {
        std::lock_guard lock(mutex_);
        if (hardwareFactory_) {
            const auto support = hardwareFactory_->QueryCodecSupport(
                format, scalabilityMode, resolution);
            if (support.is_supported) {
                return support;
            }
        }
        return softwareFactory_
                   ? softwareFactory_->QueryCodecSupport(
                         format, scalabilityMode, resolution)
                   : webrtc::VideoEncoderFactory::CodecSupport{};
    }

    std::unique_ptr<webrtc::VideoEncoder> CreateSelected(
        const webrtc::Environment& environment,
        const webrtc::SdpVideoFormat& format,
        std::uint64_t* revision) const
    {
        std::lock_guard lock(mutex_);
        std::unique_ptr<webrtc::VideoEncoder> hardware;
        if (hardwareFactory_) {
            hardware = hardwareFactory_->Create(environment, format);
        }
        std::unique_ptr<webrtc::VideoEncoder> software;
        if (softwareFactory_) {
            software = softwareFactory_->Create(environment, format);
            if (software && runtimeState_) {
                software =
                    std::make_unique<RuntimeTrackedSoftwareEncoder>(
                        std::move(software), runtimeState_);
            }
        }
        if (revision) {
            *revision = revision_;
        }
        if (hardware && software) {
            hardware =
                std::make_unique<TransientFailureRetryVideoEncoder>(
                    std::move(hardware));
            webrtc::EnvironmentFactory fallbackEnvironmentFactory(
                environment);
            fallbackEnvironmentFactory.Set(
                std::make_unique<HardwareFailureOnlyFieldTrials>(
                    environment.field_trials()));
            const webrtc::Environment fallbackEnvironment =
                fallbackEnvironmentFactory.Create();
            return webrtc::CreateVideoEncoderSoftwareFallbackWrapper(
                fallbackEnvironment,
                std::move(software), std::move(hardware),
                false);
        }
        return hardware ? std::move(hardware) : std::move(software);
    }

    std::uint64_t Revision() const
    {
        std::lock_guard lock(mutex_);
        return revision_;
    }

    bool HasHardwareEncoderFor(
        const webrtc::SdpVideoFormat& format) const
    {
        std::lock_guard lock(mutex_);
        return hardwareFactory_ && format.IsCodecInList(
            hardwareFactory_->GetSupportedFormats());
    }

    bool HasSoftwareEncoderFor(
        const webrtc::SdpVideoFormat& format) const
    {
        std::lock_guard lock(mutex_);
        return softwareFactory_ && format.IsCodecInList(
            softwareFactory_->GetSupportedFormats());
    }

    void ReplaceFactories(
        std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
        std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory)
    {
        std::lock_guard lock(mutex_);
        softwareFactory_ = std::move(softwareFactory);
        hardwareFactory_ = std::move(hardwareFactory);
        ++revision_;
    }

private:
    mutable std::mutex mutex_;
    std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory_;
    std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory_;
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState_;
    std::uint64_t revision_ = 1;
};

namespace {

class ReconfigurableVideoEncoder final : public webrtc::VideoEncoder {
public:
    ReconfigurableVideoEncoder(
        std::shared_ptr<WindowsPreferredVideoEncoderFactoryState> state,
        webrtc::Environment environment,
        webrtc::SdpVideoFormat format)
        : state_(std::move(state)),
          environment_(std::move(environment)),
          format_(std::move(format)),
          encoder_(state_->CreateSelected(
              environment_, format_, &revision_))
    {}

    void SetFecControllerOverride(
        webrtc::FecControllerOverride* fecControllerOverride) override
    {
        fecControllerOverride_ = fecControllerOverride;
        if (encoder_) {
            encoder_->SetFecControllerOverride(fecControllerOverride);
        }
    }

    int InitEncode(
        const webrtc::VideoCodec* codecSettings,
        const Settings& settings) override
    {
        if (!codecSettings || !EnsureCurrent(false)) {
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        codecSettings_ = *codecSettings;
        settings_ = settings;
        const int result = encoder_->InitEncode(codecSettings, settings);
        initialized_ = result == WEBRTC_VIDEO_CODEC_OK;
        return result;
    }

    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override
    {
        callback_ = callback;
        return encoder_
                   ? encoder_->RegisterEncodeCompleteCallback(callback)
                   : WEBRTC_VIDEO_CODEC_ERROR;
    }

    int32_t Release() override
    {
        initialized_ = false;
        callback_ = nullptr;
        rates_.reset();
        return encoder_ ? encoder_->Release() : WEBRTC_VIDEO_CODEC_OK;
    }

    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frameTypes) override
    {
        return EnsureCurrent(true)
                   ? encoder_->Encode(frame, frameTypes)
                   : WEBRTC_VIDEO_CODEC_ERROR;
    }

    void SetRates(const RateControlParameters& parameters) override
    {
        rates_ = parameters;
        if (encoder_) {
            encoder_->SetRates(parameters);
        }
    }

    void OnPacketLossRateUpdate(float packetLossRate) override
    {
        if (encoder_) {
            encoder_->OnPacketLossRateUpdate(packetLossRate);
        }
    }

    void OnRttUpdate(int64_t rttMs) override
    {
        if (encoder_) {
            encoder_->OnRttUpdate(rttMs);
        }
    }

    void OnLossNotification(
        const LossNotification& lossNotification) override
    {
        if (encoder_) {
            encoder_->OnLossNotification(lossNotification);
        }
    }

    EncoderInfo GetEncoderInfo() const override
    {
        return encoder_ ? encoder_->GetEncoderInfo() : EncoderInfo{};
    }

private:
    bool EnsureCurrent(bool initialize)
    {
        if (!state_) {
            return false;
        }
        const std::uint64_t currentRevision = state_->Revision();
        if (encoder_ && currentRevision == revision_) {
            return true;
        }
        std::uint64_t newRevision = 0;
        auto replacement = state_->CreateSelected(
            environment_, format_, &newRevision);
        if (!replacement) {
            return false;
        }
        replacement->SetFecControllerOverride(fecControllerOverride_);
        if (initialize && initialized_ && codecSettings_ && settings_) {
            const int initResult = replacement->InitEncode(
                &*codecSettings_, *settings_);
            if (initResult != WEBRTC_VIDEO_CODEC_OK) {
                replacement->Release();
                return false;
            }
            if (callback_ &&
                replacement->RegisterEncodeCompleteCallback(callback_) !=
                    WEBRTC_VIDEO_CODEC_OK) {
                replacement->Release();
                return false;
            }
            if (rates_) {
                replacement->SetRates(*rates_);
            }
        }
        if (encoder_) {
            encoder_->Release();
        }
        encoder_ = std::move(replacement);
        revision_ = newRevision;
        return true;
    }

    std::shared_ptr<WindowsPreferredVideoEncoderFactoryState> state_;
    webrtc::Environment environment_;
    webrtc::SdpVideoFormat format_;
    std::unique_ptr<webrtc::VideoEncoder> encoder_;
    std::uint64_t revision_ = 0;
    webrtc::FecControllerOverride* fecControllerOverride_ = nullptr;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    bool initialized_ = false;
    std::optional<webrtc::VideoCodec> codecSettings_;
    std::optional<Settings> settings_;
    std::optional<RateControlParameters> rates_;
};

}  // namespace

WindowsPreferredVideoEncoderFactory::
WindowsPreferredVideoEncoderFactory(
    std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
    std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory,
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState)
    : state_(std::make_shared<WindowsPreferredVideoEncoderFactoryState>(
          std::move(softwareFactory), std::move(hardwareFactory),
          std::move(runtimeState)))
{}

WindowsPreferredVideoEncoderFactory::
~WindowsPreferredVideoEncoderFactory() = default;

std::vector<webrtc::SdpVideoFormat>
WindowsPreferredVideoEncoderFactory::GetSupportedFormats() const
{
    return state_->GetSupportedFormats();
}

webrtc::VideoEncoderFactory::CodecSupport
WindowsPreferredVideoEncoderFactory::QueryCodecSupport(
    const webrtc::SdpVideoFormat& format,
    std::optional<std::string> scalabilityMode,
    std::optional<webrtc::Resolution> resolution) const
{
    return state_->QueryCodecSupport(
        format, std::move(scalabilityMode), resolution);
}

std::unique_ptr<webrtc::VideoEncoder>
WindowsPreferredVideoEncoderFactory::Create(
    const webrtc::Environment& environment,
    const webrtc::SdpVideoFormat& format)
{
    auto selected = std::make_unique<ReconfigurableVideoEncoder>(
        state_, environment, format);
    if (selected->GetEncoderInfo().implementation_name.empty()) {
        return nullptr;
    }
    return std::make_unique<FrameTimedVideoEncoder>(
        std::move(selected));
}

bool WindowsPreferredVideoEncoderFactory::HasHardwareEncoderFor(
    const webrtc::SdpVideoFormat& format) const
{
    return state_->HasHardwareEncoderFor(format);
}

bool WindowsPreferredVideoEncoderFactory::HasSoftwareEncoderFor(
    const webrtc::SdpVideoFormat& format) const
{
    return state_->HasSoftwareEncoderFor(format);
}

void WindowsPreferredVideoEncoderFactory::ReplaceFactories(
    std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
    std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory)
{
    state_->ReplaceFactories(
        std::move(softwareFactory), std::move(hardwareFactory));
}

}  // namespace remote
