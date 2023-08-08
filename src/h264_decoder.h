//
//  h264_decoder.h
//  rtc
//
//  Created by Mr.Panda on 2023/3/16.
//

#ifndef LIBRTC_H264_ENCODER_H
#define LIBRTC_H264_ENCODER_H
#pragma once

#include "api/create_peerconnection_factory.h"
#include "api/video/i420_buffer.h"
#include "h264.h"

#include <optional>

extern "C"
{
#include "libavcodec/avcodec.h"
}

class H264Decoder
	: public webrtc::VideoDecoder
{
public:
	static std::vector<webrtc::SdpVideoFormat> GetSupportedFormats();
	static std::unique_ptr<H264Decoder> Create();

	// Prepares decoder to handle incoming encoded frames. Can be called multiple
	// times, in such case only latest `settings` are in effect.
	bool Configure(const Settings& settings);
	int32_t Decode(const webrtc::EncodedImage& image, bool missing_frames, int64_t render_time_ms);
	int32_t RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback);

	// Free encoder memory.
	// Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
	int32_t Release();
private:
	int _ReadFrame(const webrtc::EncodedImage& input_image, int64_t render_time_ms);

	std::optional<rtc::scoped_refptr<webrtc::I420Buffer>> _i420_buffer = std::nullopt;
	std::optional<webrtc::DecodedImageCallback*> _callback = std::nullopt;
	std::optional<webrtc::EncodedImage> _image = std::nullopt;

	AVCodecParserContext* _parser = nullptr;
	const AVCodec* _codec = nullptr;
	AVCodecContext* _ctx = nullptr;
	AVPacket* _packet = nullptr;
	AVFrame* _frame = nullptr;
};

#endif // LIBRTC_H264_ENCODER_H
