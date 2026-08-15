/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarSttVoskWakeWordExampleTest.cpp.
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

#ifndef XHAL_RPI5CARSTTVOSKWAKEWORDEXAMPLETESTTYPES_H
#define XHAL_RPI5CARSTTVOSKWAKEWORDEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskWakeWordExample.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carsttvoskwakewordexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carsttvoskwakewordexampletest
{

    using namespace xwalk::hal;

    /** @brief Records deterministic transcripts, timeouts, and status messages. */
    struct WakeWordExampleState
    {
            XWalkHal::stringvector transcripts{"background noise", "HEY ROBOT please wake"};
            XWalkHal::stringvector messages;
            XWalkHal::uint32vector timeouts;
            XWalkHal::size transcriptIndex{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carsttvoskwakewordexampletest */

#endif /* XHAL_RPI5CARSTTVOSKWAKEWORDEXAMPLETESTTYPES_H */
