/******************************************************************************
 * @file        xHal_Rpi5CarGpioHardwareTest.cpp
 * @brief       Provides the physical Robot HAT GPIO smoke test.
 *
 * @details
 * Constructs a Linux backend and drives the Robot HAT LED line inactive. This executable is opt-in.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Hardware Test
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

#include "xHal_Rpi5CarGpioLinux.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Drives the physical Robot HAT LED GPIO line low through a selected chip.
 *
 * @param[in] argumentCount Required process-argument count.
 * @param[in] arguments Device path, optional exact chip name, and optional exact label.
 * @return
 * Zero after completion or two for invalid arguments; an exception reports hardware failure.
 *
 * @warning
 * Running this function accesses `/dev/gpiochip0` and changes a physical output.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    if (argumentCount != 4)
    {
        return 2;
    }
    xwalk::hal::XWalkGpioLinux backend(arguments[1U], arguments[2U], arguments[3U], 28U);
    const xwalk::hal::XWalkGpioCallbacks callbacks = XHAL_GPIO_CALLBACKS(xwalk::hal::XWalkGpioLinux);
    xwalk::hal::XWalkGpio gpio(&backend, callbacks, "LED");
    static_cast<void>(gpio.off());
    return 0;
}
