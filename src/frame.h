//
//  frame.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_FRAME_H
#define LIBRTC_FRAME_H
#pragma once

#include "api/video/video_frame.h"
#include "base.h"
#include "rtc.h"

IAudioFrame* into_c(const int16_t* data,
					int sample_rate,
					size_t channels,
					size_t frames,
					int64_t timestamp);
IVideoFrame* into_c(webrtc::VideoFrame* frame);
webrtc::VideoFrame from_c(IVideoFrame* frame);

size_t get_i420_buffer_size(webrtc::I420BufferInterface* data);
size_t get_i420_buffer_size(const webrtc::I420BufferInterface* data);

#endif  // LIBRTC_FRAME_H
