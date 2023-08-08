//
//  base.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_BASE_H
#define LIBRTC_BASE_H
#pragma once

#ifdef WIN32
#define EXPORT __declspec(dllexport)
#endif

#ifdef WEBRTC_POSIX
#define EXPORT
#endif

#include <string>

void free_incomplete_ptr(void* ptr);
char* copy_c_str(std::string& source);

#endif  // LIBRTC_BASE_H
