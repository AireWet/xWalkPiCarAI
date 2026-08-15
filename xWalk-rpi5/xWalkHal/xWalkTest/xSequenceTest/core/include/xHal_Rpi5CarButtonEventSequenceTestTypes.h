/******************************************************************************
 * @file        xHal_Rpi5CarButtonEventSequenceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarButtonEventSequenceTest.cpp.
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

#ifndef XHAL_RPI5CARBUTTONEVENTSEQUENCETESTTYPES_H
#define XHAL_RPI5CARBUTTONEVENTSEQUENCETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarButtonEventSequence.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carbuttoneventsequencetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carbuttoneventsequencetest
{

    using namespace xwalk::hal;

    struct TestState
    {
            XWalkHal::uint8 pin{};
            XWalkHal::XWalkGpioMode mode{XWalkHal::XWalkGpioMode::Output};
            XWalkHal::XWalkGpioEdge edge{XWalkHal::XWalkGpioEdge::Falling};
            XWalkHal::XWalkGpioPull pull{XWalkHal::XWalkGpioPull::None};
            XWalkHal::uint32 debounceMilliseconds{};
            XWalkHal::uint32 waitedMilliseconds{};
            XWalkHal::uint32 cancelCount{};
            XWalkHal::contextpointer handlerContext{nullptr};
            XWalkHal::gpiointerrupthandler handler{nullptr};
            XWalkHal::boolean events[2U]{};
            XWalkHal::float64 timestamps[2U]{};
            XWalkHal::uint32 eventCount{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carbuttoneventsequencetest */

#endif /* XHAL_RPI5CARBUTTONEVENTSEQUENCETESTTYPES_H */
