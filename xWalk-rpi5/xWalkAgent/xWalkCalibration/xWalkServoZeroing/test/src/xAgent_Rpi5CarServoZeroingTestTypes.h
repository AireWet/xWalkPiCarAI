/******************************************************************************
 * @file        xAgent_Rpi5CarServoZeroingTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarServoZeroingTest.cpp.
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

#ifndef XAGENT_RPI5CARSERVOZEROINGTESTTYPES_H
#define XAGENT_RPI5CARSERVOZEROINGTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarServoZeroing.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xagent_rpi5carservozeroingtest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carservozeroingtest
{

    using namespace xwalk::hal;
    using namespace xwalk::agent;

    /** @brief Stores deterministic angle and cancellation observations. */
    struct TestState
    {
            xwalk::agent::uint32vector servoIds{};
            xwalk::agent::float64vector angles{};
            xwalk::agent::uint32vector delays{};
            xwalk::agent::uint32 continueQueries{};
    };

} /* namespace xwalk::source_types::xagent_rpi5carservozeroingtest */

#endif /* XAGENT_RPI5CARSERVOZEROINGTESTTYPES_H */
