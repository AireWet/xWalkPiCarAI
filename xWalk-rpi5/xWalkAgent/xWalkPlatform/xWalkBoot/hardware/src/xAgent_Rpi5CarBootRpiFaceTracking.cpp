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
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runFaceTracking(agent::contextpointer context,
                                               bootapplicationcallback callback,
                                               hal::XWalkConfigStore& config,
                                               XWalkPicarx& picarx)
    {
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
        return callback(context, services);
    }

} /* namespace xwalk::agent */
