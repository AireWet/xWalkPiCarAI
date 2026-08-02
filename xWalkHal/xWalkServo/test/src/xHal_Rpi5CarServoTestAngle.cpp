/******************************************************************************
 * @file        xHal_Rpi5CarServoTestAngle.cpp
 * @brief       Tests Servo angle conversion and clamping.
 *
 * @details
 * Confirms that low, centered, and high angle commands produce the same
 * truncated PWM counts as the Python Robot HAT implementation.
 *
 * @project     xWalk Firmware
 * @module      xWalkServo Host Test
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

#include "xHal_Rpi5CarServoTestFunctions.h"
#include "xHal_Rpi5CarServoTestI2c.h"

#include "xHal_Rpi5CarServo.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Test function definitions
 ******************************************************************************/

/**
 * @brief Verifies angle clamping, mapping, truncation, and output byte order.
 *
 * @post
 * The assertions cover below-minimum, centered, and above-maximum commands.
 */
void testServoAngles()
{
    XWalkServoTestI2c bus;
    XWalkI2c i2c(&bus, &XWalkServoTestI2c::probeCallback, &XWalkServoTestI2c::writeRegisterCallback,
        &XWalkServoTestI2c::readCallback);
    XWalkPwmTimerState timerState;
    XWalkPwm pwm(i2c, 0U, 0x14U, timerState);
    XWalkServo servo(pwm);

    bus.clearWrites();
    servo.setAngle(-120.0);
    const bytevector expectedMinimumBytes{0x00U, 0x66U};
    assert(bus.writeCount() == 1U);
    assert(bus.writeRegister(0U) == 0x20U);
    assert(bus.writeData(0U) == expectedMinimumBytes);
    assert(pwm.pulseWidth() == 102U);

    bus.clearWrites();
    servo.setAngle(0.0);
    const bytevector expectedCenterBytes{0x01U, 0x33U};
    assert(bus.writeCount() == 1U);
    assert(bus.writeRegister(0U) == 0x20U);
    assert(bus.writeData(0U) == expectedCenterBytes);
    assert(pwm.pulseWidth() == 307U);

    bus.clearWrites();
    servo.setAngle(120.0);
    const bytevector expectedMaximumBytes{0x01U, 0xFFU};
    assert(bus.writeCount() == 1U);
    assert(bus.writeRegister(0U) == 0x20U);
    assert(bus.writeData(0U) == expectedMaximumBytes);
    assert(pwm.pulseWidth() == 511U);
}

} /* namespace xwalk::hal::test */
