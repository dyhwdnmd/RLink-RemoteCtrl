// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "QualityOpenH264Encoder.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>

#include "api/video/encoded_image.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video_codecs/video_codec.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/codecs/interface/common_constants.h"
#include "modules/video_coding/include/video_codec_interface.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "third_party/openh264/src/codec/api/wels/codec_api.h"
#include "third_party/openh264/src/codec/api/wels/codec_app_def.h"
#include "third_party/openh264/src/codec/api/wels/codec_def.h"

namespace remote {
namespace {

ECOMPLEXITY_MODE OpenH264Complexity(FfmpegX264Preset quality)
{
    switch (VideoEncoderQualityProfileForPreset(quality)
                .openH264Complexity) {
    case OpenH264QualityComplexity::kHigh:
        return HIGH_COMPLEXITY;
    case OpenH264QualityComplexity::kMedium:
        return MEDIUM_COMPLEXITY;
    case OpenH264QualityComplexity::kLow:
        return LOW_COMPLEXITY;
    }
    return MEDIUM_COMPLEXITY;
}

const char* OpenH264ComplexityName(FfmpegX264Preset quality)
{
    switch (OpenH264Complexity(quality)) {
    case HIGH_COMPLEXITY:
        return "high";
    case LOW_COMPLEXITY:
        return "low";
    case MEDIUM_COMPLEXITY:
    default:
        return "medium";
    }
}

}  // namespace

class QualityOpenH264Encoder::Impl final {
public:
    explicit Impl(FfmpegX264Preset quality) : quality_(quality) {}
    ~Impl() { Release(); }

