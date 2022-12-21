#include "video_encoder.h"

webrtc::SdpVideoFormat from_c(EncoderFormat* c_format)
{
	std::map<std::string, std::string> parameters;
	for (size_t i = 0; i < c_format->len; i++)
	{
		auto par = c_format->pars[i];
		parameters.insert(std::string(par->key), std::string(par->value));
	}

	return webrtc::SdpVideoFormat(std::string(c_format->name), parameters);
}

std::vector<webrtc::SdpVideoFormat> from_c(EncoderFormats* c_formats)
{
	std::vector<webrtc::SdpVideoFormat> formats;
	for (size_t i = 0; i < c_formats->len; i++)
	{
		formats.push_back(from_c(c_formats->formats[i]));
	}

	return formats;
}