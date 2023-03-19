//
//  frame.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "api/video/i420_buffer.h"
#include "frame.h"

void rtc_free_video_frame(IVideoFrame* frame)
{
    free_incomplete_ptr(frame);
}

void rtc_free_audio_frame(IAudioFrame* frames)
{
    free_incomplete_ptr(frames);
}

IVideoFrame* into_c(webrtc::VideoFrame* frame)
{
    IVideoFrame* i420_frame = (IVideoFrame*)malloc(sizeof(IVideoFrame));
    if (!i420_frame)
    {
        return NULL;
    }
    
    auto video_frame_buf = frame->video_frame_buffer();
    auto i420_buf = video_frame_buf->GetI420();
    if (!i420_buf)
    {
        i420_buf = video_frame_buf->ToI420().get();
    }
    
    size_t size_y = i420_frame->stride_y * i420_frame->height;
    size_t size_uv = i420_frame->stride_u * (i420_frame->height / 2);
    size_t buf_size = size_y + (size_uv * 2);
    
    i420_frame->remote = true;
    i420_frame->len = buf_size;
    i420_frame->buf = i420_buf->DataY();
    i420_frame->width = i420_buf->width();
    i420_frame->height = i420_buf->height();
    i420_frame->stride_y = i420_buf->StrideY();
    i420_frame->stride_u = i420_buf->StrideU();
    i420_frame->stride_v = i420_buf->StrideV();
    
    return i420_frame;
}

webrtc::VideoFrame from_c(IVideoFrame* frame)
{
    size_t size_y = frame->stride_y * frame->height;
    size_t size_u = frame->stride_u * (frame->height / 2);
    auto i420_buf = webrtc::I420Buffer::Copy(
                                             frame->width, frame->height,
                                             frame->buf, frame->stride_y,
                                             frame->buf + size_y, frame->stride_u,
                                             frame->buf + size_y + size_u, frame->stride_v);
    return webrtc::VideoFrame(i420_buf, 0, 0, webrtc::kVideoRotation_0);
}

IAudioFrame* into_c(const void* buf,
                    int bits_per_sample,
                    int sample_rate,
                    size_t channels,
                    size_t frames_)
{
    IAudioFrame* frames = (IAudioFrame*)malloc(sizeof(IAudioFrame));
    if (!frames)
    {
        return NULL;
    }
    
    frames->buf = buf;
    frames->remote = true;
    frames->frames = frames_;
    frames->channels = channels;
    frames->sample_rate = sample_rate;
    frames->bits_per_sample = bits_per_sample;
    return frames;
}

size_t get_i420_buffer_size(webrtc::I420BufferInterface* buf)
{
    size_t sizey = buf->StrideY() * buf->height();
    size_t sizeu = buf->StrideU() * (buf->height() / 2);
    return sizey + (sizeu * 2);
}

size_t get_i420_buffer_size(const webrtc::I420BufferInterface* buf)
{
    return get_i420_buffer_size((webrtc::I420BufferInterface*)buf);
}
