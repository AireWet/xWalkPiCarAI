/******************************************************************************
 * @file        xHal_Rpi5CarServoTestValidation.cpp
 * @brief       Tests Servo rejection of non-finite commands.
 *
 * @details
 * Confirms that conversion to integer PWM counts never receives infinity.
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

#include "xHal_Rpi5CarTestFunctions.h"
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
 * @brief Verifies non-finite angle and pulse-duration validation.
 *
 * @post
 * The assertions confirm both commands throw `std::invalid_argument`.
 */
void testServoValidation()
{
    XWalkServoTestI2c bus;
    XWalkI2c i2c(&bus, &XWalkServoTestI2c::probeCallback, &XWalkServoTestI2c::writeRegisterCallback,
        &XWalkServoTestI2c::readCallback);
    XWalkPwmTimerState timerState;
    XWalkPwm pwm(i2c, 0U, 0x14U, timerState);
    XWalkServo servo(pwm);

    expectFailure([&servo]()
    {
        servo.setAngle(0.0);
    });
    static_cast<void>(servo.initialize());

    expectFailure([&servo]()
    {
        servo.setAngle(XHAL_POSITIVE_INFINITY(float64));
    });
    expectFailure([&servo]()
    {
        servo.setPulseWidthTime(XHAL_POSITIVE_INFINITY(float64));
    });
}

} /* namespace xwalk::hal::test */
