//
//  h264.h
//  rtc
//
//  Created by Mr.Panda on 2023/3/15.
//

#ifndef LIBRTC_H264_H
#define LIBRTC_H264_H
#pragma once

#include "modules/video_coding/codecs/h264/include/h264.h"

#include <string>
#include <optional>

extern "C"
{
#include "libavutil/hwcontext.h"
}

enum CodecRet
{
	Ok = WEBRTC_VIDEO_CODEC_OK,
	Err = WEBRTC_VIDEO_CODEC_ERROR,
};

typedef struct
{
	const std::string name;
	AVHWDeviceType type;
} CodecDesc;

static CodecDesc VideoDecoders[] = {
	{"h264_qsv", AV_HWDEVICE_TYPE_QSV},
	{"h264_cuvid", AV_HWDEVICE_TYPE_CUDA},
	{"h264_videotoolbox", AV_HWDEVICE_TYPE_VIDEOTOOLBOX},
};

static CodecDesc VideoEncoders[] = {
	{"h264_qsv", AV_HWDEVICE_TYPE_QSV},
	{"h264_nvenc", AV_HWDEVICE_TYPE_CUDA},
	{"h264_videotoolbox", AV_HWDEVICE_TYPE_VIDEOTOOLBOX},
};

constexpr webrtc::ScalabilityMode IkSupportedScalabilityModes[] = {
	webrtc::ScalabilityMode::kL1T1,
	webrtc::ScalabilityMode::kL1T2,
	webrtc::ScalabilityMode::kL1T3
};

webrtc::SdpVideoFormat create_h264_format(webrtc::H264Profile profile,
										  webrtc::H264Level level,
										  const std::string& packetization_mode,
										  bool add_scalability_modes);
std::vector<webrtc::SdpVideoFormat> supported_h264_codecs(bool mode /* add_scalability_modes */);

template <size_t S>
const std::string find_codec(CodecDesc(&codecs)[S])
{
	AVBufferRef* ctx = nullptr;
	for (auto codec : codecs)
	{
		if (av_hwdevice_ctx_create(&ctx, codec.type, nullptr, nullptr, 0) == 0)
		{
			av_buffer_unref(&ctx);
			return codec.name;
		}
	}

	if (ctx != nullptr)
	{
		av_buffer_unref(&ctx);
	}

	return "libx264";
}

#endif // LIBRTC_H264_H
