/******************************************************************************
 * @file        xHal_Rpi5CarInitAnglesSequenceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarInitAnglesSequenceTest.cpp.
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

#ifndef XHAL_RPI5CARINITANGLESSEQUENCETESTTYPES_H
#define XHAL_RPI5CARINITANGLESSEQUENCETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarInitAnglesSequence.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carinitanglessequencetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carinitanglessequencetest
{

    using namespace xwalk::hal;

    struct GpioState
    {
            XWalkHal::boolean writes[2U]{};
            XWalkHal::uint32 writeCount{};
    };

    struct I2cState
    {
            XWalkHal::uint32 writeCount{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carinitanglessequencetest */

#endif /* XHAL_RPI5CARINITANGLESSEQUENCETESTTYPES_H */
