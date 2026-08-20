/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiFaceTracking.cpp
 * @brief       Composes Raspberry Pi face tracking.
 * @details     Binds configured OpenCV face detection to the PiCar-X camera servos.
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
#include "xAgent_Rpi5CarFaceTracking.h"
#include "xHal_Rpi5CarConfigStore.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured face tracking.
     * @param[in] parameters Non-owning application callback, configuration,
     * and PiCar-X dependencies valid through this synchronous composition.
     * @return Status returned by the configured callback.
     * @pre `parameters.callback`, `parameters.config`, and
     * `parameters.picarx` are non-null.
     */
    agent::int32 XWalkBootRpi::runFaceTracking(const xAgentContext& parameters)
    {
        hal::XWalkConfigStore& config = *parameters.config;
        XWalkPicarx& picarx = *parameters.picarx;
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .053, "Boot composing face-tracking services");
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
        XWalkFaceTrackingConfiguration trackingConfiguration;
        trackingConfiguration.frameWidthPixels = visionConfiguration.widthPixels;
        trackingConfiguration.frameHeightPixels = visionConfiguration.heightPixels;
        XWalkFaceTracking faceTracking(picarx, &visionBackend, visionCallbacks, trackingConfiguration);
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.faceTracking = &faceTracking;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
