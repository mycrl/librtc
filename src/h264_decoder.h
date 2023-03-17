//
//  h264_decoder.h
//  rtc
//
//  Created by Mr.Panda on 2023/3/16.
//

#ifndef librtc_h264_decoder_h
#define librtc_h264_decoder_h
#pragma once

#include "api/create_peerconnection_factory.h"
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
    bool Configure(const Settings& settings);
    int32_t Decode(const webrtc::EncodedImage& input_image,
                   bool missing_frames,
                   int64_t render_time_ms);
    int32_t RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback);
    int32_t Release();
private:
    int _OnFrame();
    
    webrtc::DecodedImageCallback* _callback;
    webrtc::EncodedImage _image;
    AVCodecContext* _ctx;
    AVPacket* _packet;
    AVFrame* _frame;
    CodecLayer _layer;
};

#endif /* librtc_h264_decoder_h */
