/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVideoRecording.cpp
 * @brief       Composes the Raspberry Pi video-recording mode.
 * @details     Publishes one configured OpenCV recorder without actuator ownership.
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xAgent_Rpi5CarVideoRecordingOpenCv.h"
#include "xHal_Rpi5CarConfigStore.h"

namespace xwalk::agent
{

/**
 * @brief Runs configured standalone video recording.
 * @param[in,out] context Nullable caller-owned application context.
 * @param[in] callback Non-null synchronous application callback.
 * @param[in,out] config Loaded deployment configuration.
 * @return Status returned by `callback`.
 */
agent::int32 XWalkBootRpi::runVideoRecording(agent::contextpointer context,
    bootapplicationcallback callback, hal::XWalkConfigStore& config)
{
    XWalkVideoRecordingOpenCvConfiguration videoConfiguration;
    videoConfiguration.cameraDevice = config.get(
        "computer_vision_camera_device", "/dev/video0");
    videoConfiguration.videoDirectory = config.get(
        "video_recording_directory", "/tmp/xwalk-videos");
    videoConfiguration.widthPixels = parseUnsigned(config.get(
        "computer_vision_width", "640"), "computer_vision_width", 7'680U);
    videoConfiguration.heightPixels = parseUnsigned(config.get(
        "computer_vision_height", "480"), "computer_vision_height", 4'320U);
    videoConfiguration.framesPerSecond = static_cast<agent::float64>(
        parseUnsigned(config.get("video_recording_fps", "20"),
            "video_recording_fps", 120U));
    XWalkVideoRecordingOpenCv videoBackend(videoConfiguration);
    XWalkVideoRecordingCallbacks videoCallbacks = videoBackend.callbacks();
    videoCallbacks.delay = &delayMilliseconds;
    videoCallbacks.continueOperation = &continueComputerVision;
    XWalkVideoRecording videoRecording(&videoBackend, videoCallbacks);
    XWalkBootServices services{};
    services.videoRecording = &videoRecording;
    return callback(context, services);
}

} /* namespace xwalk::agent */
