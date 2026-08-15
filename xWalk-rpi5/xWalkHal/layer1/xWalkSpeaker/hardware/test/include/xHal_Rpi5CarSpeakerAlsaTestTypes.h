/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerAlsaTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarSpeakerAlsaTest.cpp.
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

#ifndef XHAL_RPI5CARSPEAKERALSATESTTYPES_H
#define XHAL_RPI5CARSPEAKERALSATESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeaker.h"
#include "xHal_Rpi5CarSpeakerAlsa.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <fstream>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carspeakeralsatest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carspeakeralsatest
{

    using namespace xwalk::hal;

    /** @brief Records every observable adapter operation. */
    struct TestBackend
    {
            /** @brief Stable non-null simulated PCM handle. */
            XWalkHal::uint8 pcmToken{1U};
            /** @brief Stable non-null simulated mixer handle. */
            XWalkHal::uint8 mixerToken{2U};
            /** @brief Frame count returned by the optional decoder. */
            XWalkHal::size decodedFrameCount{4'096U};
            /** @brief Most recent decoder family. */
            XWalkHal::XWalkSpeakerAudioHandler handler{XWalkHal::XWalkSpeakerAudioHandler::SoundFile};
            /** @brief Number of decode operations. */
            XWalkHal::uint32 decodeCount{};
            /** @brief Number of configured streams. */
            XWalkHal::uint32 configureCount{};
            /** @brief Number of PCM write operations. */
            XWalkHal::uint32 writeCount{};
            /** @brief Number of PCM close operations. */
            XWalkHal::uint32 closeCount{};
            /** @brief Most recent configured mixer volume. */
            XWalkHal::uint8 volumePercent{};
            /** @brief Complete PCM bytes accepted across writes. */
            XWalkHal::bytevector writtenPcm{};
            /** @brief `true` when PCM writes must report a failure. */
            XWalkHal::boolean failWrite{};
            /** @brief `true` when decoded samples must have incomplete stereo alignment. */
            XWalkHal::boolean invalidAudio{};
            /** @brief `true` when decoded samples must exceed the configured bound. */
            XWalkHal::boolean excessiveAudio{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carspeakeralsatest */

#endif /* XHAL_RPI5CARSPEAKERALSATESTTYPES_H */
