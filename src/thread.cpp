//
//  thread.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/8/8.
//

#include "thread.h"

RtcThreads::RtcThreads()
    : _work_thread(rtc::Thread::Create())
    , _network_thread(rtc::Thread::CreateWithSocketServer())
    , _signaling_thread(rtc::Thread::CreateWithSocketServer())
{
    _work_thread->Start();
    _network_thread->Start();
    _signaling_thread->Start();
}

RtcThreads::~RtcThreads()
{
    _work_thread->Stop();
    _network_thread->Stop();
    _signaling_thread->Stop();
}

std::unique_ptr<RtcThreads> RtcThreads::Create()
{
    return std::make_unique<RtcThreads>();
}

rtc::Thread* RtcThreads::GetWorkThread()
{
    return _work_thread.get();
}

rtc::Thread* RtcThreads::GetNetworkThread()
{
    return _network_thread.get();
}

rtc::Thread* RtcThreads::GetSignalingThread()
{
    return _signaling_thread.get();
}