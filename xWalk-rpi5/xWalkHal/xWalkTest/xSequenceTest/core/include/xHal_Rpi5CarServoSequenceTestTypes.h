/******************************************************************************
 * @file        xHal_Rpi5CarServoSequenceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarServoSequenceTest.cpp.
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

#ifndef XHAL_RPI5CARSERVOSEQUENCETESTTYPES_H
#define XHAL_RPI5CARSERVOSEQUENCETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServoSequence.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carservosequencetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carservosequencetest
{

    using namespace xwalk::hal;

    /** @brief Counts writes performed through the simulated Robot HAT I2C bus. */
    struct I2cState
    {
            /** @brief Total number of simulated register writes. */
            XWalkHal::uint32 writeCount{};
    };

    /** @brief Collects wait durations and complete PWM snapshots. */
    struct WaitState
    {
            /** @brief Non-owning PWM pointers ordered by channels zero through 11. */
            XWalkHal::fixedarray<XWalkHal::XWalkPwm*, 12U> pwmObjects{};
            /** @brief Ordered callback wait durations in milliseconds. */
            XWalkHal::uint32vector durations;
            /** @brief Flattened PWM snapshots captured after every angle command. */
            XWalkHal::uint32vector pulseWidthSnapshots;
    };

} /* namespace xwalk::source_types::xhal_rpi5carservosequencetest */

#endif /* XHAL_RPI5CARSERVOSEQUENCETESTTYPES_H */
