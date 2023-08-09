//
//  thread.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/8/8.
//

#include "thread.h"

RtcThreads::RtcThreads()
    : work_thread(rtc::Thread::Create())
    , network_thread(rtc::Thread::CreateWithSocketServer())
    , signaling_thread(rtc::Thread::CreateWithSocketServer())
{ 
    work_thread->Start();
    network_thread->Start();
    signaling_thread->Start();
}

RtcThreads::~RtcThreads()
{
    work_thread->Stop();
    network_thread->Stop();
    signaling_thread->Stop();
}

std::unique_ptr<RtcThreads> RtcThreads::Create()
{
    return std::make_unique<RtcThreads>();
}