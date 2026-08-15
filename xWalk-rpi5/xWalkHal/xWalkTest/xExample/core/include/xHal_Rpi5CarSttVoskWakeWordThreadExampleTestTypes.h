/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordThreadExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarSttVoskWakeWordThreadExampleTest.cpp.
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

#ifndef XHAL_RPI5CARSTTVOSKWAKEWORDTHREADEXAMPLETESTTYPES_H
#define XHAL_RPI5CARSTTVOSKWAKEWORDTHREADEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskWakeWordThreadExample.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carsttvoskwakewordthreadexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carsttvoskwakewordthreadexampletest
{

    using namespace xwalk::hal;

    /** @brief Records deterministic wake-word lifecycle and reporting activity. */
    struct WakeWordExampleState
    {
            XWalkHal::uint32 starts{};
            XWalkHal::uint32 stops{};
            XWalkHal::uint32 pollInAttempt{};
            XWalkHal::uint32 wakeAfterPoll{1U};
            XWalkHal::uint32vector waits;
            XWalkHal::stringvector messages;
    };

} /* namespace xwalk::source_types::xhal_rpi5carsttvoskwakewordthreadexampletest */

#endif /* XHAL_RPI5CARSTTVOSKWAKEWORDTHREADEXAMPLETESTTYPES_H */
