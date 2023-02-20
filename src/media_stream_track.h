#ifndef BATRACHIATC_MEDIA_STREAM_TRACK_H_
#define BATRACHIATC_MEDIA_STREAM_TRACK_H_
#pragma once

#include "rtc_base/synchronization/mutex.h"
#include "media/base/video_broadcaster.h"
#include "api/video/video_frame_buffer.h"
#include "api/media_stream_interface.h"
#include "media/base/video_adapter.h"
#include "pc/video_track_source.h"
#include "frame.h"
#include "base.h"

/*
 video source
 */

typedef struct
{
    int cropped_width = 0;
    int cropped_height = 0;
    int width = 0;
    int height = 0;
    bool drop;
    bool resize;
} AdaptFrameResult;

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

/*
 audio source
 */
class IAudioTrackSource
: public rtc::RefCountedObject<webrtc::AudioSourceInterface>
{
public:
    void AddSink(webrtc::AudioTrackSinkInterface* sink);
    void RemoveSink(webrtc::AudioTrackSinkInterface* sink);
    void AddData(const uint16_t* buf, size_t size, size_t frames_size);
    
    size_t number_of_channels;
    int bits_per_sample;
    int sample_rate;
private:
    std::vector<webrtc::AudioTrackSinkInterface*> _sinks;
    std::vector<webrtc::ObserverInterface*> _observers;
};

/*
 audio sink
 */

class IAudioTrackSink
: public webrtc::AudioTrackSinkInterface
, public rtc::RefCountInterface
{
public:
    IAudioTrackSink(webrtc::AudioTrackInterface* track);
    static IAudioTrackSink* Create(webrtc::AudioTrackInterface* track);
    void OnData(const void* buf, int b, int s, size_t c, size_t f);
    void SetOnFrame(void* ctx, void(*handler)(void* ctx, IAudioFrame* frame));
    void RemoveOnFrame();
private:
    void(*_handler)(void* ctx, IAudioFrame* frame) = NULL;
    webrtc::AudioTrackInterface* _track = NULL;
    void* _ctx = NULL;
};

/*
 extern
 */

typedef enum {
    MediaStreamTrackKindVideo,
    MediaStreamTrackKindAudio,
} MediaStreamTrackKind;

/*
 MediaStreamTrack
 
 The MediaStreamTrack interface represents a single media track within a stream;
 typically, these are audio or video tracks, but other track types may exist as
 well.
 */
typedef struct {
    /*
     Returns a string set to "audio" if the track is an audio track and to
     "video", if it is a video track. It doesn't change if the track is
     disassociated from its source.
     */
    MediaStreamTrackKind kind;
    /*
     Returns a string containing a user agent-assigned label that identifies the
     track source, as in "internal microphone". The string may be left empty and
     is empty as long as no source has been connected. When the track is
     disassociated from its source, the label is not changed.
     */
    char* label;
    
    /* --------------- video --------------- */
    IVideoTrackSource* video_source;
    IVideoTrackSink* video_sink;
    
    /* --------------- audio --------------- */
    IAudioTrackSource* audio_source;
    IAudioTrackSink* audio_sink;
} MediaStreamTrack;

extern "C" EXPORT void rtc_set_video_track_frame_h(
                                                   MediaStreamTrack * track,
                                                   void(handler)(void* ctx, IVideoFrame * frame),
                                                   void* ctx);

extern "C" EXPORT void rtc_set_audio_track_frame_h(
                                                   MediaStreamTrack* track,
                                                   void(handler)(void* ctx, IAudioFrame* frame),
                                                   void* ctx);

extern "C" EXPORT void rtc_add_video_track_frame(
                                                 MediaStreamTrack * track, 
                                                 IVideoFrame* frame);

extern "C" EXPORT void rtc_remove_media_stream_track_frame_h(
                                                             MediaStreamTrack * track);

extern "C" EXPORT MediaStreamTrack* rtc_create_video_track(char* label);
extern "C" EXPORT void rtc_free_media_stream_track(MediaStreamTrack * track);

MediaStreamTrack* from(webrtc::VideoTrackInterface* track);
MediaStreamTrack* from(webrtc::AudioTrackInterface* track);

#endif  // BATRACHIATC_MEDIA_STREAM_TRACK_H_
