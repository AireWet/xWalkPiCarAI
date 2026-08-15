/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonSimulation.cpp
 * @brief       Implements the device-free xWalkUserButton simulation.
 * @project     xWalk Firmware
 * @module      xWalkUserButton Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarUserButtonSimulation.h"
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarUserButtonHostStub.h"
namespace xwalk::hal::sim
{
    int32 runUserButtonSimulation()
    {
        XWalkUserButtonHostStub backend;
        const XWalkGpioCallbacks callbackSet = XWalkUserButtonHostStub::callbacks();
        XWalkGpio gpio(&backend, callbackSet, "USER", XWalkGpioMode::Input, XWalkGpioPull::Up);
        XWalkUserButton button(gpio);
        button.setOnClick(&backend, &XWalkUserButtonHostStub::countClick);
        button.start();
        common::sleepMilliseconds(100U);
        backend.setPressed(true);
        common::sleepMilliseconds(150U);
        const boolean pressObserved = button.isPressed();
        backend.setPressed(false);
        common::sleepMilliseconds(150U);
        button.stop();
        const boolean valid = pressObserved && (button.isPressed() == false) && (backend.clickCount() == 1U);
        XWALK_HAL_TRACE_UID0(RPI .227, "xWalkUserButton host simulation completed");
        return valid ? 0 : 1;
    }
} /* namespace xwalk::hal::sim */
