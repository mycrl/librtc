//
//  video_track.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "api/video/i420_buffer.h"
#include "video_track.h"

/* IVideoSource */

void IVideoSource::AddOrUpdateSink(
                                   rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
                                   const rtc::VideoSinkWants& wants)
{
    _broadcaster.AddOrUpdateSink(sink, wants);
}

void IVideoSource::RemoveSink(
                              rtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
{
    _broadcaster.RemoveSink(sink);
}

void IVideoSource::AddFrame(const webrtc::VideoFrame& original_frame)
{
    auto frame = _MaybePreprocess(original_frame);
    auto ret = _AdaptFrameResolution(frame);
    if (!ret.drop)
    {
        return;
    }
    
    if (ret.resize)
    {
        _broadcaster.OnFrame(_ScaleFrame(frame, ret));
    }
    else
    {
        _broadcaster.OnFrame(frame);
    }
}

webrtc::VideoFrame IVideoSource::_MaybePreprocess(
                                                  const webrtc::VideoFrame& frame)
{
    webrtc::MutexLock lock(&_lock);
    if (_preprocessor != nullptr)
    {
        return _preprocessor->Preprocess(frame);
    }
    else
    {
        return frame;
    }
}

webrtc::VideoFrame IVideoSource::_ScaleFrame(
                                             const webrtc::VideoFrame& original_frame,
                                             AdaptFrameResult& ret)
{
    auto scaled_buffer = webrtc::I420Buffer::Create(ret.width, ret.height);
    scaled_buffer->ScaleFrom(*original_frame.video_frame_buffer()->ToI420());
    auto new_frame_builder = webrtc::VideoFrame::Builder()
        .set_video_frame_buffer(scaled_buffer)
        .set_rotation(webrtc::VideoRotation::kVideoRotation_0)
        .set_timestamp_us(original_frame.timestamp_us())
        .set_id(original_frame.id());
    
    if (!original_frame.has_update_rect())
    {
        return new_frame_builder.build();
    }
    
    auto rect = original_frame.update_rect().ScaleWithFrame(
                                                            original_frame.width(),
                                                            original_frame.height(),
                                                            0,
                                                            0,
                                                            original_frame.width(),
                                                            original_frame.height(),
                                                            ret.width,
                                                            ret.height);
    new_frame_builder.set_update_rect(rect);
    return new_frame_builder.build();
}

AdaptFrameResult IVideoSource::_AdaptFrameResolution(
                                                     const webrtc::VideoFrame& frame)
{
    AdaptFrameResult ret;
    ret.drop = _video_adapter.AdaptFrameResolution(
                                                   frame.width(),
                                                   frame.height(),
                                                   frame.timestamp_us() * 1000,
                                                   &ret.cropped_width,
                                                   &ret.cropped_height,
                                                   &ret.width,
                                                   &ret.height);
    ret.resize = ret.height != frame.height() ||
    ret.width != frame.width();
    return ret;
}

/* IVideoTrackSource */

IVideoTrackSource* IVideoTrackSource::Create()
{
    auto self = new rtc::RefCountedObject<IVideoTrackSource>();
    self->AddRef();
    return self;
}

void IVideoTrackSource::AddFrame(const webrtc::VideoFrame& frame)
{
    _source.AddFrame(frame);
}

rtc::VideoSourceInterface<webrtc::VideoFrame>* IVideoTrackSource::source()
{
    return static_cast<rtc::VideoSourceInterface<webrtc::VideoFrame>*>(&_source);
}

/* IVideoTrackSink */

IVideoTrackSink::IVideoTrackSink(webrtc::VideoTrackInterface* track)
{
    _ctx = NULL;
    _track = track;
    _track->AddRef();
}

IVideoTrackSink* IVideoTrackSink::Create(webrtc::VideoTrackInterface* track)
{
    auto self = new rtc::RefCountedObject<IVideoTrackSink>(track);
    self->AddRef();
    return self;
}

void IVideoTrackSink::OnFrame(const webrtc::VideoFrame& frame)
{
    if (!_handler)
    {
        return;
    }
    
    auto i420_frame = into_c((webrtc::VideoFrame*)&frame);
    if (!i420_frame)
    {
        return;
    }
    
    _handler(_ctx, i420_frame);
}

void IVideoTrackSink::SetOnFrame(void* ctx,
                                 void(*handler)(void* ctx, IVideoFrame* frame))
{
    _track->AddOrUpdateSink(this, _wants);
    _handler = handler;
    _ctx = ctx;
}

void IVideoTrackSink::RemoveOnFrame()
{
    _track->RemoveSink(this);
    _handler = NULL;
    _ctx = NULL;
}
