//
//  audio_capture_module.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/22.
//

#include "audio_device_module.h"

rtc::scoped_refptr<IAudioDeviceModule> IAudioDeviceModule::Create()
{
    return rtc::make_ref_counted<IAudioDeviceModule>();
}

int32_t IAudioDeviceModule::ActiveAudioLayer(AudioLayer* audio_layer) const
{
    return 0;
}

int32_t IAudioDeviceModule::RegisterAudioCallback(webrtc::AudioTransport* _audio_transport)
{
    return 0;
}

int32_t IAudioDeviceModule::Init()
{
    _initialized = true;
    return 0;
}

int32_t IAudioDeviceModule::Terminate()
{
    _initialized = false;
    return 0;
}

bool IAudioDeviceModule::Initialized() const
{
    return _initialized;
}

int16_t IAudioDeviceModule::PlayoutDevices()
{
    return 0;
}

int16_t IAudioDeviceModule::RecordingDevices()
{
    return 0;
}

int32_t IAudioDeviceModule::PlayoutDeviceName(uint16_t index,
                                              char name[webrtc::kAdmMaxDeviceNameSize],
                                              char guid[webrtc::kAdmMaxGuidSize])
{
    return 0;
}

int32_t IAudioDeviceModule::RecordingDeviceName(uint16_t index,
                                                char name[webrtc::kAdmMaxDeviceNameSize],
                                                char guid[webrtc::kAdmMaxGuidSize])
{
    return 0;
}

int32_t IAudioDeviceModule::SetPlayoutDevice(uint16_t index)
{
    return 0;
}

int32_t IAudioDeviceModule::SetPlayoutDevice(WindowsDeviceType device)
{
    return 0;
}

int32_t IAudioDeviceModule::SetRecordingDevice(uint16_t index)
{
    return 0;
}

int32_t IAudioDeviceModule::SetRecordingDevice(WindowsDeviceType device)
{
    return 0;
}

int32_t IAudioDeviceModule::PlayoutIsAvailable(bool* available)
{
    return 0;
}

int32_t IAudioDeviceModule::InitPlayout()
{
    return 0;
}

bool IAudioDeviceModule::PlayoutIsInitialized() const
{
    return _initialized;
}

int32_t IAudioDeviceModule::RecordingIsAvailable(bool* available)
{
    *available = false;
    return 0;
}

int32_t IAudioDeviceModule::InitRecording()
{
    return 0;
}

bool IAudioDeviceModule::RecordingIsInitialized() const
{
    return false;
}

int32_t IAudioDeviceModule::StartPlayout()
{
    _playing = true;
    return 0;
}

int32_t IAudioDeviceModule::StopPlayout()
{
    _playing = false;
    return 0;
}

bool IAudioDeviceModule::Playing() const
{
    return _playing;
}

int32_t IAudioDeviceModule::StartRecording()
{
    return 0;
}

int32_t IAudioDeviceModule::StopRecording()
{
    return 0;
}

bool IAudioDeviceModule::Recording() const
{
    return false;
}

int32_t IAudioDeviceModule::InitSpeaker()
{
    return 0;
}

bool IAudioDeviceModule::SpeakerIsInitialized() const
{
    return true;
}

int32_t IAudioDeviceModule::InitMicrophone()
{
    return 0;
}

bool IAudioDeviceModule::MicrophoneIsInitialized() const
{
    return false;
}

int32_t IAudioDeviceModule::SpeakerVolumeIsAvailable(bool* available)
{
    *available = true;
    return 0;
}

int32_t IAudioDeviceModule::SetSpeakerVolume(uint32_t volume)
{
    return 0;
}

int32_t IAudioDeviceModule::SpeakerVolume(uint32_t* volume) const
{
    *volume = 0;
    return 0;
}

int32_t IAudioDeviceModule::MaxSpeakerVolume(uint32_t* max_volume) const
{
    *max_volume = 0;
    return 0;
}

int32_t IAudioDeviceModule::MinSpeakerVolume(uint32_t* min_volume) const
{
    *min_volume = 0;
    return 0;
}

int32_t IAudioDeviceModule::MicrophoneVolumeIsAvailable(bool* available)
{
    return 0;
}

int32_t IAudioDeviceModule::SetMicrophoneVolume(uint32_t volume)
{
    return 0;
}

int32_t IAudioDeviceModule::MicrophoneVolume(uint32_t* volume) const
{
    return 0;
}

int32_t IAudioDeviceModule::MaxMicrophoneVolume(uint32_t* max_volume) const
{
    return 0;
}

int32_t IAudioDeviceModule::MinMicrophoneVolume(uint32_t* min_volume) const
{
    return 0;
}

int32_t IAudioDeviceModule::SpeakerMuteIsAvailable(bool* available)
{
    *available = true;
    return 0;
}

int32_t IAudioDeviceModule::SetSpeakerMute(bool enable)
{
    return 0;
}

int32_t IAudioDeviceModule::SpeakerMute(bool* enabled) const
{
    *enabled = false;
    return 0;
}

int32_t IAudioDeviceModule::MicrophoneMuteIsAvailable(bool* available)
{
    return 0;
}

int32_t IAudioDeviceModule::SetMicrophoneMute(bool enable)
{
    return 0;
}

int32_t IAudioDeviceModule::MicrophoneMute(bool* enabled) const
{
    return 0;
}

int32_t IAudioDeviceModule::StereoPlayoutIsAvailable(bool* available) const
{
    *available = true;
    return 0;
}

int32_t IAudioDeviceModule::SetStereoPlayout(bool enable)
{
    return 0;
}

int32_t IAudioDeviceModule::StereoPlayout(bool* enabled) const
{
    *enabled = true;
    return 0;
}

int32_t IAudioDeviceModule::StereoRecordingIsAvailable(bool* available) const
{
    *available = false;
    return 0;
}

int32_t IAudioDeviceModule::SetStereoRecording(bool enable)
{
    return 0;
}

int32_t IAudioDeviceModule::StereoRecording(bool* enabled) const
{
    return 0;
}

int32_t IAudioDeviceModule::PlayoutDelay(uint16_t* delay_ms) const
{
    *delay_ms = 0;
    return 0;
}

bool IAudioDeviceModule::BuiltInAECIsAvailable() const
{
    return false;
}

bool IAudioDeviceModule::BuiltInAGCIsAvailable() const
{
    return false;
}

bool IAudioDeviceModule::BuiltInNSIsAvailable() const
{
    return false;
}

int32_t IAudioDeviceModule::EnableBuiltInAEC(bool enable)
{
    return 0;
}

int32_t IAudioDeviceModule::EnableBuiltInAGC(bool enable)
{
    return 0;
}

int32_t IAudioDeviceModule::EnableBuiltInNS(bool enable)
{
    return 0;
}

#if defined(WEBRTC_IOS)
int IAudioDeviceModule::GetPlayoutAudioParameters(webrtc::AudioParameters* params) const
{
    return 0;
}

int IAudioDeviceModule::GetRecordAudioParameters(webrtc::AudioParameters* params) const
{
    return 0;
}
#endif  // WEBRTC_IOS
