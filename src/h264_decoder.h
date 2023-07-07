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

	rtc::scoped_refptr<webrtc::I420Buffer> _i420_buffer = nullptr;
	webrtc::DecodedImageCallback* _callback;
	webrtc::EncodedImage _image;
	AVCodecParserContext* _parser;
	const AVCodec* _codec;
	AVCodecContext* _ctx;
	AVPacket* _packet;
	AVFrame* _frame;
};

#endif // LIBRTC_H264_ENCODER_H
