//
//  media_stream_track.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef librtc_media_stream_track_h
#define librtc_media_stream_track_h
#pragma once

#include "video_track.h"
#include "audio_track.h"
#include "base.h"

typedef enum {
    MediaStreamTrackKindVideo,
    MediaStreamTrackKindAudio,
} MediaStreamTrackKind;


// MediaStreamTrack
//
// The MediaStreamTrack interface represents a single media track within a stream;
// typically, these are audio or video tracks, but other track types may exist as
// well.
typedef struct {
    // Returns a string set to "audio" if the track is an audio track and to
    // "video", if it is a video track. It doesn't change if the track is
    // disassociated from its source.
    MediaStreamTrackKind kind;
    
    // Returns a string containing a user agent-assigned label that identifies the
    // track source, as in "internal microphone". The string may be left empty and
    // is empty as long as no source has been connected. When the track is
    // disassociated from its source, the label is not changed.
    char* label;
    
    /* --------------- video --------------- */
    IVideoTrackSource* video_source;
    IVideoTrackSink* video_sink;
    
    /* --------------- audio --------------- */
    IAudioTrackSource* audio_source;
    IAudioTrackSink* audio_sink;
} MediaStreamTrack;

extern "C" EXPORT void rtc_free_media_stream_track(MediaStreamTrack * track);
extern "C" EXPORT void rtc_remove_media_stream_track_frame_h(MediaStreamTrack * track);

/* video track */

extern "C" EXPORT MediaStreamTrack* rtc_create_video_track(char* label);
extern "C" EXPORT void rtc_set_video_track_frame_h(MediaStreamTrack * track,
                                                   void(handler)(void* ctx, IVideoFrame * frame),
                                                   void* ctx);
extern "C" EXPORT void rtc_add_video_track_frame(MediaStreamTrack * track,
                                                 IVideoFrame* frame);

/* audio track */

extern "C" EXPORT MediaStreamTrack* rtc_create_audio_track(char* label);
extern "C" EXPORT void rtc_set_audio_track_frame_h(MediaStreamTrack* track,
                                                   void(handler)(void* ctx, IAudioFrame* frame),
                                                   void* ctx);
extern "C" EXPORT void rtc_add_audio_track_frame(MediaStreamTrack * track,
                                                 IAudioFrame* frame);

MediaStreamTrack* from(webrtc::VideoTrackInterface* track);
MediaStreamTrack* from(webrtc::AudioTrackInterface* track);

#endif  // librtc_media_stream_track_h
