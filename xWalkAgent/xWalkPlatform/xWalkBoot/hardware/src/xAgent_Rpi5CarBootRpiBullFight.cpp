/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiBullFight.cpp
 * @brief       Composes Raspberry Pi red-target pursuit.
 * @details     Binds configured OpenCV red detection to the PiCar-X coordinator.
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

#include "xAgent_Rpi5CarBullFight.h"
#include "xAgent_Rpi5CarComputerVisionOpenCv.h"
#include "xHal_Rpi5CarConfigStore.h"

namespace xwalk::agent
{

/**
 * @brief Runs configured red-target pursuit.
 * @param[in,out] context Nullable caller-owned application context.
 * @param[in] callback Non-null synchronous application callback.
 * @param[in,out] config Loaded deployment configuration.
 * @param[in,out] picarx Caller-owned PiCar-X coordinator.
 * @return Status returned by `callback`.
 */
agent::int32 XWalkBootRpi::runBullFight(agent::contextpointer context,
    bootapplicationcallback callback, hal::XWalkConfigStore& config,
    XWalkPicarx& picarx)
{
    XWalkComputerVisionOpenCvConfiguration visionConfiguration;
    visionConfiguration.cameraDevice = config.get(
        "computer_vision_camera_device", "/dev/video0");
    visionConfiguration.photoDirectory = config.get(
        "computer_vision_photo_directory", "/tmp/xwalk-pictures");
    visionConfiguration.faceCascadePath = config.get("computer_vision_face_cascade",
        "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml");
    visionConfiguration.widthPixels = parseUnsigned(config.get(
        "computer_vision_width", "640"), "computer_vision_width", 7'680U);
    visionConfiguration.heightPixels = parseUnsigned(config.get(
        "computer_vision_height", "480"), "computer_vision_height", 4'320U);
    XWalkComputerVisionOpenCv visionBackend(visionConfiguration);
    XWalkComputerVisionCallbacks visionCallbacks = visionBackend.callbacks();
    visionCallbacks.delay = &delayMilliseconds;
    visionCallbacks.continueOperation = &continueComputerVision;
    XWalkBullFightConfiguration bullFightConfiguration;
    bullFightConfiguration.frameWidthPixels = visionConfiguration.widthPixels;
    bullFightConfiguration.frameHeightPixels = visionConfiguration.heightPixels;
    XWalkBullFight bullFight(picarx, &visionBackend,
        visionCallbacks, bullFightConfiguration);
    XWalkBootServices services{};
    services.picarx = &picarx;
    services.bullFight = &bullFight;
    return callback(context, services);
}

} /* namespace xwalk::agent */
