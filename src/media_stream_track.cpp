//
//  media_stream_track.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "api/video/i420_buffer.h"
#include "media_stream_track.h"

void rtc_free_media_stream_track(MediaStreamTrack* track)
{
    free_incomplete_ptr(track->label);
    free_incomplete_ptr(track);
}

MediaStreamTrack* from(webrtc::VideoTrackInterface* itrack)
{
    MediaStreamTrack* track = (MediaStreamTrack*)malloc(sizeof(MediaStreamTrack));
    if (!track)
    {
        return NULL;
    }
    
    track->video_sink = IVideoTrackSink::Create(itrack);
    if (!track->video_sink)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    
    auto id = itrack->id();
    track->label = (char*)malloc(sizeof(char) * id.size() + 1);
    if (!track->label)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    
    strcpy(track->label, id.c_str());
    
    track->kind = MediaStreamTrackKindVideo;
    return track;
}

MediaStreamTrack* from(webrtc::AudioTrackInterface* itrack)
{
    MediaStreamTrack* track = (MediaStreamTrack*)malloc(sizeof(MediaStreamTrack));
    if (!track)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    
    track->audio_sink = IAudioTrackSink::Create(itrack);
    if (!track->audio_sink)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    
    auto id = itrack->id();
    track->label = (char*)malloc(sizeof(char) * id.size() + 1);
    if (!track->label)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    
    strcpy(track->label, id.c_str());
    
    track->kind = MediaStreamTrackKindAudio;
    return track;
}

void rtc_add_video_track_frame(MediaStreamTrack* track, IVideoFrame* frame)
{
    if (!track->video_source) {
        return;
    }
    
    track->video_source->AddFrame(from_c(frame));
}

void rtc_set_video_track_frame_h(
                                 MediaStreamTrack* track,
                                 void(handler)(void* ctx, IVideoFrame* frame),
                                 void* ctx)
{
    if (!track->video_sink) {
        return;
    }
    
    track->video_sink->SetOnFrame(ctx, handler);
}

MediaStreamTrack* rtc_create_video_track(char* label)
{
    MediaStreamTrack* track = (MediaStreamTrack*)malloc(sizeof(MediaStreamTrack));
    if (!track)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    
    track->video_source = IVideoTrackSource::Create();
    if (!track->video_source)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    
    track->label = (char*)malloc(sizeof(char) * (strlen(label) + 1));
    if (!track->label)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    else
    {
        strcpy(track->label, label);
    }
    
    track->kind = MediaStreamTrackKindVideo;
    return track;
}

MediaStreamTrack* rtc_create_audio_track(char* label)
{
    MediaStreamTrack* track = (MediaStreamTrack*)malloc(sizeof(MediaStreamTrack));
    if (!track)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    
    track->audio_source = IAudioTrackSource::Create();
    if (!track->audio_source)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    
    track->label = (char*)malloc(sizeof(char) * (strlen(label) + 1));
    if (!track->label)
    {
        rtc_free_media_stream_track(track);
        return NULL;
    }
    else
    {
        strcpy(track->label, label);
    }
    
    track->kind = MediaStreamTrackKindAudio;
    return track;
}

void rtc_add_audio_track_frame(MediaStreamTrack* track, IAudioFrame* frame)
{
    if (!track->audio_source)
    {
        return;
    }
    
    track->audio_source->OnData(frame->buf,
                                frame->frames,
                                frame->channels,
                                frame->bits_per_sample,
                                frame->sample_rate,
                                frame->ms);
}

void rtc_set_audio_track_frame_h(
                                 MediaStreamTrack* track,
                                 void(handler)(void* ctx, IAudioFrame* frame),
                                 void* ctx)
{
    track->audio_sink->SetOnFrame(ctx, handler);
}

void rtc_remove_media_stream_track_frame_h(MediaStreamTrack* track)
{
    if (track->video_sink)
    {
        track->video_sink->RemoveOnFrame();
    }
    
    if (track->audio_sink)
    {
        track->audio_sink->RemoveOnFrame();
    }
}
