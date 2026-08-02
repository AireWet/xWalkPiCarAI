/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestValidation.cpp
 * @brief       Tests PWM input-validation failure behavior.
 *
 * @details
 * Verifies that invalid numeric and textual channels, frequencies, pulse
 * widths, and duty-cycle percentages produce exceptions.
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

#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarPwmTestI2c.h"

#include "xHal_Rpi5CarPwm.h"

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
 * @brief Verifies exception behavior for invalid PWM inputs.
 *
 * @post
 * The assertions confirm rejection of invalid channel, frequency, pulse,
 * and percentage inputs.
 */
void testValidation()
{
    XWalkPwmTestI2c bus;
    XWalkI2c i2c(&bus, &XWalkPwmTestI2c::probeCallback, &XWalkPwmTestI2c::writeRegisterCallback,
        &XWalkPwmTestI2c::readCallback);
    XWalkPwmTimerState timerState;
    expectFailure([&i2c, &timerState]()
    {
        XWalkPwm invalid(i2c, 20U, 0x14U, timerState);
    });
    expectFailure([&i2c, &timerState]()
    {
        XWalkPwm invalid(i2c, 0U, 0x80U, timerState);
    });
    expectFailure([&i2c, &timerState]()
    {
        XWalkPwm invalid(i2c, "PWM0", 0x14U, timerState);
    });

    XWalkPwm pwm(i2c, 0U, 0x14U, timerState);
    expectFailure([&pwm]()
    {
        pwm.setFrequency(0.0);
    });
    expectFailure([&pwm]()
    {
        pwm.setPulseWidth(-1.0);
    });
    expectFailure([&pwm]()
    {
        pwm.setPulseWidthPercent(101.0);
    });
    expectFailure([&pwm]()
    {
        pwm.setFrequency(XHAL_POSITIVE_INFINITY(float64));
    });
    expectFailure([&pwm]()
    {
        const float64 maximumFrequencyHz = static_cast<float64>(XHAL_RPI5CAR_UINT32_MAX);
        const float64 excessiveFrequencyHz = maximumFrequencyHz + 1.0;
        pwm.setFrequency(excessiveFrequencyHz);
    });
    expectFailure([&pwm]()
    {
        pwm.setPeriod(XHAL_POSITIVE_INFINITY(float64));
    });
    expectFailure([&pwm]()
    {
        pwm.setPrescaler(XHAL_POSITIVE_INFINITY(float64));
    });
    expectFailure([&pwm]()
    {
        pwm.setPulseWidth(XHAL_POSITIVE_INFINITY(float64));
    });
    expectFailure([&pwm]()
    {
        pwm.setPulseWidthPercent(XHAL_POSITIVE_INFINITY(float64));
    });
}

} /* namespace xwalk::hal::test */
