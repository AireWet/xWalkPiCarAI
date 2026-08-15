/******************************************************************************
 * @file        xHal_Rpi5CarTtsOpenAiExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarTtsOpenAiExampleTest.cpp.
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

#ifndef XHAL_RPI5CARTTSOPENAIEXAMPLETESTTYPES_H
#define XHAL_RPI5CARTTSOPENAIEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTtsOpenAiExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carttsopenaiexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carttsopenaiexampletest
{

    using namespace xwalk::hal;

    /** @brief Records one request for exact-value verification. */
    struct TtsOpenAiRequest
    {
            XWalkHal::string model;
            XWalkHal::string voice;
            XWalkHal::string text;
            XWalkHal::string instructions;
    };

    /** @brief Records all deterministic requests and report messages. */
    struct TtsOpenAiExampleState
    {
            std::vector<TtsOpenAiRequest> requests;
            XWalkHal::stringvector reports;
    };

} /* namespace xwalk::source_types::xhal_rpi5carttsopenaiexampletest */

#endif /* XHAL_RPI5CARTTSOPENAIEXAMPLETESTTYPES_H */
