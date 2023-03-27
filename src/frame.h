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

/* yuv420p format */

typedef struct
{
    bool remote;
    uint32_t width;
    uint32_t height;
    int64_t timestamp;
    
    // planar y
    const uint8_t* data_y;
    uint32_t stride_y;
    
    // planar u
    const uint8_t* data_u;
    uint32_t stride_u;
    
    // planar v
    const uint8_t* data_v;
    uint32_t stride_v;
} IVideoFrame;

/* pcm format */

typedef struct
{
    bool remote;
    size_t size;
    size_t frames;
    size_t channels;
    int sample_rate;
    int64_t timestamp;
    const int16_t* data;
} IAudioFrame;

extern "C" EXPORT void rtc_free_frame(void* frame);

IAudioFrame* into_c(const int16_t* data,
                    int sample_rate,
                    size_t channels,
                    size_t frames,
                    int64_t timestamp);
IVideoFrame* into_c(webrtc::VideoFrame* frame);
webrtc::VideoFrame from_c(IVideoFrame* frame);

size_t get_i420_buffer_size(webrtc::I420BufferInterface* data);
size_t get_i420_buffer_size(const webrtc::I420BufferInterface* data);

#endif  // librtc_frame_h
