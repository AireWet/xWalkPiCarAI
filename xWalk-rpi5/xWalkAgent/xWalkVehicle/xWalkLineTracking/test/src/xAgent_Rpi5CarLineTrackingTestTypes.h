/******************************************************************************
 * @file        xAgent_Rpi5CarLineTrackingTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarLineTrackingTest.cpp.
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

#ifndef XAGENT_RPI5CARLINETRACKINGTESTTYPES_H
#define XAGENT_RPI5CARLINETRACKINGTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarLineTracking.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xagent_rpi5carlinetrackingtest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carlinetrackingtest
{

    using namespace xwalk::hal;
    using namespace xwalk::agent;

    /** @brief Provides an ordered sequence of simulated ADC sample counts. */
    struct TestBus
    {
            agent::uint32vector samples{};
            agent::size nextSample{};
    };

    /** @brief Stores one simulated GPIO level. */
    struct TestGpio
    {
            agent::boolean value{};
    };

    /** @brief Records requested line-recovery delays. */
    struct TestTiming
    {
            agent::uint32vector delays{};
    };

} /* namespace xwalk::source_types::xagent_rpi5carlinetrackingtest */

#endif /* XAGENT_RPI5CARLINETRACKINGTESTTYPES_H */
