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
#include "xAgent_Rpi5CarCameraCaptureTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"

int main()
{
    xwalk::agent::test::camera::CameraCaptureTestState state;
    xwalk::hal::XWalkCamera camera(&state, &xwalk::agent::test::camera::capture);
    xwalk::agent::XWalkCameraCapture cameraCapture(camera, "voice-image.jpg");
    const agent::string requiredPath = cameraCapture.capture();
    xwalk::hal::test::requireTestCondition(requiredPath == "voice-image.jpg");
    const agent::string optionalPath = xwalk::agent::XWalkCameraCapture::captureImage(&cameraCapture);
    xwalk::hal::test::requireTestCondition(optionalPath == "voice-image.jpg");
    state.result = false;
    const agent::string unavailablePath = xwalk::agent::XWalkCameraCapture::captureImage(&cameraCapture);
    xwalk::hal::test::requireTestCondition(unavailablePath.empty());
    xwalk::hal::test::requireTestCondition(state.captureCount == 3U);
    return 0;
}
