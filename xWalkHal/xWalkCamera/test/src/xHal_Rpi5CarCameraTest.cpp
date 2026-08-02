/******************************************************************************
 * @file        xHal_Rpi5CarCameraTest.cpp
 * @brief       Verifies backend-neutral camera behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera Host Test
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarCamera.h"

#include <cassert>

namespace
{

struct CameraTestState
{
    XWalkHal::uint32 captureCount{};
    XWalkHal::string outputPath{};
};

XWalkHal::boolean captureImage(XWalkHal::contextpointer context,
    XWalkHal::stringview outputPath,
    const xwalk::hal::XWalkCameraConfiguration& configuration)
{
    CameraTestState& state = *static_cast<CameraTestState*>(context);
    ++state.captureCount;
    state.outputPath = outputPath;
    return (configuration.widthPixels == 640U) &&
        (configuration.heightPixels == 480U) &&
        (configuration.timeoutMs == 5'000U);
}

} /* namespace */

int main()
{
    CameraTestState state{};
    xwalk::hal::XWalkCamera camera(&state, &captureImage);
    assert(camera.capture("image.jpg") == "image.jpg");
    assert(state.captureCount == 1U);
    assert(state.outputPath == "image.jpg");
    assert(xwalk::hal::XWalkCamera::connectionFromString("csi") ==
        xwalk::hal::XWalkCameraConnection::Csi);
    assert(xwalk::hal::XWalkCamera::connectionFromString("usb") ==
        xwalk::hal::XWalkCameraConnection::Usb);
    return 0;
}
