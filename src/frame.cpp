//
//  frame.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "api/video/i420_buffer.h"
#include "rtc_base/time_utils.h"
#include "frame.h"

void rtc_free_frame(void* frame)
{
	free_incomplete_ptr(frame);
}

IVideoFrame* into_c(webrtc::VideoFrame* frame)
{
	IVideoFrame* i420_frame = (IVideoFrame*)malloc(sizeof(IVideoFrame));
	if (!i420_frame)
	{
		return nullptr;
	}

	auto video_frame_buf = frame->video_frame_buffer();
	auto i420_buf = video_frame_buf->GetI420();
	if (!i420_buf)
	{
		i420_buf = video_frame_buf->ToI420().get();
	}

	if (!i420_buf)
	{
		rtc_free_frame(i420_frame);
		return nullptr;
	}

	i420_frame->remote = true;
	i420_frame->width = i420_buf->width();
	i420_frame->height = i420_buf->height();
	i420_frame->data_y = i420_buf->DataY();
	i420_frame->stride_y = i420_buf->StrideY();
	i420_frame->data_u = i420_buf->DataU();
	i420_frame->stride_u = i420_buf->StrideU();
	i420_frame->data_v = i420_buf->DataV();
	i420_frame->stride_v = i420_buf->StrideV();
	i420_frame->timestamp = frame->timestamp();

	return i420_frame;
}

webrtc::VideoFrame from_c(IVideoFrame* frame)
{
	auto i420_buf = webrtc::I420Buffer::Copy(frame->width,
											 frame->height,
											 frame->data_y,
											 frame->stride_y,
											 frame->data_u,
											 frame->stride_u,
											 frame->data_v,
											 frame->stride_v);
	return webrtc::VideoFrame(i420_buf,
							  webrtc::kVideoRotation_0,
							  frame->timestamp * rtc::kNumMicrosecsPerMillisec);
}

IAudioFrame* into_c(const int16_t* data,
					int sample_rate,
					size_t channels,
					size_t frames,
					int64_t timestamp)
{
	IAudioFrame* frame = (IAudioFrame*)malloc(sizeof(IAudioFrame));
	if (!frame)
	{
		return nullptr;
	}

	frame->remote = true;
	frame->data = data;
	frame->frames = frames;
	frame->channels = channels;
	frame->timestamp = timestamp;
	frame->sample_rate = sample_rate;

	return frame;
}

size_t get_i420_buffer_size(webrtc::I420BufferInterface* data)
{
	size_t sizey = data->StrideY() * data->height();
	size_t sizeu = data->StrideU() * (data->height() / 2);
	return sizey + (sizeu * 2);
}

size_t get_i420_buffer_size(const webrtc::I420BufferInterface* data)
{
	return get_i420_buffer_size((webrtc::I420BufferInterface*)data);
}
