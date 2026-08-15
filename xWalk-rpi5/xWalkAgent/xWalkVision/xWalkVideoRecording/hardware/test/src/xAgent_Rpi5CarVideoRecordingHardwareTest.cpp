/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingHardwareTest.cpp
 * @brief       Provides an opt-in physical-camera AVI recording check.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording Hardware Test
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoRecordingOpenCv.h"

#include "xHal_Rpi5CarCommonFunctions.h"

namespace
{

    void delay(agent::contextpointer context, agent::uint32 durationMs)
    {
        static_cast<void>(context);
        xwalk::hal::common::sleepMilliseconds(durationMs);
    }

    agent::boolean continueOperation(agent::contextpointer context)
    {
        static_cast<void>(context);
        return true;
    }

} /* namespace */

int main(int argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        return 1;
    }
    xwalk::agent::XWalkVideoRecordingOpenCvConfiguration configuration;
    configuration.cameraDevice = argumentValues[1U];
    configuration.videoDirectory = argumentValues[2U];
    xwalk::agent::XWalkVideoRecordingOpenCv backend(configuration);
    xwalk::agent::XWalkVideoRecordingCallbacks callbacks = backend.callbacks();
    callbacks.delay = &delay;
    callbacks.continueOperation = &continueOperation;
    xwalk::agent::XWalkVideoRecording recording(&backend, callbacks);
    const agent::boolean recordingStarted = recording.start();
    if (recordingStarted == false)
    {
        return 2;
    }
    static_cast<void>(recording.handleKey("q"));
    delay(nullptr, 1'000U);
    static_cast<void>(recording.handleKey("e"));
    recording.stop();
    return 0;
}
