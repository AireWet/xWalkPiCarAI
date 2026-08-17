/******************************************************************************
 * @file        xControllerTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xControllerTest.cpp.
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

#ifndef XCONTROLLERTESTTYPES_H
#define XCONTROLLERTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xController.h"
#include "xControllerCommands.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xcontrollertest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xcontrollertest
{

    using namespace xwalk::hal;
    using namespace xwalk::controller;

    /** @brief Supplies one deterministic 1,000-count ADC sample. */
    struct TestBus
    {
            ::ctrl::bytevector sample{0x03U, 0xE8U};
    };

    /** @brief Stores one simulated GPIO level. */
    struct TestGpio
    {
            ::ctrl::boolean value{};
    };

    /** @brief Records CLI platform interactions. */
    struct TestCliBackend
    {
            ::ctrl::stringvector outputLines;
            ::ctrl::stringvector inputLines;
            ::ctrl::size inputIndex{};
            ::ctrl::uint32vector delays;
            ::ctrl::uint64 monotonicMilliseconds{}; /**< Simulated elapsed monotonic time. */
            ::ctrl::uint32 delayOverrunMs{};        /**< Simulated scheduler overrun per delay callback. */
            ::ctrl::float64vector leftSpeeds;
            ::ctrl::float64vector rightSpeeds;
            ::ctrl::float64vector steeringAngles;
            xwalk::hal::XWalkMotors* motors{nullptr};
            xwalk::agent::XWalkPicarx* picarx{nullptr};
            ::ctrl::boolean soundAvailable{true};
            xwalk::ctrl::XWalkSoundRequest soundRequest{};
            ::ctrl::uint32 operationQueries{};
            ::ctrl::uint32 operationQueryLimit{1'000'000U};
            ::ctrl::string musicSoundFile;
            ::ctrl::boolean failDelay{};
            ::ctrl::boolean failSound{};
    };

} /* namespace xwalk::source_types::xcontrollertest */

#endif /* XCONTROLLERTESTTYPES_H */
