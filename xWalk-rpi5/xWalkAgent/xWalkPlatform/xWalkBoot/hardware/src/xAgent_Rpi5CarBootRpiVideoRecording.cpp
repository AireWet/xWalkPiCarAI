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
#include "xHal_Rpi5CarTrace.h"

#include "xAgent_Rpi5CarVideoRecordingOpenCv.h"
#include "xHal_Rpi5CarConfigStore.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured standalone video recording.
     * @param[in] parameters Non-owning application callback and configuration
     * dependency valid through this synchronous composition.
     * @return Status returned by the configured callback.
     * @pre `parameters.callback` and `parameters.config` are non-null.
     */
    agent::int32 XWalkBootRpi::runVideoRecording(const xAgentContext& parameters)
    {
        hal::XWalkConfigStore& config = *parameters.config;
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .066, "Boot composing video-recording services");
        XWalkVideoRecordingOpenCvConfiguration videoConfiguration;
        videoConfiguration.cameraBackend =
            XWalkVideoRecordingOpenCv::backendFromString(config.get("video_recording_camera_backend", "v4l2"));
        videoConfiguration.cameraDevice = config.get("video_recording_camera_device", "");
        videoConfiguration.videoDirectory = config.get("video_recording_directory", "/tmp/xwalk-videos");
        videoConfiguration.widthPixels =
            parseUnsigned(config.get("computer_vision_width", "640"), "computer_vision_width", 7'680U);
        videoConfiguration.heightPixels =
            parseUnsigned(config.get("computer_vision_height", "480"), "computer_vision_height", 4'320U);
        videoConfiguration.framesPerSecond = static_cast<agent::float64>(
            parseUnsigned(config.get("video_recording_fps", "20"), "video_recording_fps", 120U));
        videoConfiguration.readTimeoutMilliseconds = parseUnsigned(
            config.get("video_recording_read_timeout_ms", "1000"), "video_recording_read_timeout_ms", 60'000U);
        XWalkVideoRecordingOpenCv videoBackend(videoConfiguration);
        XWalkVideoRecordingCallbacks videoCallbacks = videoBackend.callbacks();
        videoCallbacks.delay = &delayMilliseconds;
        videoCallbacks.continueOperation = &continueComputerVision;
        XWalkVideoRecording videoRecording(&videoBackend, videoCallbacks);
        XWalkBootServices services{};
        services.videoRecording = &videoRecording;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
