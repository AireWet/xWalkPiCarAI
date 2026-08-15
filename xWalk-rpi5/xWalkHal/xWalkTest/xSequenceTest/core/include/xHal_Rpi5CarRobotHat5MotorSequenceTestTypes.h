/******************************************************************************
 * @file        xHal_Rpi5CarRobotHat5MotorSequenceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarRobotHat5MotorSequenceTest.cpp.
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

#ifndef XHAL_RPI5CARROBOTHAT5MOTORSEQUENCETESTTYPES_H
#define XHAL_RPI5CARROBOTHAT5MOTORSEQUENCETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarRobotHat5MotorSequence.h"
#include "xHal_Rpi5CarTrace.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carrobothat5motorsequencetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carrobothat5motorsequencetest
{

    using namespace xwalk::hal;

    struct TestI2c
    {
            XWalkHal::uint32 writeCount{};
    };

    struct WaitState
    {
            XWalkHal::fixedarray<XWalkHal::XWalkMotor*, 4U> motors{};
            XWalkHal::uint32vector durations;
            XWalkHal::float64vector observedSpeeds;
            XWalkHal::boolean failNextWait{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carrobothat5motorsequencetest */

#endif /* XHAL_RPI5CARROBOTHAT5MOTORSEQUENCETESTTYPES_H */
