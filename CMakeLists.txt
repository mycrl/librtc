cmake_minimum_required(VERSION 3.16)
project(rtc)

# Version 110 of webrtc native requires at least c++ 17 support.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Default build target is Debug.
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Used to download or initialize third-party dependent libraries.
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# Check whether the third-party dependent library directory is created.
if (NOT EXISTS "${THIRD_PARTY_DIR}")
    execute_process(COMMAND mkdir "${THIRD_PARTY_DIR}")
endif()

# WebRTC Native Module.

# Clone the depot_tools repository.
if (NOT EXISTS "${THIRD_PARTY_DIR}/depot_tools")
    execute_process(
        WORKING_DIRECTORY "${THIRD_PARTY_DIR}"
        COMMAND bash -c "git clone \
                         https://chromium.googlesource.com/chromium/tools/depot_tools.git")
endif()

# On Windows operating systems, the PATH environment variable uses a 
# semicolon ; to separate paths. On Linux and macOS operating systems, 
# use a colon : to separate paths.
if (WIN32)
    set(ENV{PATH} "$ENV{PATH};${THIRD_PARTY_DIR}/depot_tools")
else()
    set(ENV{PATH} "$ENV{PATH}:${THIRD_PARTY_DIR}/depot_tools")
endif()

# Sync webrtc toolchain and many dependencies. The checkout size is large due the 
# use of the Chromium build toolchain and many dependencies. Estimated size:
#
# - Linux: 6.4 GB.
# - Linux (with Android): 16 GB (of which ~8 GB is Android SDK+NDK images).
# - Mac (with iOS support): 5.6GB
if (NOT EXISTS "${THIRD_PARTY_DIR}/webrtc")
    # Get webrtc native source code and synchronize third-party dependencies.
    execute_process(COMMAND mkdir "${THIRD_PARTY_DIR}/webrtc")
    execute_process(
        WORKING_DIRECTORY "${THIRD_PARTY_DIR}/webrtc"
        COMMAND bash -c "fetch --nohooks webrtc \
                         && cd src \
                         && git checkout branch-heads/5481 \
                         && gclient sync")
endif()

# Compile the Debug/Release target of the webrtc static library.
if (NOT EXISTS "${THIRD_PARTY_DIR}/webrtc/src/out/${CMAKE_BUILD_TYPE}")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(DEBUG "false")
    else()
        set(DEBUG "true")
    endif()

    # Compile with native toolchain (only Windows).
    if (WIN32)
        set(DEPOT_TOOLS_WIN_TOOLCHAIN "0")
    endif()

    execute_process(
        WORKING_DIRECTORY "${THIRD_PARTY_DIR}/webrtc/src"
        COMMAND bash -c "gn gen out/${CMAKE_BUILD_TYPE} --args='is_debug=${DEBUG} \
                                                                is_component_build=false \
                                                                use_lld=false \
                                                                treat_warnings_as_errors=false \
                                                                use_rtti=true \
                                                                rtc_include_tests=false \
                                                                rtc_build_examples=false \
                                                                use_custom_libcxx=false \
                                                                rtc_use_h264=false' \
                        && ninja -C out/${CMAKE_BUILD_TYPE}")

endif()

# FFmpeg

