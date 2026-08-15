/******************************************************************************
 * @file        xHal_Rpi5CarMotorSequenceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarMotorSequenceTest.cpp.
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

#ifndef XHAL_RPI5CARMOTORSEQUENCETESTTYPES_H
#define XHAL_RPI5CARMOTORSEQUENCETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMotorSequence.h"
#include "xHal_Rpi5CarTrace.h"
#include <cassert>
#include <vector>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carmotorsequencetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carmotorsequencetest
{

    using namespace xwalk::hal;

    struct TestI2c
    {
            XWalkHal::uint32 writeCount{};
    };

    struct TestGpio
    {
            XWalkHal::uint8 pin{};
            XWalkHal::boolean value{};
    };

    struct WaitState
    {
            XWalkHal::fixedarray<XWalkHal::XWalkMotor*, 2U> motors{};
            XWalkHal::fixedarray<TestGpio*, 2U> directions{};
            XWalkHal::uint32vector durations;
            XWalkHal::float64vector observedSpeeds;
            std::vector<XWalkHal::boolean> observedDirections;
            XWalkHal::uint32 failureWait{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carmotorsequencetest */

#endif /* XHAL_RPI5CARMOTORSEQUENCETESTTYPES_H */
