/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechAlsaTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarTextToSpeechAlsaTest.cpp.
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

#ifndef XHAL_RPI5CARTEXTTOSPEECHALSATESTTYPES_H
#define XHAL_RPI5CARTEXTTOSPEECHALSATESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTextToSpeechAlsa.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5cartexttospeechalsatest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5cartexttospeechalsatest
{

    using namespace xwalk::hal;

    /** @brief Records observable provider and shared-audio operations. */
    struct TestBackend
    {
            /** @brief Stable non-null simulated PCM handle. */
            XWalkHal::uint8 pcmToken{1U};
            /** @brief Stable non-null simulated mixer handle. */
            XWalkHal::uint8 mixerToken{2U};
            /** @brief Provider PCM result returned by value. */
            XWalkHal::XWalkTextToSpeechPcmData audioData{};
            /** @brief Owned copy of the latest synthesis text. */
            XWalkHal::string text{};
            /** @brief Frame count accepted by each shared-audio write. */
            XWalkHal::uint32vector frameCounts{};
            /** @brief Complete PCM bytes accepted across all writes. */
            XWalkHal::bytevector writtenPcm{};
            /** @brief Number of synthesis calls. */
            XWalkHal::uint32 synthesisCount{};
            /** @brief Number of opened PCM handles. */
            XWalkHal::uint32 openCount{};
            /** @brief Number of closed PCM handles. */
            XWalkHal::uint32 closeCount{};
            /** @brief Most recent configured mixer volume. */
            XWalkHal::uint8 volumePercent{};
            /** @brief Makes synthesis report a provider failure when true. */
            XWalkHal::boolean failSynthesis{};
            /** @brief Makes PCM writes report an unrecoverable failure when true. */
            XWalkHal::boolean failWrite{};
    };

} /* namespace xwalk::source_types::xhal_rpi5cartexttospeechalsatest */

#endif /* XHAL_RPI5CARTEXTTOSPEECHALSATESTTYPES_H */
