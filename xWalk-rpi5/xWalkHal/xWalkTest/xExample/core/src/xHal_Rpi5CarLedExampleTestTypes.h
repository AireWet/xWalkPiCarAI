/******************************************************************************
 * @file        xHal_Rpi5CarLedExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarLedExampleTest.cpp.
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

#ifndef XHAL_RPI5CARLEDEXAMPLETESTTYPES_H
#define XHAL_RPI5CARLEDEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLedExample.h"
#include "xHal_Rpi5CarTrace.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carledexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carledexampletest
{

    using namespace xwalk::hal;

    /** @brief Records every observable LED example operation. */
    struct LedExampleState
    {
            /** @brief Ordered LED operation names. */
            XWalkHal::stringvector operations;
            /** @brief Ordered source-compatible status messages. */
            XWalkHal::stringvector messages;
            /** @brief Ordered wait durations in milliseconds. */
            XWalkHal::uint32vector waits;
            /** @brief Ordered blink cycle counts. */
            XWalkHal::uint32vector blinkCounts;
            /** @brief Ordered blink transition delays in seconds. */
            XWalkHal::float64vector blinkDelays;
            /** @brief Ordered blink pauses in seconds. */
            XWalkHal::float64vector blinkPauses;
            /** @brief One-based wait callback that must throw, or zero for none. */
            XWalkHal::uint32 failingWait{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carledexampletest */

#endif /* XHAL_RPI5CARLEDEXAMPLETESTTYPES_H */
