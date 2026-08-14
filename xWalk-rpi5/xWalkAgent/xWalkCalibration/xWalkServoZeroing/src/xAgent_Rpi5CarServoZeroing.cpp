/******************************************************************************
 * @file        xAgent_Rpi5CarServoZeroing.cpp
 * @brief       Implements the twelve-channel servo-zeroing sequence.
 *
 * @project     xWalk Firmware
 * @module      xWalkServoZeroing
 *
 * @author      Joxy John
 * @date        2026-08-05
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

#include "xAgent_Rpi5CarServoZeroing.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Pulses channels zero through eleven, zeros them, and idles.
 * @return `true` after every channel reaches zero; otherwise `false` for early cancellation.
 * @warning Commands physical servo movement when bound to the Raspberry Pi provider.
 */
agent::boolean XWalkServoZeroing::run()
{
    for (agent::uint8 servoId = 0U;
        servoId < XAGENT_RPI5CAR_SERVO_ZEROING_CHANNEL_COUNT; ++servoId)
    {
        callbacks.setAngle(
            callbackContext, servoId, configurationValue.pulseAngleDegrees);
        const agent::boolean pulseDelayCompleted =
            wait(configurationValue.commandDelayMs);
        if (pulseDelayCompleted == false)
        {
            return false;
        }
        callbacks.setAngle(
            callbackContext, servoId, configurationValue.zeroAngleDegrees);
        const agent::boolean zeroDelayCompleted =
            wait(configurationValue.commandDelayMs);
        if (zeroDelayCompleted == false)
        {
            return false;
        }
    }
    const agent::boolean processingLoopRequested{true};
    while (processingLoopRequested)
    {
        const agent::boolean waitSucceeded =
            static_cast<agent::boolean>(
                wait(configurationValue.idleDelayMs));
        if (waitSucceeded == false)
        {
            break;
        }
    }
    return true;
}

} /* namespace xwalk::agent */
