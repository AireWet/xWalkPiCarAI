/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonHardwareTest.cpp
 * @brief       Provides an opt-in Robot HAT user-button smoke test.
 *
 * @details
 * Composes the Linux GPIO backend, caller-owned pull-up input, and user-button
 * monitor, then performs one bounded monitoring interval.
 *
 * @project     xWalk Firmware
 * @module      xWalkUserButton Hardware Test
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
#include "xHal_Rpi5CarUserButton.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Monitors the Robot HAT user-button input for one polling interval.
 *
 * @return
 * Zero after the monitoring worker stops successfully.
 *
 * @warning
 * Running this function accesses `/dev/gpiochip0` and claims the USER input.
 */
XWalkHal::int32 main()
{
    XWalkHal::XWalkGpioLinux backend;
    const XWalkHal::XWalkGpioCallbacks callbacks =
        XHAL_GPIO_CALLBACKS(XWalkHal::XWalkGpioLinux);
    XWalkHal::XWalkGpio gpio(&backend, callbacks, "USER", XWalkHal::XWalkGpioMode::Input,
        XWalkHal::XWalkGpioPull::Up);
    XWalkHal::XWalkUserButton button(gpio);
    button.start();
    XWalkHal::common::sleepMilliseconds(XHAL_RPI5CAR_USER_BUTTON_POLL_INTERVAL_MS);
    button.stop();
    return 0;
}
