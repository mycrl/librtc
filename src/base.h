//
//  base.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef librtc_base_h
#define librtc_base_h
#pragma once

#ifdef WIN32
#define EXPORT __declspec(dllexport)
#endif

#ifdef WEBRTC_POSIX
#define EXPORT
#endif

void free_incomplete_ptr(void* ptr);

#endif  // librtc_base_h
