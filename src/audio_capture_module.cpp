//
//  audio_capture_module.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/22.
//

#include "audio_capture_module.h"

rtc::scoped_refptr<AudioCaptureModule> AudioCaptureModule::Create()
{
    return rtc::make_ref_counted<AudioCaptureModule>();
}

int32_t AudioCaptureModule::ActiveAudioLayer(AudioLayer* audio_layer) const
{
    return 0;
}

int32_t AudioCaptureModule::RegisterAudioCallback(
                                                  webrtc::AudioTransport* _audio_transport)
{
    return 0;
}

int32_t AudioCaptureModule::Init()
{
    _initialized = true;
    return 0;
}

int32_t AudioCaptureModule::Terminate()
{
    _initialized = false;
    return 0;
}

bool AudioCaptureModule::Initialized() const
{
    return _initialized;
}

int16_t AudioCaptureModule::PlayoutDevices()
{
    return 0;
}

int16_t AudioCaptureModule::RecordingDevices()
{
    return 0;
}

int32_t AudioCaptureModule::PlayoutDeviceName(
                                              uint16_t index,
                                              char name[webrtc::kAdmMaxDeviceNameSize],
                                              char guid[webrtc::kAdmMaxGuidSize])
{
    return 0;
}

int32_t AudioCaptureModule::RecordingDeviceName(
                                                uint16_t index,
                                                char name[webrtc::kAdmMaxDeviceNameSize],
                                                char guid[webrtc::kAdmMaxGuidSize])
{
    return 0;
}

int32_t AudioCaptureModule::SetPlayoutDevice(uint16_t index)
{
    return 0;
}

int32_t AudioCaptureModule::SetPlayoutDevice(WindowsDeviceType device)
{
    return 0;
}

int32_t AudioCaptureModule::SetRecordingDevice(uint16_t index)
{
    return 0;
}

int32_t AudioCaptureModule::SetRecordingDevice(WindowsDeviceType device)
{
    return 0;
}

int32_t AudioCaptureModule::PlayoutIsAvailable(bool* available)
{
    return 0;
}

int32_t AudioCaptureModule::InitPlayout()
{
    return 0;
}

bool AudioCaptureModule::PlayoutIsInitialized() const
{
    return _initialized;
}

int32_t AudioCaptureModule::RecordingIsAvailable(bool* available)
{
    return 0;
}

int32_t AudioCaptureModule::InitRecording()
{
    return 0;
}

bool AudioCaptureModule::RecordingIsInitialized() const
{
    return false;
}

int32_t AudioCaptureModule::StartPlayout()
{
    _playing = true;
    return 0;
}

int32_t AudioCaptureModule::StopPlayout()
{
    _playing = false;
    return 0;
}

bool AudioCaptureModule::Playing() const
{
    return _playing;
}

int32_t AudioCaptureModule::StartRecording()
{
    return 0;
}

int32_t AudioCaptureModule::StopRecording()
{
    return 0;
}

bool AudioCaptureModule::Recording() const
{
    return false;
}

int32_t AudioCaptureModule::InitSpeaker()
{
    return 0;
}

bool AudioCaptureModule::SpeakerIsInitialized() const
{
    return false;
}

int32_t AudioCaptureModule::InitMicrophone()
{
    return 0;
}

bool AudioCaptureModule::MicrophoneIsInitialized() const
{
    return false;
}

int32_t AudioCaptureModule::SpeakerVolumeIsAvailable(bool* available)
{
    return 0;
}

int32_t AudioCaptureModule::SetSpeakerVolume(uint32_t volume)
{
    return 0;
}

int32_t AudioCaptureModule::SpeakerVolume(uint32_t* volume) const
{
    return 0;
}

int32_t AudioCaptureModule::MaxSpeakerVolume(uint32_t* max_volume) const
{
    return 0;
}

int32_t AudioCaptureModule::MinSpeakerVolume(uint32_t* min_volume) const
{
    return 0;
}

int32_t AudioCaptureModule::MicrophoneVolumeIsAvailable(bool* available)
{
    return 0;
}

int32_t AudioCaptureModule::SetMicrophoneVolume(uint32_t volume)
{
    return 0;
}

int32_t AudioCaptureModule::MicrophoneVolume(uint32_t* volume) const
{
    return 0;
}

int32_t AudioCaptureModule::MaxMicrophoneVolume(uint32_t* max_volume) const
{
    return 0;
}

int32_t AudioCaptureModule::MinMicrophoneVolume(uint32_t* min_volume) const
{
    return 0;
}

int32_t AudioCaptureModule::SpeakerMuteIsAvailable(bool* available)
{
    return 0;
}

int32_t AudioCaptureModule::SetSpeakerMute(bool enable)
{
    return 0;
}

int32_t AudioCaptureModule::SpeakerMute(bool* enabled) const
{
    return 0;
}

int32_t AudioCaptureModule::MicrophoneMuteIsAvailable(bool* available)
{
    return 0;
}

int32_t AudioCaptureModule::SetMicrophoneMute(bool enable)
{
    return 0;
}

int32_t AudioCaptureModule::MicrophoneMute(bool* enabled) const
{
    return 0;
}

int32_t AudioCaptureModule::StereoPlayoutIsAvailable(bool* available) const
{
    *available = true;
    return 0;
}

int32_t AudioCaptureModule::SetStereoPlayout(bool enable)
{
    return 0;
}

int32_t AudioCaptureModule::StereoPlayout(bool* enabled) const
{
    return 0;
}

int32_t AudioCaptureModule::StereoRecordingIsAvailable(bool* available) const
{
    *available = false;
    return 0;
}

int32_t AudioCaptureModule::SetStereoRecording(bool enable)
{
    return 0;
}

int32_t AudioCaptureModule::StereoRecording(bool* enabled) const
{
    return 0;
}

int32_t AudioCaptureModule::PlayoutDelay(uint16_t* delay_ms) const
{
    *delay_ms = 0;
    return 0;
}

bool AudioCaptureModule::BuiltInAECIsAvailable() const
{
    return false;
}

bool AudioCaptureModule::BuiltInAGCIsAvailable() const
{
    return false;
}

bool AudioCaptureModule::BuiltInNSIsAvailable() const
{
    return false;
}

int32_t AudioCaptureModule::EnableBuiltInAEC(bool enable)
{
    return 0;
}

int32_t AudioCaptureModule::EnableBuiltInAGC(bool enable)
{
    return 0;
}

int32_t AudioCaptureModule::EnableBuiltInNS(bool enable)
{
    return 0;
}

#if defined(WEBRTC_IOS)
int AudioCaptureModule::GetPlayoutAudioParameters(
                                                  webrtc::AudioParameters* params) const
{
    return 0;
}

int AudioCaptureModule::GetRecordAudioParameters(
                                                 webrtc::AudioParameters* params) const
{
    return 0;
}
#endif  // WEBRTC_IOS
