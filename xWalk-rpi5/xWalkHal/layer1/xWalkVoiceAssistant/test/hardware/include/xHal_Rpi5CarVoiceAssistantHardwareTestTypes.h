/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantHardwareTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarVoiceAssistantHardwareTest.cpp.
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

#ifndef XHAL_RPI5CARVOICEASSISTANTHARDWARETESTTYPES_H
#define XHAL_RPI5CARVOICEASSISTANTHARDWARETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarVoiceAssistant.h"
#include "xHal_Rpi5CarAudioAlsa.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTextToSpeechAlsa.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carvoiceassistanthardwaretest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carvoiceassistanthardwaretest
{

    using namespace xwalk::hal;

    /** @brief Retains deployment-selected prompt and synthesis-fixture state. */
    struct SmokeProviders
    {
            XWalkHal::string prompt{};
            XWalkHal::filesystempath synthesisFixture{};
            XWalkHal::boolean gpioLevel{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carvoiceassistanthardwaretest */

#endif /* XHAL_RPI5CARVOICEASSISTANTHARDWARETESTTYPES_H */
