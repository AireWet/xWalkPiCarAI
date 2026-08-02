/******************************************************************************
 * @file        xHal_Rpi5CarServoTestPulse.cpp
 * @brief       Tests direct Servo pulse-duration commands.
 *
 * @details
 * Verifies microsecond clamping and conversion to 4095-period PWM timer counts.
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
 * @brief Verifies direct pulse-duration clamping and count conversion.
 *
 * @post
 * The assertions cover pulse durations below, within, and above the range.
 */
void testServoPulseWidths()
{
    XWalkServoTestI2c bus;
    XWalkI2c i2c(&bus, &XWalkServoTestI2c::probeCallback, &XWalkServoTestI2c::writeRegisterCallback,
        &XWalkServoTestI2c::readCallback);
    XWalkPwmTimerState timerState;
    XWalkPwm pwm(i2c, 0U, 0x14U, timerState);
    XWalkServo servo(pwm);

    bus.clearWrites();
    servo.setPulseWidthTime(0.0);
    const bytevector expectedMinimumBytes{0x00U, 0x66U};
    assert(bus.writeData(0U) == expectedMinimumBytes);
    assert(pwm.pulseWidth() == 102U);

    bus.clearWrites();
    servo.setPulseWidthTime(1500.0);
    const bytevector expectedCenterBytes{0x01U, 0x33U};
    assert(bus.writeData(0U) == expectedCenterBytes);
    assert(pwm.pulseWidth() == 307U);

    bus.clearWrites();
    servo.setPulseWidthTime(3000.0);
    const bytevector expectedMaximumBytes{0x01U, 0xFFU};
    assert(bus.writeData(0U) == expectedMaximumBytes);
    assert(pwm.pulseWidth() == 511U);
}

} /* namespace xwalk::hal::test */
