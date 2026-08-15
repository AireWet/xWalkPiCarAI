/******************************************************************************
 * @file        xHal_Rpi5CarServoExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarServoExampleTest.cpp.
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

#ifndef XHAL_RPI5CARSERVOEXAMPLETESTTYPES_H
#define XHAL_RPI5CARSERVOEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServoExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carservoexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carservoexampletest
{

    using namespace xwalk::hal;

    struct ServoExampleState
    {
            /** @brief Ordered angles commanded by the example. */
            XWalkHal::float64vector angles;
            /** @brief Ordered wait durations requested by the example. */
            XWalkHal::uint32vector waits;
            /** @brief Ordered angle values reported by the example. */
            XWalkHal::float64vector reports;
    };

} /* namespace xwalk::source_types::xhal_rpi5carservoexampletest */

#endif /* XHAL_RPI5CARSERVOEXAMPLETESTTYPES_H */
