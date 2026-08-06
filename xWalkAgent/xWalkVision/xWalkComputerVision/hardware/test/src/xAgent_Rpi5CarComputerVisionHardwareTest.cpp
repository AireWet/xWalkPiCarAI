/******************************************************************************
 * @file        xAgent_Rpi5CarComputerVisionHardwareTest.cpp
 * @brief       Provides opt-in physical-camera computer-vision verification.
 *
 * @details
 * Opens the configured camera, exercises color, face, QR, observation, and one
 * temporary photograph operation, then removes the test-owned photograph.
 *
 * @project     xWalk Firmware
 * @module      xWalkComputerVision Hardware Test
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarComputerVisionOpenCv.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarFileFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains scheduling callbacks for the opt-in physical test. */
namespace
{

/** @brief Applies the Agent's bounded post-key delay. */
void delay(agent::contextpointer context, agent::uint32 durationMs)
{
    static_cast<void>(context);
    xwalk::hal::common::sleepMilliseconds(durationMs);
}

/** @brief Allows the bounded physical scenario to finish. */
agent::boolean continueOperation(agent::contextpointer context)
{
    static_cast<void>(context);
    return true;
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs one explicitly selected physical-camera scenario.
 * @param[in] argumentCount Must equal four.
 * @param[in] arguments Device, cascade, and temporary output directory.
 * @return Zero after successful bounded capture and detector operations.
 * @warning Opens a camera and briefly records one test-owned JPEG image.
 */
int main(int argumentCount, char* arguments[])
{
    if (argumentCount != 4)
    {
        return 1;
    }
    xwalk::agent::XWalkComputerVisionOpenCvConfiguration configuration;
    configuration.cameraDevice = arguments[1U];
    configuration.faceCascadePath = arguments[2U];
    configuration.photoDirectory = arguments[3U];
    xwalk::agent::XWalkComputerVisionOpenCv provider(configuration);
    xwalk::agent::XWalkComputerVisionCallbacks callbacks = provider.callbacks();
    callbacks.delay = &delay;
    callbacks.continueOperation = &continueOperation;
    xwalk::agent::XWalkComputerVision vision(&provider, callbacks);
    const agent::boolean started = vision.start();
    assert(started);
    static_cast<void>(vision.handleKey("1"));
    static_cast<void>(vision.handleKey("f"));
    static_cast<void>(vision.handleKey("r"));
    const xwalk::agent::XWalkComputerVisionResult observation = vision.handleKey("s");
    assert(observation.event == xwalk::agent::XWalkComputerVisionEvent::ObjectsShown);
    const xwalk::agent::XWalkComputerVisionResult photograph = vision.handleKey("q");
    assert(photograph.event == xwalk::agent::XWalkComputerVisionEvent::PhotoCaptured);
    assert(xwalk::hal::filesystemEntryExists(photograph.photoPath));
    vision.stop();
    const agent::boolean removed =
        xwalk::hal::removeFilesystemEntry(photograph.photoPath);
    assert(removed);
    return 0;
}
