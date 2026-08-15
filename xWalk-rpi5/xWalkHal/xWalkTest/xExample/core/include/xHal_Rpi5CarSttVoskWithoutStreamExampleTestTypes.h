/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWithoutStreamExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarSttVoskWithoutStreamExampleTest.cpp.
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

#ifndef XHAL_RPI5CARSTTVOSKWITHOUTSTREAMEXAMPLETESTTYPES_H
#define XHAL_RPI5CARSTTVOSKWITHOUTSTREAMEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskWithoutStreamExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carsttvoskwithoutstreamexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carsttvoskwithoutstreamexampletest
{

    using namespace xwalk::hal;

    /** @brief Records deterministic recognition and reporting activity. */
    struct WithoutStreamExampleState
    {
            XWalkHal::stringvector transcripts{"hello", "robot ready"};
            XWalkHal::stringvector reports;
            XWalkHal::uint32vector timeouts;
            XWalkHal::size transcriptIndex{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carsttvoskwithoutstreamexampletest */

#endif /* XHAL_RPI5CARSTTVOSKWITHOUTSTREAMEXAMPLETESTTYPES_H */
