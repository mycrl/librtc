//
//  video_track.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef video_track_h
#define video_track_h
#pragma once

#include "rtc_base/synchronization/mutex.h"
#include "media/base/video_broadcaster.h"
#include "api/video/video_frame_buffer.h"
#include "media/base/video_adapter.h"
#include "pc/video_track_source.h"
#include "frame.h"
#include "base.h"

/*
 video source
 */

struct AdaptFrameResult
{
    int cropped_width = 0;
    int cropped_height = 0;
    int width = 0;
    int height = 0;
    bool drop;
    bool resize;
};

class FramePreprocessor
{
public:
    virtual ~FramePreprocessor() = default;
    virtual webrtc::VideoFrame Preprocess(const webrtc::VideoFrame& frame) = 0;
};

class IVideoSource
: public rtc::VideoSourceInterface<webrtc::VideoFrame>
{
public:
    void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink, const rtc::VideoSinkWants& wants);
    void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink);
    void AddFrame(const webrtc::VideoFrame& original_frame);
private:
    webrtc::VideoFrame _MaybePreprocess(const webrtc::VideoFrame& frame);
    AdaptFrameResult _AdaptFrameResolution(const webrtc::VideoFrame& frame);
    webrtc::VideoFrame _ScaleFrame(const webrtc::VideoFrame& original_frame, AdaptFrameResult& ret);
    
    webrtc::Mutex _lock;
    rtc::VideoBroadcaster _broadcaster;
    cricket::VideoAdapter _video_adapter;
    std::unique_ptr<FramePreprocessor> _preprocessor RTC_GUARDED_BY(_lock);
};

class IVideoTrackSource
: public webrtc::VideoTrackSource
{
public:
    IVideoTrackSource(): VideoTrackSource(false) {}
    static IVideoTrackSource* Create();
    void AddFrame(const webrtc::VideoFrame& frame);
    rtc::VideoSourceInterface<webrtc::VideoFrame>* source();
private:
    IVideoSource _source;
};

/*
 video Sink
 */

class IVideoTrackSink
: public rtc::VideoSinkInterface<webrtc::VideoFrame>
, public rtc::RefCountInterface
{
public:
    IVideoTrackSink(webrtc::VideoTrackInterface* track);
    static IVideoTrackSink* Create(webrtc::VideoTrackInterface* track);
    void OnFrame(const webrtc::VideoFrame& frame);
    void SetOnFrame(void* ctx, void(*handler)(void* ctx, IVideoFrame* frame));
    void RemoveOnFrame();
private:
    void(*_handler)(void* ctx, IVideoFrame* frame) = NULL;
    webrtc::VideoTrackInterface* _track;
    rtc::VideoSinkWants _wants;
    void* _ctx;
};

#endif /* video_track_h */