    int Init(const webrtc::VideoCodec* codecSettings,
             const webrtc::VideoEncoder::Settings& settings)
    {
        std::lock_guard lock(mutex_);
        ReleaseUnlocked();
        if (!codecSettings ||
            codecSettings->codecType != webrtc::kVideoCodecH264 ||
            codecSettings->width == 0 || codecSettings->height == 0 ||
            codecSettings->maxFramerate == 0) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        if (!codecSettings->IsSinglecast()) {
            return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
        }
        if (WelsCreateSVCEncoder(&encoder_) != 0 || !encoder_) {
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        width_ = codecSettings->width;
        height_ = codecSettings->height;
        frameRate_ = codecSettings->maxFramerate;
        bitrateBps_ = (std::max<std::uint32_t>)(
            codecSettings->startBitrate * 1000, 100'000);

        SEncParamExt parameters{};
        if (encoder_->GetDefaultParams(&parameters) != 0) {
            ReleaseUnlocked();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        parameters.iUsageType = codecSettings->mode ==
                webrtc::VideoCodecMode::kScreensharing
            ? SCREEN_CONTENT_REAL_TIME
            : CAMERA_VIDEO_REAL_TIME;
        parameters.iPicWidth = static_cast<int>(width_);
        parameters.iPicHeight = static_cast<int>(height_);
        parameters.iTargetBitrate = static_cast<int>(bitrateBps_);
        parameters.iMaxBitrate = UNSPECIFIED_BIT_RATE;
        parameters.iRCMode = RC_BITRATE_MODE;
        parameters.fMaxFrameRate = static_cast<float>(frameRate_);
        parameters.iComplexityMode = OpenH264Complexity(quality_);
        parameters.bEnableFrameSkip = codecSettings->GetFrameDropEnabled();
        parameters.uiIntraPeriod = codecSettings->H264().keyFrameInterval;
        parameters.eSpsPpsIdStrategy = SPS_LISTING;
        parameters.uiMaxNalSize = 0;
        parameters.iMultipleThreadIdc = (std::max)(
            1, (std::min)(settings.number_of_cores, 4));
        parameters.iTemporalLayerNum = 1;
        auto& layer = parameters.sSpatialLayers[0];
        layer.iVideoWidth = parameters.iPicWidth;
        layer.iVideoHeight = parameters.iPicHeight;
        layer.fFrameRate = parameters.fMaxFrameRate;
        layer.iSpatialBitrate = parameters.iTargetBitrate;
        layer.iMaxSpatialBitrate = parameters.iMaxBitrate;
        layer.uiProfileIdc = PRO_BASELINE;
        layer.uiLevelIdc = LEVEL_UNKNOWN;
        layer.sSliceArgument.uiSliceNum = 1;
        layer.sSliceArgument.uiSliceMode = SM_FIXEDSLCNUM_SLICE;

        if (encoder_->InitializeExt(&parameters) != 0) {
            ReleaseUnlocked();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        int format = videoFormatI420;
        if (encoder_->SetOption(ENCODER_OPTION_DATAFORMAT, &format) != 0) {
            ReleaseUnlocked();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        initialized_ = true;
        forceKeyFrame_ = true;
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int RegisterCallback(webrtc::EncodedImageCallback* callback)
    {
        std::lock_guard lock(mutex_);
        callback_ = callback;
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int Release()
    {
        std::lock_guard lock(mutex_);
        ReleaseUnlocked();
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int Encode(const webrtc::VideoFrame& frame,
               const std::vector<webrtc::VideoFrameType>* frameTypes)
    {
        std::lock_guard lock(mutex_);
        if (!initialized_ || !encoder_ || !callback_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }
        const auto source = frame.video_frame_buffer();
        if (!source || source->width() != static_cast<int>(width_) ||
            source->height() != static_cast<int>(height_)) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        const auto i420 = source->ToI420();
        if (!i420) {
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        const bool keyRequested = forceKeyFrame_ ||
            (frameTypes &&
             std::find(frameTypes->begin(), frameTypes->end(),
                       webrtc::VideoFrameType::kVideoFrameKey) !=
                 frameTypes->end());
        if (keyRequested) {
            if (encoder_->ForceIntraFrame(true) != 0) {
                return WEBRTC_VIDEO_CODEC_ERROR;
            }
            forceKeyFrame_ = false;
        }

        SSourcePicture picture{};
        picture.iColorFormat = videoFormatI420;
        picture.iPicWidth = static_cast<int>(width_);
        picture.iPicHeight = static_cast<int>(height_);
        picture.iStride[0] = i420->StrideY();
        picture.iStride[1] = i420->StrideU();
        picture.iStride[2] = i420->StrideV();
        picture.pData[0] = const_cast<unsigned char*>(i420->DataY());
        picture.pData[1] = const_cast<unsigned char*>(i420->DataU());
        picture.pData[2] = const_cast<unsigned char*>(i420->DataV());
        picture.uiTimeStamp = static_cast<unsigned long long>(
            frame.timestamp_us() / 1000);

        SFrameBSInfo bitstream{};
        if (encoder_->EncodeFrame(&picture, &bitstream) != 0) {
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        if (bitstream.eFrameType == videoFrameTypeSkip ||
            bitstream.iLayerNum <= 0) {
            return WEBRTC_VIDEO_CODEC_OK;
        }

        std::size_t encodedSize = 0;
        for (int layerIndex = 0; layerIndex < bitstream.iLayerNum;
             ++layerIndex) {
            const auto& layerInfo = bitstream.sLayerInfo[layerIndex];
            for (int nal = 0; nal < layerInfo.iNalCount; ++nal) {
                if (layerInfo.pNalLengthInByte[nal] < 0) {
                    return WEBRTC_VIDEO_CODEC_ERROR;
                }
                encodedSize += static_cast<std::size_t>(
                    layerInfo.pNalLengthInByte[nal]);
            }
        }
        auto encodedBuffer =
            webrtc::EncodedImageBuffer::Create(encodedSize);
        std::size_t offset = 0;
        for (int layerIndex = 0; layerIndex < bitstream.iLayerNum;
             ++layerIndex) {
            const auto& layerInfo = bitstream.sLayerInfo[layerIndex];
            std::size_t layerSize = 0;
            for (int nal = 0; nal < layerInfo.iNalCount; ++nal) {
                layerSize += static_cast<std::size_t>(
                    layerInfo.pNalLengthInByte[nal]);
            }
            std::memcpy(encodedBuffer->data() + offset,
                        layerInfo.pBsBuf, layerSize);
            offset += layerSize;
        }

        const bool keyFrame = bitstream.eFrameType == videoFrameTypeIDR ||
                              bitstream.eFrameType == videoFrameTypeI;
        webrtc::EncodedImage image;
        image.SetEncodedData(std::move(encodedBuffer));
        image.SetRtpTimestamp(frame.rtp_timestamp());
        image.capture_time_ms_ = frame.timestamp_us() / 1000;
        image._encodedWidth = width_;
        image._encodedHeight = height_;
        image.rotation_ = frame.rotation();
        image.SetFrameType(
            keyFrame ? webrtc::VideoFrameType::kVideoFrameKey
                     : webrtc::VideoFrameType::kVideoFrameDelta);

        webrtc::CodecSpecificInfo codecInfo;
        codecInfo.codecType = webrtc::kVideoCodecH264;
        codecInfo.codecSpecific.H264.packetization_mode =
            webrtc::H264PacketizationMode::NonInterleaved;
        codecInfo.codecSpecific.H264.temporal_idx = webrtc::kNoTemporalIdx;
        codecInfo.codecSpecific.H264.idr_frame = keyFrame;
        codecInfo.codecSpecific.H264.base_layer_sync = false;
        callback_->OnEncodedImage(image, &codecInfo);
        return WEBRTC_VIDEO_CODEC_OK;
    }

    void SetRates(
        const webrtc::VideoEncoder::RateControlParameters& parameters)
    {
        std::lock_guard lock(mutex_);
        if (!encoder_) {
            return;
        }
        const std::uint32_t bitrate = parameters.bitrate.get_sum_bps();
        if (bitrate != 0) {
            bitrateBps_ = bitrate;
            SBitrateInfo target{};
            target.iLayer = SPATIAL_LAYER_ALL;
            target.iBitrate = static_cast<int>(bitrateBps_);
            encoder_->SetOption(ENCODER_OPTION_BITRATE, &target);
        }
        if (parameters.framerate_fps >= 1.0) {
            float frameRate = static_cast<float>(
                parameters.framerate_fps);
            encoder_->SetOption(ENCODER_OPTION_FRAME_RATE,
                                &frameRate);
            frameRate_ = static_cast<std::uint32_t>(
                parameters.framerate_fps);
        }
    }

    const char* ComplexityName() const
    {
        return OpenH264ComplexityName(quality_);
    }

private:
    void ReleaseUnlocked()
    {
        if (encoder_) {
            encoder_->Uninitialize();
            WelsDestroySVCEncoder(encoder_);
            encoder_ = nullptr;
        }
        initialized_ = false;
        forceKeyFrame_ = false;
        width_ = 0;
        height_ = 0;
    }

    std::mutex mutex_;
    ISVCEncoder* encoder_ = nullptr;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    FfmpegX264Preset quality_ = FfmpegX264Preset::kMedium;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    std::uint32_t frameRate_ = 30;
    std::uint32_t bitrateBps_ = 1'000'000;
    bool initialized_ = false;
    bool forceKeyFrame_ = false;
};

QualityOpenH264Encoder::QualityOpenH264Encoder(FfmpegX264Preset quality)
    : impl_(std::make_unique<Impl>(quality))
{}

QualityOpenH264Encoder::~QualityOpenH264Encoder() = default;

int QualityOpenH264Encoder::InitEncode(
    const webrtc::VideoCodec* codecSettings, const Settings& settings)
{
    return impl_->Init(codecSettings, settings);
}

int32_t QualityOpenH264Encoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback)
{
    return impl_->RegisterCallback(callback);
}

int32_t QualityOpenH264Encoder::Release()
{
    return impl_->Release();
}

int32_t QualityOpenH264Encoder::Encode(
    const webrtc::VideoFrame& frame,
    const std::vector<webrtc::VideoFrameType>* frameTypes)
{
    return impl_->Encode(frame, frameTypes);
}

void QualityOpenH264Encoder::SetRates(
    const RateControlParameters& parameters)
{
    impl_->SetRates(parameters);
}

webrtc::VideoEncoder::EncoderInfo
QualityOpenH264Encoder::GetEncoderInfo() const
{
    EncoderInfo info;
    info.supports_native_handle = false;
    info.implementation_name =
        std::string("OpenH264 (") + impl_->ComplexityName() + ")";
    info.has_trusted_rate_controller = false;
    info.is_hardware_accelerated = false;
    info.enable_cpu_overuse_detection = true;
    info.supports_simulcast = false;
    info.preferred_pixel_formats = {
        webrtc::VideoFrameBuffer::Type::kI420};
    info.requested_resolution_alignment = 2;
    info.apply_alignment_to_all_simulcast_layers = true;
    info.scaling_settings = ScalingSettings::kOff;
    return info;
}

std::vector<webrtc::SdpVideoFormat>
QualityOpenH264EncoderFactory::GetSupportedFormats() const
{
    return webrtc::SupportedH264Codecs();
}

webrtc::VideoEncoderFactory::CodecSupport
QualityOpenH264EncoderFactory::QueryCodecSupport(
    const webrtc::SdpVideoFormat& format,
    std::optional<std::string> scalabilityMode,
    std::optional<webrtc::Resolution> /* resolution */) const
{
    if ((scalabilityMode && *scalabilityMode != "L1T1") ||
        !format.IsCodecInList(GetSupportedFormats())) {
        return {};
    }
    return {.is_supported = true, .is_power_efficient = false};
}

std::unique_ptr<webrtc::VideoEncoder>
QualityOpenH264EncoderFactory::Create(
    const webrtc::Environment& /* environment */,
    const webrtc::SdpVideoFormat& format)
{
    if (!QueryCodecSupport(
            format, std::nullopt, std::nullopt).is_supported) {
        return nullptr;
    }
    return std::make_unique<QualityOpenH264Encoder>(quality_);
}

}  // namespace remote
