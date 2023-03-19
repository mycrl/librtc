//
//  frame.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef librtc_frame_h
#define librtc_frame_h
#pragma once

#include "api/video/video_frame.h"
#include "base.h"

typedef struct
{
    const uint8_t* buf;
    size_t len;
    
    uint32_t width;
    uint32_t height;
    uint32_t stride_y;
    uint32_t stride_u;
    uint32_t stride_v;
    bool remote;
} IVideoFrame;

typedef struct
{
    const void* buf;
    size_t len;
    
    int bits_per_sample;
    int sample_rate;
    size_t channels;
    size_t frames;
    int64_t ms;
    bool remote;
} IAudioFrame;

extern "C" EXPORT void rtc_free_video_frame(IVideoFrame* frame);
extern "C" EXPORT void rtc_free_audio_frame(IAudioFrame* frame);

IAudioFrame* into_c(const void* buf,
                    int bits_per_sample,
                    int sample_rate,
                    size_t channels,
                    size_t frames_);
IVideoFrame* into_c(webrtc::VideoFrame* frame);
webrtc::VideoFrame from_c(IVideoFrame* frame);

size_t get_i420_buffer_size(webrtc::I420BufferInterface* buf);
size_t get_i420_buffer_size(const webrtc::I420BufferInterface* buf);

#endif  // librtc_frame_h
