/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVideoCar.cpp
 * @brief       Composes Raspberry Pi camera-assisted driving.
 * @details     Binds the configured OpenCV provider to the PiCar-X video-car Agent.
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

#include "xAgent_Rpi5CarComputerVisionOpenCv.h"
#include "xAgent_Rpi5CarVideoCar.h"
#include "xHal_Rpi5CarConfigStore.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured camera-assisted driving.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runVideoCar(agent::contextpointer context,
                                           bootapplicationcallback callback,
                                           hal::XWalkConfigStore& config,
                                           XWalkPicarx& picarx)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .065, "Boot composing video-car services");
        XWalkComputerVisionOpenCvConfiguration visionConfiguration;
        visionConfiguration.cameraBackend =
            XWalkComputerVisionOpenCv::backendFromString(config.get("computer_vision_camera_backend", "v4l2"));
        visionConfiguration.cameraDevice = config.get("computer_vision_camera_device", "");
        visionConfiguration.photoDirectory = config.get("computer_vision_photo_directory", "/tmp/xwalk-pictures");
        visionConfiguration.faceCascadePath = config.get(
            "computer_vision_face_cascade", "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml");
        visionConfiguration.widthPixels =
            parseUnsigned(config.get("computer_vision_width", "640"), "computer_vision_width", 7'680U);
        visionConfiguration.heightPixels =
            parseUnsigned(config.get("computer_vision_height", "480"), "computer_vision_height", 4'320U);
        visionConfiguration.readTimeoutMilliseconds = parseUnsigned(
            config.get("computer_vision_read_timeout_ms", "1000"), "computer_vision_read_timeout_ms", 60'000U);
        XWalkComputerVisionOpenCv visionBackend(visionConfiguration);
        XWalkComputerVisionCallbacks visionCallbacks = visionBackend.callbacks();
        visionCallbacks.delay = &delayMilliseconds;
        visionCallbacks.continueOperation = &continueComputerVision;
        XWalkVideoCar videoCar(picarx, &visionBackend, visionCallbacks);
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.videoCar = &videoCar;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
