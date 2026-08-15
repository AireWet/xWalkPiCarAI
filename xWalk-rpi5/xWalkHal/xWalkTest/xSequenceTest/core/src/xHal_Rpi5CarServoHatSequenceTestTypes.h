/******************************************************************************
 * @file        xHal_Rpi5CarServoHatSequenceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarServoHatSequenceTest.cpp.
 *
 * @project     xWalk Firmware
 * @module      Source Type Support
 *
 * @author      Joxy John
 * @date        2026-08-15
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CARSERVOHATSEQUENCETESTTYPES_H
#define XHAL_RPI5CARSERVOHATSEQUENCETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServoHatSequence.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carservohatsequencetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carservohatsequencetest
{

    using namespace xwalk::hal;

    /** @brief Records the ordered values written to one simulated GPIO line. */
    struct GpioState
    {
            /** @brief First two logical output values written by the reset operation. */
            XWalkHal::boolean writes[2U]{};
            /** @brief Total number of simulated GPIO writes. */
            XWalkHal::uint32 writeCount{};
    };

    /** @brief Counts writes performed through the simulated Robot HAT I2C bus. */
    struct I2cState
    {
            /** @brief Total number of simulated register writes. */
            XWalkHal::uint32 writeCount{};
    };

    /** @brief Collects observable sequence callbacks and PWM state. */
    struct SequenceState
    {
            /** @brief Non-owning PWM pointers ordered by channels zero through 15. */
            XWalkHal::fixedarray<XWalkHal::XWalkPwm*, 16U> pwmObjects{};
            /** @brief Ordered callback wait durations in milliseconds. */
            XWalkHal::uint32vector durations;
            /** @brief Servo channels reported before their movement. */
            XWalkHal::uint32vector reportedServoChannels;
            /** @brief PWM counts observed after each servo angle command. */
            XWalkHal::uint32vector observedPulseWidths;
            /** @brief Most recently reported ADC values ordered by channel. */
            xwalk::hal::test::servohatreadings lastReadings{};
            /** @brief Number of complete five-channel ADC reports. */
            XWalkHal::uint32 adcReportCount{};
            /** @brief Servo channel whose next PWM state is observed. */
            XWalkHal::uint8 currentServoChannel{16U};
    };

} /* namespace xwalk::source_types::xhal_rpi5carservohatsequencetest */

#endif /* XHAL_RPI5CARSERVOHATSEQUENCETESTTYPES_H */
