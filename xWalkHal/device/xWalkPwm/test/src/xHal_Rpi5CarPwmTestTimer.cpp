/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestTimer.cpp
 * @brief       Tests the default PWM timer frequency configuration.
 *
 * @details
 * Verifies that PWM construction selects non-zero timer values, produces a
 * frequency close to 50 Hertz, and writes the expected timer registers.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Test
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

#include "xHal_Rpi5CarPwmTestFunctions.h"
#include "xHal_Rpi5CarPwmTestI2c.h"

#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarMath.h"

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
 * @brief Verifies the calculated and written default 50 Hertz settings.
 *
 * @post
 * The assertions confirm non-zero timer values, frequency accuracy, and
 * prescaler/period register selection.
 */
void testDefaultFrequency()
{
    XWalkPwmTestI2c bus;
    XWalkI2c i2c(&bus, &XWalkPwmTestI2c::probeCallback, &XWalkPwmTestI2c::writeRegisterCallback,
        &XWalkPwmTestI2c::readCallback);
    XWalkPwmTimerState timerState;
    XWalkPwm pwm(i2c, 0U, 0x14U, timerState);

    assert(pwm.prescaler() > 0U);
    assert(pwm.period() > 0U);
    assert(XHAL_ABSOLUTE_VALUE(pwm.frequency() - XHAL_RPI5CAR_PWM_DEFAULT_FREQUENCY_HZ) < 0.01);
    assert(bus.writeRegister(0U) == 0x40U);
    assert(bus.writeRegister(1U) == 0x44U);
}

} /* namespace xwalk::hal::test */
