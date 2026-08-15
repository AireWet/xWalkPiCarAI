/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestOutput.cpp
 * @brief       Tests PWM register encoding and shared-period output behavior.
 *
 * @details
 * Verifies high-byte-first 16-bit output and percentage conversion using timer
 * state shared by two PWM channel objects.
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
     * @brief Verifies high-byte-first encoding of a 16-bit pulse width.
     *
     * @post
     * The assertions confirm the address, register, and encoded payload bytes.
     */
    void testBigEndianRegisterData()
    {
        XWalkPwmTestI2c bus;
        XWalkI2c i2c(&bus,
                     &XWalkPwmTestI2c::probeCallback,
                     &XWalkPwmTestI2c::writeRegisterCallback,
                     &XWalkPwmTestI2c::readCallback);
        XWalkPwmTimerState timerState;
        XWalkPwm pwm(i2c, 0U, 0x14U, timerState);
        bus.clearWrites();
        pwm.setPulseWidth(4660.0);

        const bytevector expectedBytes{0x12U, 0x34U};
        assert(bus.writeCount() == 1U);
        assert(bus.writeAddress(0U) == 0x14U);
        assert(bus.writeRegister(0U) == 0x20U);
        assert(bus.writeData(0U) == expectedBytes);
    }

    /**
     * @brief Verifies duty-cycle conversion against shared timer state.
     *
     * @post
     * The assertions confirm shared period observation and truncated output.
     */
    void testSharedPeriodAndPercentage()
    {
        XWalkPwmTestI2c bus;
        XWalkI2c i2c(&bus,
                     &XWalkPwmTestI2c::probeCallback,
                     &XWalkPwmTestI2c::writeRegisterCallback,
                     &XWalkPwmTestI2c::readCallback);
        XWalkPwmTimerState timerState;
        XWalkPwm pwm0(i2c, 0U, 0x14U, timerState);
        XWalkPwm pwm1(i2c, 1U, 0x14U, timerState);
        pwm0.setPeriod(4095.0);
        bus.clearWrites();
        pwm1.setPulseWidthPercent(50.0);

        const bytevector expectedBytes{0x07U, 0xFFU};
        assert(pwm1.period() == 4095U);
        /* Preserve the Python implementation's integer truncation behavior. */
        assert(pwm1.pulseWidth() == 2047U);
        assert(bus.writeRegister(0U) == 0x21U);
        assert(bus.writeData(0U) == expectedBytes);
    }

} /* namespace xwalk::hal::test */
