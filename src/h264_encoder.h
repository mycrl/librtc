//
//  h264_encoder.h
//  rtc
//
//  Created by Mr.Panda on 2023/3/10.
//

#ifndef librtc_h264_encoder_h
#define librtc_h264_encoder_h
#pragma once

#include "common_video/h264/h264_bitstream_parser.h"
#include "api/create_peerconnection_factory.h"
#include "common_video/h264/h264_common.h"
#include <optional>

extern "C"
{
#include "libavcodec/avcodec.h"
}

class H264Encoder
: public webrtc::VideoEncoder
{
public:
    H264Encoder(const webrtc::SdpVideoFormat& format);
    static std::vector<webrtc::SdpVideoFormat> GetSupportedFormats();
    static std::unique_ptr<H264Encoder> Create(const webrtc::SdpVideoFormat& format);
    int InitEncode(const webrtc::VideoCodec* codec_settings, const Settings& settings);
    int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback);
    int32_t Encode(const webrtc::VideoFrame& frame,
                   const std::vector<webrtc::VideoFrameType>* frame_types);
    void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters);
    int32_t Release();
private:
    int _OnFrame(webrtc::VideoFrameType frame_type,
                 const uint8_t* frame_buf,
                 int width,
                 int height,
                 size_t len);
    int _ReadPacket(webrtc::VideoFrameType frame_type,
                    const webrtc::VideoFrame& frame);
    
    webrtc::H264BitstreamParser _h264_bitstream_parser;
    webrtc::EncodedImageCallback* _callback;
    webrtc::H264PacketizationMode _pkt_mode;
    webrtc::EncodedImage _image;
    const AVCodec* _codec;
    AVCodecContext* _ctx;
    AVPacket* _packet;
    AVFrame* _frame;
    std::string _name;
};

#endif /* librtc_h264_encoder_h */
