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
	assert(track);

	free_ptr(track->label);
	free_ptr(track);
}

MediaStreamTrack* from(webrtc::VideoTrackInterface* itrack)
{
	assert(itrack);

	MediaStreamTrack* track = new MediaStreamTrack;
	track->video_sink = IVideoTrackSink::Create(itrack);
	if (!track->video_sink)
	{
		rtc_free_media_stream_track(track);
		return nullptr;
	}

	auto id = itrack->id();
	track->label = copy_c_str(id);
	if (!track->label)
	{
		rtc_free_media_stream_track(track);
		return nullptr;
	}

	track->kind = MediaStreamTrackKindVideo;
	track->sender = std::nullopt;
	return track;
}

MediaStreamTrack* from(webrtc::AudioTrackInterface* itrack)
{
	assert(itrack);

	MediaStreamTrack* track = new MediaStreamTrack;
	track->audio_sink = IAudioTrackSink::Create(itrack);
	if (!track->audio_sink)
	{
		rtc_free_media_stream_track(track);
		return nullptr;
	}

	auto id = itrack->id();
	track->label = copy_c_str(id);
	if (!track->label)
	{
		rtc_free_media_stream_track(track);
		return nullptr;
	}

	track->kind = MediaStreamTrackKindAudio;
	track->sender = std::nullopt;
	return track;
}

void rtc_add_video_track_frame(MediaStreamTrack* track, IVideoFrame* frame)
{
	assert(track);
	assert(frame);

	if (!track->video_source)
	{
		return;
	}

	track->video_source->AddFrame(from_c(frame));
}

void rtc_set_video_track_frame_h(MediaStreamTrack* track,
								 void(handler)(void* ctx, IVideoFrame* frame),
								 void* ctx)
{
	assert(track);
	assert(handler);

	if (!track->video_sink)
	{
		return;
	}

	track->video_sink->SetOnFrame(ctx, handler);
}

MediaStreamTrack* rtc_create_video_track(char* label)
{
	assert(label);

	MediaStreamTrack* track = new MediaStreamTrack;
	track->video_source = IVideoTrackSource::Create();
	if (!track->video_source)
	{
		rtc_free_media_stream_track(track);
		return nullptr;
	}

	track->label = new char[strlen(label) + 1];
	strcpy(track->label, label);

	track->kind = MediaStreamTrackKindVideo;
	track->sender = std::nullopt;
	return track;
}

MediaStreamTrack* rtc_create_audio_track(char* label)
{
	assert(label);

	MediaStreamTrack* track = new MediaStreamTrack;
	track->audio_source = IAudioTrackSource::Create();
	if (!track->audio_source)
	{
		rtc_free_media_stream_track(track);
		return nullptr;
	}

	track->label = new char[strlen(label) + 1];
	strcpy(track->label, label);

	track->kind = MediaStreamTrackKindAudio;
	track->sender = std::nullopt;
	return track;
}

void rtc_add_audio_track_frame(MediaStreamTrack* track, IAudioFrame* frame)
{
	assert(track);
	assert(frame);

	if (!track->audio_source)
	{
		return;
	}

	track->audio_source->OnData(frame);
}

void rtc_set_audio_track_frame_h(MediaStreamTrack* track,
								 void(handler)(void* ctx, IAudioFrame* frame),
								 void* ctx)
{
	assert(track);
	assert(handler);

	track->audio_sink->SetOnFrame(ctx, handler);
}

void rtc_remove_media_stream_track_frame_h(MediaStreamTrack* track)
{
	assert(track);

	if (track->video_sink)
	{
		track->video_sink->RemoveOnFrame();
	}

	if (track->audio_sink)
	{
		track->audio_sink->RemoveOnFrame();
	}
}
