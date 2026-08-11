/******************************************************************************
 * @file        xHal_Rpi5CarDeviceHardwareTest.cpp
 * @brief       Provides an opt-in Robot HAT device-tree discovery smoke test.
 *
 * @details
 * Constructs the detector against the Linux firmware device-tree root without
 * changing GPIO, audio, motor, ADC, or other physical hardware state.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Hardware Test
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarDevice.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Discovers Robot HAT information from `/proc/device-tree`.
 *
 * @return
 * Zero when discovery completes, including the supported no-HAT default result.
 *
 * @warning
 * Running this function reads Linux firmware device-tree files but performs no
 * hardware output operation.
 */
XWalkHal::int32 main()
{
    const XWalkHal::XWalkDevice device;
    static_cast<void>(device.information());
    return 0;
}
