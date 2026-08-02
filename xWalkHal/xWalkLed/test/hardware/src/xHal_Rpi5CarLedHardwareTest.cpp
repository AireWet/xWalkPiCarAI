/******************************************************************************
 * @file        xHal_Rpi5CarLedHardwareTest.cpp
 * @brief       Provides an opt-in Robot HAT LED hardware smoke test.
 *
 * @details
 * Composes the Linux GPIO backend, caller-owned LED GPIO, and LED controller
 * before explicitly requesting the inactive physical output state.
 *
 * @project     xWalk Firmware
 * @module      xWalkLed Hardware Test
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
#include "xHal_Rpi5CarLed.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Places the Robot HAT LED output into its inactive state.
 *
 * @return
 * Zero after the GPIO operation completes.
 *
 * @warning
 * Running this function accesses `/dev/gpiochip0` and changes a physical output.
 */
XWalkHal::int32 main()
{
    XWalkHal::XWalkGpioLinux backend;
    const XWalkHal::XWalkGpioCallbacks callbacks =
        XHAL_GPIO_CALLBACKS(XWalkHal::XWalkGpioLinux);
    XWalkHal::XWalkGpio gpio(&backend, callbacks, "LED");
    XWalkHal::XWalkLed led(gpio);
    led.close();
    return 0;
}
