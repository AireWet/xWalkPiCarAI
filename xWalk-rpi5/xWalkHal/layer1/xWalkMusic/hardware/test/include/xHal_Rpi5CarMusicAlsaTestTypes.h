/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsaTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarMusicAlsaTest.cpp.
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

#ifndef XHAL_RPI5CARMUSICALSATESTTYPES_H
#define XHAL_RPI5CARMUSICALSATESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMusicAlsa.h"
#include "xHal_Rpi5CarMusic.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include <fstream>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carmusicalsatest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carmusicalsatest
{

    using namespace xwalk::hal;

    /** @brief Records deterministic decoder and shared-audio operations. */
    struct TestBackend
    {
            /** @brief Stable non-null PCM token returned by every sequential open. */
            XWalkHal::uint8 pcmToken{1U};
            /** @brief Stable non-null mixer token returned during audio construction. */
            XWalkHal::uint8 mixerToken{2U};
            /** @brief Number of decoder callback invocations. */
            XWalkHal::uint32 decodeCount{};
            /** @brief Number of configured PCM streams. */
            XWalkHal::uint32 configureCount{};
            /** @brief Number of PCM write callback invocations. */
            XWalkHal::uint32 writeCount{};
            /** @brief Number of PCM close callback invocations. */
            XWalkHal::uint32 closeCount{};
            /** @brief Total complete PCM bytes accepted by write callbacks. */
            XWalkHal::size writtenBytes{};
            /** @brief Most recently requested mixer percentage. */
            XWalkHal::uint8 volumePercent{};
            /** @brief `true` after at least one mixer-volume callback. */
            XWalkHal::boolean volumeSet{};
            /** @brief `true` when the most recent PCM open followed a mixer-volume
             * callback. */
            XWalkHal::boolean volumeObservedAtOpen{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carmusicalsatest */

#endif /* XHAL_RPI5CARMUSICALSATESTTYPES_H */
