/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantBackendsTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarVoiceAssistantBackendsTest.cpp.
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

#ifndef XHAL_RPI5CARVOICEASSISTANTBACKENDSTESTTYPES_H
#define XHAL_RPI5CARVOICEASSISTANTBACKENDSTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarVoiceAssistant.h"
#include "xHal_Rpi5CarAudioAlsa.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTextToSpeechAlsa.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carvoiceassistantbackendstest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carvoiceassistantbackendstest
{

    using namespace xwalk::hal;

    /** @brief Retains all observable operations for one composed assistant round. */
    struct TestBackends
    {
            XWalkHal::uint8 captureToken{1U};
            XWalkHal::uint8 pcmToken{2U};
            XWalkHal::uint8 mixerToken{3U};
            XWalkHal::boolean gpioLevel{};
            XWalkHal::string modelRequest{};
            XWalkHal::string synthesizedText{};
            XWalkHal::size capturedBytes{};
            XWalkHal::size playedBytes{};
            XWalkHal::uint32 primeCount{};
            XWalkHal::uint32 recognitionCount{};
            XWalkHal::uint32 modelCount{};
            XWalkHal::uint32 synthesisCount{};
            XWalkHal::uint32 streamCloseCount{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carvoiceassistantbackendstest */

#endif /* XHAL_RPI5CARVOICEASSISTANTBACKENDSTESTTYPES_H */
