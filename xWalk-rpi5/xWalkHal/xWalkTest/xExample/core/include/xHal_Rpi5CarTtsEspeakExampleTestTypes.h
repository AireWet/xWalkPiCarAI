/******************************************************************************
 * @file        xHal_Rpi5CarTtsEspeakExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarTtsEspeakExampleTest.cpp.
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

#ifndef XHAL_RPI5CARTTSESPEAKEXAMPLETESTTYPES_H
#define XHAL_RPI5CARTTSESPEAKEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTtsEspeakExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carttsespeakexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carttsespeakexampletest
{

    using namespace xwalk::hal;

    /** @brief Records one deterministic configured speech request. */
    struct TtsEspeakExampleState
    {
            XWalkHal::uint8 amplitude{};
            XWalkHal::uint16 speed{};
            XWalkHal::uint16 gap{};
            XWalkHal::uint8 pitch{};
            XWalkHal::string text;
            XWalkHal::uint32 callCount{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carttsespeakexampletest */

#endif /* XHAL_RPI5CARTTSESPEAKEXAMPLETESTTYPES_H */
