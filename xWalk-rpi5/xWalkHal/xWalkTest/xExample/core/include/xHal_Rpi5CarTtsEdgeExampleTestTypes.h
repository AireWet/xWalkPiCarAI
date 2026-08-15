/******************************************************************************
 * @file        xHal_Rpi5CarTtsEdgeExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarTtsEdgeExampleTest.cpp.
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

#ifndef XHAL_RPI5CARTTSEDGEEXAMPLETESTTYPES_H
#define XHAL_RPI5CARTTSEDGEEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTtsEdgeExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carttsedgeexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carttsedgeexampletest
{

    using namespace xwalk::hal;

    /** @brief Records one deterministic voice and speech message. */
    struct TtsEdgeExampleState
    {
            XWalkHal::string voice;
            XWalkHal::string text;
            XWalkHal::uint32 callCount{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carttsedgeexampletest */

#endif /* XHAL_RPI5CARTTSEDGEEXAMPLETESTTYPES_H */