if(APPLE)
	execute_process(COMMAND bash -c "brew --prefix ffmpeg"
                    OUTPUT_VARIABLE FFMPEG_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(FFMPEG_PREFIX ${FFMPEG_PREFIX})
else()
    set(FFMPEG_PREFIX $ENV{FFMPEG_PREFIX})
endif()

# The compilation target is a dynamic library.
add_library(rtc SHARED
	src/base.h
	src/base.cpp
	src/frame.h
	src/frame.cpp
	src/media_stream_track.h
	src/media_stream_track.cpp
	src/video_track.h
	src/video_track.cpp
	src/audio_track.h
	src/audio_track.cpp
	src/audio_capture_module.h
    src/audio_capture_module.cpp
	src/observer.cpp
	src/observer.h
	src/peer_connection.cpp
	src/peer_connection.h
	src/peer_connection_config.h
	src/peer_connection_config.cpp
	src/session_description.h
	src/session_description.cpp
	src/ice_candidate.h
	src/ice_candidate.cpp
	src/data_channel.h
	src/data_channel.cpp
	src/video_encoder.h
	src/video_encoder.cpp
    src/video_decoder.h
    src/video_decoder.cpp
    src/h264.h
    src/h264.cpp
    src/h264_encoder.h
    src/h264_encoder.cpp
    src/h264_decoder.h
    src/h264_decoder.cpp)

# Add third-party libraries to the search path.
include_directories("${THIRD_PARTY_DIR}/webrtc/src/third_party/libyuv/include"
                    "${THIRD_PARTY_DIR}/webrtc/src/third_party/abseil-cpp"
                    "${THIRD_PARTY_DIR}/webrtc/src"
                    "${FFMPEG_PREFIX}/include")

# Add third-party library files.
target_link_directories(rtc PRIVATE 
                        "${THIRD_PARTY_DIR}/webrtc/src/out/${CMAKE_BUILD_TYPE}/obj"
						"${FFMPEG_PREFIX}/lib")
target_link_libraries(rtc avcodec
                      avutil
                      webrtc)

# If it is WIN32, you need to set the runtime library.
if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_property(TARGET rtc PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded)
    else()
        set_property(TARGET rtc PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDebug)
    endif()
endif()

# Debug/Release target in build header.
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(NDEBUG)
else()
    add_compile_definitions(DEBUG)
endif()

# Build header.
if(WIN32)
    add_compile_definitions(_CONSOLE
                            USE_AURA=1
                            _HAS_EXCEPTIONS=0
                            __STD_C
                            _CRT_RAND_S
                            _CRT_SECURE_NO_DEPRECATE
                            _SCL_SECURE_NO_DEPRECATE
                            _ATL_NO_OPENGL
                            _WINDOWS
                            CERT_CHAIN_PARA_HAS_EXTRA_FIELDS
                            PSAPI_VERSION=2
                            WIN32
                            _SECURE_ATL
                            WINUWP
                            __WRL_NO_DEFAULT_LIB__
                            WINAPI_FAMILY=WINAPI_FAMILY_PC_APP
                            WIN10=_WIN32_WINNT_WIN10
                            WIN32_LEAN_AND_MEAN
                            NOMINMAX
                            _UNICODE
                            UNICODE
                            NTDDI_VERSION=NTDDI_WIN10_RS2
                            _WIN32_WINNT=0x0A00
                            WINVER=0x0A00
                            NVALGRIND
                            DYNAMIC_ANNOTATIONS_ENABLED=0
                            WEBRTC_ENABLE_PROTOBUF=0
                            WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE
                            RTC_ENABLE_VP9
                            HAVE_SCTP
                            WEBRTC_LIBRARY_IMPL
                            WEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=0
                            WEBRTC_WIN
                            ABSL_ALLOCATOR_NOTHROW=1
                            WEBRTC_VIDEO_CAPTURE_WINRT)
elseif(APPLE)
    add_compile_definitions(WEBRTC_MAC
                            WEBRTC_IOS
                            WEBRTC_POSIX)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(WEBRTC_POSIX)
endif()

# Introduce other runtime necessary library files.
if(WIN32)
    target_link_libraries(rtc
                          winmm
                          secur32
                          msdmo
                          dmoguids
                          wmcodecdspuuid
                          iphlpapi)
elseif(APPLE)
    target_link_libraries(rtc
                          c++
                          "-framework Foundation"
                          "-framework AudioToolbox"
                          "-framework AudioUnit"
                          "-framework CoreServices"
                          "-framework CoreFoundation"
                          "-framework CoreAudio"
                          "-framework CoreGraphics")
elseif(UNIX AND NOT APPLE)
    target_link_libraries(rtc
                          stdc++
                          X11)
endif()
