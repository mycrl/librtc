#ifndef BATRACHIATC_VIDEO_ENCODER_H_
#define BATRACHIATC_VIDEO_ENCODER_H_
#pragma once

#include "api/video_codecs/video_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "common_video/h264/h264_common.h"
#include "frame.h"
#include "base.h"

typedef struct
{
	char* key;
	char* value;
} Parameter;

typedef struct
{
	// TODO(nisse): Change to int, for consistency.
	uint16_t width;
	uint16_t height;

	uint32_t start_bitrate;  // kilobits/sec.
	uint32_t max_bitrate;    // kilobits/sec.
	uint32_t min_bitrate;    // kilobits/sec.
	uint32_t max_framerate;

	// This enables/disables encoding and sending when there aren't multiple
	// simulcast streams,by allocating 0 bitrate if inactive.
	bool active;

	uint32_t qp_max;
	uint8_t number_of_simulcast_streams;
	bool expect_encode_from_texture;

	// Legacy Google conference mode flag for simulcast screenshare
	bool legacy_conference_mode;

	bool loss_notification;
	int number_of_cores;
	size_t max_payload_size;
} CodecSettings;

typedef enum {
	kEmptyFrame = 0,
	// Wire format for MultiplexEncodedImagePacker seems to depend on numerical
	// values of these constants.
	kVideoFrameKey = 3,
	kVideoFrameDelta = 4,
} VideoFrameType;

typedef struct {
	// Target bitrate, per spatial/temporal layer.
	// A target bitrate of 0bps indicates a layer should not be encoded at all.
	//* VideoBitrateAllocation target_bitrate;
	// Adjusted target bitrate, per spatial/temporal layer. May be lower or
	// higher than the target depending on encoder behaviour.
	//* VideoBitrateAllocation bitrate;
	// Target framerate, in fps. A value <= 0.0 is invalid and should be
	// interpreted as framerate target not available. In this case the encoder
	// should fall back to the max framerate specified in `codec_settings` of
	// the last InitEncode() call.
	//* double framerate_fps;
	// The network bandwidth available for video. This is at least
	// `bitrate.get_sum_bps()`, but may be higher if the application is not
	// network constrained.
	//* DataRate bandwidth_allocation;
} RateControlParameters;

typedef std::map<std::string, std::string> Parameters;

class IVideoEncoder
	: public webrtc::VideoEncoder
{
public:
	static std::unique_ptr<IVideoEncoder> Create(std::string name,
		Parameters pars,
		void(*handler)(IVideoFrame* frame, VideoFrameType* type, size_t len),
		void(*initer)(CodecSettings* settings));
	int InitEncode(const webrtc::VideoCodec* codec_settings, const Settings& settings);
	int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback);
	int32_t Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frame_types);
	void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters);
	int32_t Release();

	Parameters pars;
	std::string name;
private:
	webrtc::EncodedImageCallback* _callback = NULL;
	void(*_handler)(IVideoFrame* frame, VideoFrameType* type, size_t len) = NULL;
	void(*_initer)(CodecSettings* settings) = NULL;
};

typedef struct
{
	std::unique_ptr<IVideoEncoder> encoder;
} VideoEncoder;

class IVideoEncoderFactory
	: public webrtc::VideoEncoderFactory
{
public:
	static std::unique_ptr<IVideoEncoderFactory> Create(std::vector<VideoEncoder*> encoders);
	std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const;
	std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& format);
private:
	std::vector<webrtc::SdpVideoFormat> _formats;
	std::map<std::string, IVideoEncoder> _codecs;;
};

typedef struct
{
	std::unique_ptr<IVideoEncoderFactory> factory;
} VideoEncoderFactory;

extern "C" EXPORT VideoEncoder* rtc_create_video_encoder(char* name, 
	Parameter** pars, 
	size_t len);

extern "C" EXPORT VideoEncoderFactory* rtc_create_video_encoder_factory(
	VideoEncoder** encoders, 
	size_t len);

#endif  // BATRACHIATC_VIDEO_ENCODER_H_