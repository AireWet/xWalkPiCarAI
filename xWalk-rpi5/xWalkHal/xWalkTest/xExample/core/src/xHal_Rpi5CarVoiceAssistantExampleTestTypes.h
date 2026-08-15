/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarVoiceAssistantExampleTest.cpp.
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

#ifndef XHAL_RPI5CARVOICEASSISTANTEXAMPLETESTTYPES_H
#define XHAL_RPI5CARVOICEASSISTANTEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarVoiceAssistantExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carvoiceassistantexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carvoiceassistantexampletest
{

    using namespace xwalk::hal;

    /** @brief Records deterministic assistant operations and supplied values. */
    struct VoiceAssistantExampleState
    {
            xwalk::hal::example::XWalkVoiceAssistantExampleConfiguration configuration;
            XWalkHal::stringvector keyboardInputs;
            XWalkHal::stringvector microphoneInputs;
            XWalkHal::stringvector spokenTexts;
            XWalkHal::stringvector spokenModels;
            XWalkHal::stringvector reports;
            XWalkHal::string promptModel;
            XWalkHal::string promptText;
            XWalkHal::string promptImage;
            XWalkHal::size keyboardIndex{};
            XWalkHal::size microphoneIndex{};
            XWalkHal::uint32 configureCount{};
            XWalkHal::uint32 captureCount{};
            XWalkHal::uint32 promptCount{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carvoiceassistantexampletest */

#endif /* XHAL_RPI5CARVOICEASSISTANTEXAMPLETESTTYPES_H */
