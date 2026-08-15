/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskStreamExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarSttVoskStreamExampleTest.cpp.
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

#ifndef XHAL_RPI5CARSTTVOSKSTREAMEXAMPLETESTTYPES_H
#define XHAL_RPI5CARSTTVOSKSTREAMEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskStreamExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carsttvoskstreamexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carsttvoskstreamexampletest
{

    using namespace xwalk::hal;

    /** @brief Records deterministic prompt, listen, and streamed-result activity. */
    struct SttVoskStreamExampleState
    {
            XWalkHal::uint32 promptCount{};
            XWalkHal::uint32 listenCount{};
            XWalkHal::uint32 timeoutMs{};
            XWalkHal::stringvector results;
    };

} /* namespace xwalk::source_types::xhal_rpi5carsttvoskstreamexampletest */

#endif /* XHAL_RPI5CARSTTVOSKSTREAMEXAMPLETESTTYPES_H */
