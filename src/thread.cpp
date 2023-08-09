//
//  thread.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/8/8.
//

#include "thread.h"

RtcThreads::RtcThreads()
    : worker_thread(rtc::Thread::Create())
    , network_thread(rtc::Thread::CreateWithSocketServer())
    , signaling_thread(rtc::Thread::CreateWithSocketServer())
    , task_queue_factory(webrtc::CreateDefaultTaskQueueFactory())
{
    worker_thread->SetName("librtc::worker_thread", nullptr);
    network_thread->SetName("librtc::network_thread", nullptr);
    signaling_thread->SetName("librtc::signaling_thread", nullptr);
    worker_thread->Start();
    network_thread->Start();
    signaling_thread->Start();
}

RtcThreads::~RtcThreads()
{
    worker_thread->Stop();
    network_thread->Stop();
    signaling_thread->Stop();
}

std::unique_ptr<RtcThreads> RtcThreads::Create()
{
    return std::make_unique<RtcThreads>();
}