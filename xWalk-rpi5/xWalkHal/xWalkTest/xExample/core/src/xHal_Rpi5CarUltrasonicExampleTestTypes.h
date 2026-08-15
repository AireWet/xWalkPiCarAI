/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarUltrasonicExampleTest.cpp.
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

#ifndef XHAL_RPI5CARULTRASONICEXAMPLETESTTYPES_H
#define XHAL_RPI5CARULTRASONICEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarUltrasonicExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carultrasonicexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carultrasonicexampletest
{

    using namespace xwalk::hal;

    /** @brief Records distances, reports, waits, and the next read index. */
    struct UltrasonicExampleState
    {
            XWalkHal::float64vector values{12.5, 24.75, -1.0};
            XWalkHal::float64vector reports;
            XWalkHal::uint32vector waits;
            XWalkHal::size readIndex{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carultrasonicexampletest */

#endif /* XHAL_RPI5CARULTRASONICEXAMPLETESTTYPES_H */
