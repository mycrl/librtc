#ifndef BATRACHIATC_FRAME_H_
#define BATRACHIATC_FRAME_H_
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
    const uint8_t* buf;
    size_t len;

    int bits_per_sample;
    int sample_rate;
    int channels;
    int frames;
    bool remote;
} IAudioFrame;

extern "C" EXPORT void rtc_free_video_frame(IVideoFrame* frame);
extern "C" EXPORT void rtc_free_audio_frame(IAudioFrame* frame);

IAudioFrame* into_c(const uint8_t* buf, int b, int r, size_t c, size_t f);
IVideoFrame* into_c(webrtc::VideoFrame* frame);
webrtc::VideoFrame from_c(IVideoFrame* frame);

#endif  // BATRACHIATC_FRAME_H_