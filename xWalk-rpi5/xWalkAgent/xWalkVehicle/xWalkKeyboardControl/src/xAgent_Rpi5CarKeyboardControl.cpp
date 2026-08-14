/******************************************************************************
 * @file        xAgent_Rpi5CarKeyboardControl.cpp
 * @brief       Implements keyboard-to-actuator behavior.
 *
 * @details
 * Preserves the key mapping, camera increments, movement speed, pulse timing,
 * and final centered state from upstream `example/3.keyboard_control.py`.
 *
 * @project     xWalk Firmware
 * @module      xWalkKeyboardControl
 *
 * @author      Joxy John
 * @date        2026-08-04
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

#include "xAgent_Rpi5CarKeyboardControl.h"

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
 * @brief Processes one key and performs the upstream 500-millisecond pulse.
 * @param[in] keyText One-character `w`, `s`, `a`, `d`, `i`, `k`, `j`, or `l` input.
 * @return Handled, ignored, or cancelled processing status.
 * @warning A movement key drives the physical car at 80-percent requested speed.
 */
XWalkKeyboardControlResult XWalkKeyboardControl::handleKey(agent::stringview keyText)
{
    const agent::boolean keyTextDifferent =
        static_cast<agent::boolean>(
            keyText.size() != 1U);
    if (keyTextDifferent)
    {
        return XWalkKeyboardControlResult::Ignored;
    }
    const agent::boolean operationRequested = continueCallback(callbackContext);
    if (operationRequested == false)
    {
        return XWalkKeyboardControlResult::Cancelled;
    }

    const char key = keyText[0U];
    if ((key == 'w') || (key == 'W'))
    {
        picarxObject->setDirectionServoAngle(0.0);
        picarxObject->forward(80.0);
    }
    else if ((key == 's') || (key == 'S'))
    {
        picarxObject->setDirectionServoAngle(0.0);
        picarxObject->backward(80.0);
    }
    else if ((key == 'a') || (key == 'A'))
    {
        picarxObject->setDirectionServoAngle(-30.0);
        picarxObject->forward(80.0);
    }
    else if ((key == 'd') || (key == 'D'))
    {
        picarxObject->setDirectionServoAngle(30.0);
        picarxObject->forward(80.0);
    }
    else if ((key == 'i') || (key == 'I'))
    {
        tiltAngleDegreesValue += 5.0;
        if (tiltAngleDegreesValue > 30.0)
        {
            tiltAngleDegreesValue = 30.0;
        }
    }
    else if ((key == 'k') || (key == 'K'))
    {
        tiltAngleDegreesValue -= 5.0;
        if (tiltAngleDegreesValue < -30.0)
        {
            tiltAngleDegreesValue = -30.0;
        }
    }
    else if ((key == 'l') || (key == 'L'))
    {
        panAngleDegreesValue += 5.0;
        if (panAngleDegreesValue > 30.0)
        {
            panAngleDegreesValue = 30.0;
        }
    }
    else if ((key == 'j') || (key == 'J'))
    {
        panAngleDegreesValue -= 5.0;
        if (panAngleDegreesValue < -30.0)
        {
            panAngleDegreesValue = -30.0;
        }
    }
    else
    {
        return XWalkKeyboardControlResult::Ignored;
    }

    applyCameraAngles();
    const agent::boolean delayCompleted = wait(500U);
    if (delayCompleted == false)
    {
        picarxObject->stop();
        return XWalkKeyboardControlResult::Cancelled;
    }
    picarxObject->forward(0.0);
    return XWalkKeyboardControlResult::Handled;
}

/**
 * @brief Centers steering and camera servos, stops motors, and waits 200 milliseconds.
 * @warning Physically moves all three servos to their logical centers.
 */
void XWalkKeyboardControl::finish()
{
    panAngleDegreesValue = 0.0;
    tiltAngleDegreesValue = 0.0;
    picarxObject->setCameraTiltAngle(0.0);
    picarxObject->setCameraPanAngle(0.0);
    picarxObject->setDirectionServoAngle(0.0);
    picarxObject->stop();
    delayCallback(callbackContext, 200U);
}

/**
 * @brief Returns the retained logical camera-pan angle in degrees.
 * @return Current value from minus 30 through plus 30 degrees.
 */
agent::float64 XWalkKeyboardControl::panAngleDegrees() const noexcept
{
    return panAngleDegreesValue;
}

/**
 * @brief Returns the retained logical camera-tilt angle in degrees.
 * @return Current value from minus 30 through plus 30 degrees.
 */
agent::float64 XWalkKeyboardControl::tiltAngleDegrees() const noexcept
{
    return tiltAngleDegreesValue;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Applies both retained camera angles to the observed vehicle.
 * @post Camera tilt and pan receive the retained logical angle commands.
 */
void XWalkKeyboardControl::applyCameraAngles()
{
    picarxObject->setCameraTiltAngle(tiltAngleDegreesValue);
    picarxObject->setCameraPanAngle(panAngleDegreesValue);
}

} /* namespace xwalk::agent */
