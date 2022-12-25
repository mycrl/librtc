#include "video_encoder.h"

std::unique_ptr<IVideoEncoder> IVideoEncoder::Create(std::string name,
	Parameters pars,
	void(*handler)(IVideoFrame* frame, VideoFrameType* type, size_t len),
	void(*initer)(CodecSettings* settings))
{
	auto self = std::make_unique<IVideoEncoder>();
	self->_handler = handler;
	self->_initer = initer;
	self->pars = pars;
	self->name = name;
	return self;
}

int IVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings, 
	const Settings& settings)
{
	CodecSettings settings_;
	settings_.width = codec_settings->width;
	settings_.height = codec_settings->height;
	settings_.start_bitrate = codec_settings->startBitrate;
	settings_.max_bitrate = codec_settings->maxBitrate;
	settings_.min_bitrate = codec_settings->minBitrate;
	settings_.max_framerate = codec_settings->maxFramerate;
	settings_.active = codec_settings->active;
	settings_.qp_max = codec_settings->qpMax;
	settings_.number_of_simulcast_streams = codec_settings->numberOfSimulcastStreams;
	settings_.expect_encode_from_texture = codec_settings->expect_encode_from_texture;
	settings_.legacy_conference_mode = codec_settings->legacy_conference_mode;
	settings_.loss_notification = settings.capabilities.loss_notification;
	settings_.number_of_cores = settings.number_of_cores;
	settings_.max_payload_size = settings.max_payload_size;
	_initer(&settings_);
	return WEBRTC_VIDEO_CODEC_OK;
}

int32_t IVideoEncoder::RegisterEncodeCompleteCallback(
	webrtc::EncodedImageCallback* callback)
{
	_callback = callback;
	return WEBRTC_VIDEO_CODEC_OK;
}

int32_t IVideoEncoder::Encode(const webrtc::VideoFrame& frame,
	const std::vector<webrtc::VideoFrameType>* frame_types)
{
	auto c_frame = into_c((webrtc::VideoFrame*)&frame);
	auto types = (VideoFrameType*)frame_types->data();
	_handler(c_frame, types, frame_types->size());
	return WEBRTC_VIDEO_CODEC_OK;
}

void IVideoEncoder::SetRates(
	const webrtc::VideoEncoder::RateControlParameters& parameters)
{

}

int32_t IVideoEncoder::Release()
{
	return WEBRTC_VIDEO_CODEC_OK;
}

VideoEncoder* rtc_create_video_encoder(char* name, Parameter** pars, size_t len)
{
	VideoEncoder* v_encoder = (VideoEncoder*)malloc(sizeof(VideoEncoder));
	if (!v_encoder)
	{
		return NULL;
	}

	Parameters parameters;
	for (size_t i = 0; i < len; i++)
	{
		parameters.insert(std::string(pars[i]->key), std::string(pars[i]->value));
	}

	v_encoder->encoder = IVideoEncoder::Create(std::string(name), parameters);
	return v_encoder;
}

VideoEncoderFactory* rtc_create_video_encoder_factory(VideoEncoder** encoders, size_t len)
{
	VideoEncoderFactory* v_factory = (VideoEncoderFactory*)malloc(sizeof(VideoEncoderFactory));
	if (!v_factory)
	{
		return NULL;
	}

	std::vector<VideoEncoder*> encoders_;
	for (size_t i = 0; i < len; i++)
	{
		encoders_.push_back(encoders[i]);
	}

	v_factory->factory = IVideoEncoderFactory::Create(encoders_);
	return v_factory;
}