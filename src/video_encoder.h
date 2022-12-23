#ifndef BATRACHIATC_VIDEO_ENCODER_H_
#define BATRACHIATC_VIDEO_ENCODER_H_
#pragma once

#include "api/video_codecs/video_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "common_video/h264/h264_common.h"

typedef struct
{
	char* key;
	char* value;
} Parameter;

typedef struct
{
	char* name;
	Parameter** pars;
    size_t capacity;
	size_t len;
} Codec;

typedef struct
{
	Codec** codes;
    size_t capacity;
	size_t len;
} Codecs;

class IVideoEncoder
	: public webrtc::VideoEncoder
{
public:
	static std::unique_ptr<IVideoEncoder> Create();
	int InitEncode(const webrtc::VideoCodec* codec_settings, const Settings& settings);
	int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback);
	int32_t Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frame_types);
	void SetRates(const RateControlParameters& parameters);
	int32_t Release();
private:
	webrtc::EncodedImageCallback* _callback = NULL;
};

class IVideoEncoderFactory
	: public webrtc::VideoEncoderFactory
{
public:
	static std::unique_ptr<IVideoEncoderFactory> Create();
	std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const;
	std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& format);
private:
	std::vector<webrtc::SdpVideoFormat> _formats;
	std::map<std::string, IVideoEncoder> _codecs;;
};

#endif  // BATRACHIATC_VIDEO_ENCODER_H_