/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestAddress.cpp
 * @brief       Tests PWM address selection and channel-to-timer mapping.
 *
 * @details
 * Uses the in-memory I2C test double to verify probe order, selected address,
 * initialization registers, and the timer mapping for all twenty channels.
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
     * @brief Verifies address probing and timer mapping for channel `P19`.
     *
     * @post
     * The assertions confirm probe order, address selection, timer mapping,
     * and initialization-register selection.
     */
    void testAddressAndTimerSelection()
    {
        XWalkPwmTestI2c bus;
        bus.addPresentAddress(0x15U);
        XWalkI2c i2c(&bus,
                     &XWalkPwmTestI2c::probeCallback,
                     &XWalkPwmTestI2c::writeRegisterCallback,
                     &XWalkPwmTestI2c::readCallback);
        XWalkPwmTimerState timerState;
        XWalkPwm pwm(i2c, "P19", {}, timerState);

        const bytevector expectedProbes{0x14U, 0x15U};
        assert(pwm.channel() == 19U);
        assert(pwm.timerIndex() == 6U);
        assert(pwm.address() == 0x15U);
        assert(bus.probes() == expectedProbes);
        assert(bus.writeCount() == 2U);
        assert(bus.writeRegister(0U) == 0x52U);
        assert(bus.writeRegister(1U) == 0x56U);
    }

    /**
     * @brief Verifies the expected timer index for every numeric channel.
     *
     * @post
     * The assertions cover all twenty channel-to-timer mappings.
     */
    void testAllTimerMappings()
    {
        const uint32vector expected{0U, 0U, 0U, 0U, 1U, 1U, 1U, 1U, 2U, 2U, 2U, 2U, 3U, 3U, 3U, 3U, 4U, 4U, 5U, 6U};

        for (uint32 channel = 0U; channel < static_cast<uint32>(expected.size()); ++channel)
        {
            XWalkPwmTestI2c bus;
            XWalkI2c i2c(&bus,
                         &XWalkPwmTestI2c::probeCallback,
                         &XWalkPwmTestI2c::writeRegisterCallback,
                         &XWalkPwmTestI2c::readCallback);
            XWalkPwmTimerState timerState;
            XWalkPwm pwm(i2c, channel, 0x14U, timerState);
            assert(pwm.timerIndex() == expected[channel]);
        }
    }

} /* namespace xwalk::hal::test */
