/******************************************************************************
 * @file        xHal_Rpi5CarTraceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarTraceTest.cpp.
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

#ifndef XHAL_RPI5CARTRACETESTTYPES_H
#define XHAL_RPI5CARTRACETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarTraceBuildConfig.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <tinyxml2.h>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5cartracetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5cartracetest
{

    using namespace xwalk::hal;

    /** @brief Retains bounded synchronous callback output for host assertions. */
    struct TraceCapture
    {
            /** @brief Ordered accepted severities retained by the callback. */
            fixedarray<XWalkTraceLevel, 8U> levels{};
            /** @brief Ordered owned message copies retained by the callback. */
            fixedarray<string, 8U> messages{};
            /** @brief Number of valid records currently stored in the arrays. */
            size count{};
    };

} /* namespace xwalk::source_types::xhal_rpi5cartracetest */

#endif /* XHAL_RPI5CARTRACETESTTYPES_H */
