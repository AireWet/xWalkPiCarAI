/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiTreasureHunt.cpp
 * @brief       Composes the Raspberry Pi treasure-hunt mode.
 *
 * @details
 * Binds configured OpenCV color detection and Pico2Wave speech to PiCar-X and
 * supplies source-compatible random treasure selection.
 *
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

#include "xAgent_Rpi5CarComputerVisionOpenCv.h"
#include "xAgent_Rpi5CarTreasureHunt.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarTextToSpeechPico2Wave.h"

#include <random>

namespace xwalk::agent
{

/**
 * @brief Selects one random source-compatible treasure color.
 * @param[in] context Optional context; unused.
 * @return One of red, orange, yellow, green, blue, or purple.
 */
XWalkComputerVisionColor XWalkBootRpi::selectTreasureColor(
    agent::contextpointer context)
{
    static_cast<void>(context);
    std::random_device entropy;
    const agent::uint32 randomValue = static_cast<agent::uint32>(entropy());
    switch (randomValue % 6U)
    {
        case 0U:
            return XWalkComputerVisionColor::Red;
        case 1U:
            return XWalkComputerVisionColor::Orange;
        case 2U:
            return XWalkComputerVisionColor::Yellow;
        case 3U:
            return XWalkComputerVisionColor::Green;
        case 4U:
            return XWalkComputerVisionColor::Blue;
        case 5U:
        default:
            return XWalkComputerVisionColor::Purple;
    }
}

/**
 * @brief Runs configured treasure hunt.
 * @param[in,out] context Nullable caller-owned application context.
 * @param[in] callback Non-null synchronous application callback.
 * @param[in,out] config Loaded deployment configuration.
 * @param[in,out] boardControl Caller-owned board controller.
 * @param[in,out] picarx Caller-owned PiCar-X coordinator.
 * @return Status returned by `callback`.
 */
agent::int32 XWalkBootRpi::runTreasureHunt(agent::contextpointer context,
    bootapplicationcallback callback, hal::XWalkConfigStore& config,
    hal::XWalkBoardControl& boardControl, XWalkPicarx& picarx)
{
    XWalkComputerVisionOpenCvConfiguration visionConfiguration;
    visionConfiguration.cameraBackend = XWalkComputerVisionOpenCv::backendFromString(
        config.get("computer_vision_camera_backend", "v4l2"));
    visionConfiguration.cameraDevice = config.get(
        "computer_vision_camera_device", "");
    visionConfiguration.photoDirectory = config.get(
        "computer_vision_photo_directory", "/tmp/xwalk-pictures");
    visionConfiguration.faceCascadePath = config.get("computer_vision_face_cascade",
        "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml");
    visionConfiguration.widthPixels = parseUnsigned(config.get(
        "computer_vision_width", "640"), "computer_vision_width", 7'680U);
    visionConfiguration.heightPixels = parseUnsigned(config.get(
        "computer_vision_height", "480"), "computer_vision_height", 4'320U);
    visionConfiguration.readTimeoutMilliseconds = parseUnsigned(config.get(
        "computer_vision_read_timeout_ms", "1000"),
        "computer_vision_read_timeout_ms", 60'000U);
    XWalkComputerVisionOpenCv visionBackend(visionConfiguration);
    XWalkComputerVisionCallbacks visionCallbacks = visionBackend.callbacks();
    visionCallbacks.delay = &delayMilliseconds;
    visionCallbacks.continueOperation = &continueComputerVision;
    hal::XWalkTextToSpeechPico2Wave pico2Wave(
        config.get("treasure_hunt_pico2wave_executable", "pico2wave"),
        config.get("treasure_hunt_playback_executable", "aplay"),
        config.get("treasure_hunt_language", "en-US"));
    hal::XWalkTextToSpeech textToSpeech(
        boardControl, &pico2Wave, pico2Wave.callback());
    const XWalkTreasureHuntCallbacks treasureCallbacks{
        visionCallbacks, &selectTreasureColor};
    XWalkTreasureHunt treasureHunt(
        picarx, textToSpeech, &visionBackend, treasureCallbacks);
    XWalkBootServices services{};
    services.picarx = &picarx;
    services.treasureHunt = &treasureHunt;
    return callback(context, services);
}

} /* namespace xwalk::agent */
