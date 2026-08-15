/******************************************************************************
 * @file        xHal_Rpi5CarPinInputExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarPinInputExampleTest.cpp.
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

#ifndef XHAL_RPI5CARPININPUTEXAMPLETESTTYPES_H
#define XHAL_RPI5CARPININPUTEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarPinInputExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carpininputexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carpininputexampletest
{

    using namespace xwalk::hal;

    /** @brief Records pin values, reports, waits, and the next read index. */
    struct PinInputExampleState
    {
            XWalkHal::uint32vector values{1U, 0U, 1U};
            XWalkHal::uint32vector reports;
            XWalkHal::uint32vector waits;
            XWalkHal::size readIndex{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carpininputexampletest */

#endif /* XHAL_RPI5CARPININPUTEXAMPLETESTTYPES_H */
