/******************************************************************************
 * @file        xHal_Rpi5CarServoTestInitialization.cpp
 * @brief       Tests Servo timer initialization.
 *
 * @details
 * Verifies the period and prescaler values written when a Servo is composed
 * with a caller-created PWM channel.
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
     * @brief Verifies Servo construction configures the Python-compatible timer.
     *
     * @post
     * The assertions confirm a 4095-count period and rounded prescaler value 352.
     */
    void testServoInitialization()
    {
        XWalkServoTestI2c bus;
        XWalkI2c i2c(&bus,
                     &XWalkServoTestI2c::probeCallback,
                     &XWalkServoTestI2c::writeRegisterCallback,
                     &XWalkServoTestI2c::readCallback);
        XWalkPwmTimerState timerState;
        XWalkPwm pwm(i2c, 0U, 0x14U, timerState);
        bus.clearWrites();
        XWalkServo servo(pwm);

        assert(bus.writeCount() == 0U);
        assert(servo.isInitialized() == false);
        assert(servo.initialize());
        assert(servo.isInitialized());
        assert(servo.initialize() == false);

        const bytevector expectedPeriodBytes{0x0FU, 0xFFU};
        const bytevector expectedPrescalerBytes{0x01U, 0x5FU};
        assert(bus.writeCount() == 2U);
        assert(bus.writeRegister(0U) == 0x44U);
        assert(bus.writeData(0U) == expectedPeriodBytes);
        assert(bus.writeRegister(1U) == 0x40U);
        assert(bus.writeData(1U) == expectedPrescalerBytes);
        assert(pwm.period() == 4095U);
        assert(pwm.prescaler() == 352U);
    }

} /* namespace xwalk::hal::test */
