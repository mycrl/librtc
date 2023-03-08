//
//  video_encoder.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef librtc_video_encoder_h
#define librtc_video_encoder_h
#pragma once

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "api/video_codecs/video_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "common_video/h264/h264_bitstream_parser.h"
#include "common_video/h264/h264_common.h"
#include "frame.h"
#include "base.h"

class Encoder
{
public:
    Encoder();
    void Release();
    
    webrtc::H264PacketizationMode packetization_mode;
    webrtc::EncodedImage image;
    uint8_t simulcast_idx;
    
    const AVCodec* codec;
    AVCodecContext* ctx;
    AVPacket* packet;
    AVFrame* frame;
};

class IVideoEncoder
: public webrtc::VideoEncoder
{
public:
    IVideoEncoder(const webrtc::SdpVideoFormat& format): _format(format) {}
    static std::unique_ptr<IVideoEncoder> Create(const webrtc::SdpVideoFormat& format);
    int InitEncode(const webrtc::VideoCodec* codec_settings, const Settings& settings);
    int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback);
    int32_t Encode(const webrtc::VideoFrame& frame,
                   const std::vector<webrtc::VideoFrameType>* frame_types);
    void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters);
    int32_t Release();
private:
    std::optional<Encoder> open_encoder(const webrtc::SimulcastStream* stream,
                                        int stream_idx);
    
    const webrtc::VideoCodec* _codec_settings = nullptr;
    webrtc::EncodedImageCallback* _callback = nullptr;
    webrtc::H264BitstreamParser _h264_bitstream_parser;
    const webrtc::SdpVideoFormat& _format;
    std::vector<Encoder> _encoders;
};

class IVideoEncoderFactory
: public webrtc::VideoEncoderFactory
{
public:
    static std::unique_ptr<IVideoEncoderFactory> Create();
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const;
    std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& format);
    
    std::optional<std::unique_ptr<webrtc::VideoEncoderFactory>> _factory = nullptr;
    std::vector<webrtc::SdpVideoFormat> _formats;
};

typedef struct
{
    std::unique_ptr<IVideoEncoderFactory> factory;
} VideoEncoderFactory;

#endif  // librtc_video_encoder_h
