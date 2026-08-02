/******************************************************************************
 * @file        xAgent_Rpi5CarCameraCaptureTest.cpp
 * @brief       Verifies the camera-capture Agent callback.
 *
 * @project     xWalk Firmware
 * @module      xWalkCameraCapture Host Test
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarCameraCapture.h"

#include <cassert>

namespace
{

XWalkHal::boolean capture(XWalkHal::contextpointer context,
    XWalkHal::stringview outputPath,
    const xwalk::hal::XWalkCameraConfiguration& configuration)
{
    static_cast<void>(context);
    static_cast<void>(configuration);
    return outputPath == "voice-image.jpg";
}

} /* namespace */

int main()
{
    xwalk::hal::XWalkCamera camera(nullptr, &capture);
    xwalk::agent::XWalkCameraCapture cameraCapture(camera, "voice-image.jpg");
    assert(cameraCapture.capture() == "voice-image.jpg");
    assert(xwalk::agent::XWalkCameraCapture::captureImage(&cameraCapture) ==
        "voice-image.jpg");
    return 0;
}
