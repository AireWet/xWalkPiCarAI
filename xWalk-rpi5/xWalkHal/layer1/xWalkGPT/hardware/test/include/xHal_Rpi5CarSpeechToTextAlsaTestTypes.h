/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextAlsaTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarSpeechToTextAlsaTest.cpp.
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

#ifndef XHAL_RPI5CARSPEECHTOTEXTALSATESTTYPES_H
#define XHAL_RPI5CARSPEECHTOTEXTALSATESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carspeechtotextalsatest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carspeechtotextalsatest
{

    using namespace xwalk::hal;

    struct TestBackend
    {
            XWalkHal::uint8 token{1U};
            XWalkHal::size capturedFrames{};
            XWalkHal::size recognizedBytes{};
            XWalkHal::uint32 readCount{};
            XWalkHal::uint32 closeCount{};
            XWalkHal::uint32 cancelCount{};
            XWalkHal::boolean failFirstRead{};
            XWalkHal::boolean delayRead{};
            XWalkHal::atomicboolean readStarted{false};
    };

} /* namespace xwalk::source_types::xhal_rpi5carspeechtotextalsatest */

#endif /* XHAL_RPI5CARSPEECHTOTEXTALSATESTTYPES_H */
