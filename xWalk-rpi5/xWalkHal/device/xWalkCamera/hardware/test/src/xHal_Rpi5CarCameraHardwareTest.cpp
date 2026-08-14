/******************************************************************************
 * @file        xHal_Rpi5CarCameraHardwareTest.cpp
 * @brief       Captures one bounded JPEG through a selected physical camera.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera Hardware Test
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarCameraLinux.h"

int main(int argumentCount, char* arguments[])
{
    if (argumentCount != 5)
    {
        return 2;
    }
    const xwalk::hal::XWalkCameraConnection connection =
        xwalk::hal::XWalkCamera::connectionFromString(arguments[1]);
    xwalk::hal::XWalkCameraLinux backend(connection, arguments[2], arguments[3]);
    xwalk::hal::XWalkCamera camera(&backend, backend.callback());
    static_cast<void>(camera.capture(arguments[4]));
    return 0;
}
