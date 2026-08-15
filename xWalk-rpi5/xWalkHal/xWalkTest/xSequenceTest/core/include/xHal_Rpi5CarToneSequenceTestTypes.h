/******************************************************************************
 * @file        xHal_Rpi5CarToneSequenceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarToneSequenceTest.cpp.
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

#ifndef XHAL_RPI5CARTONESEQUENCETESTTYPES_H
#define XHAL_RPI5CARTONESEQUENCETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarToneSequence.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5cartonesequencetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5cartonesequencetest
{

    using namespace xwalk::hal;

    /** @brief Records observable operations from the in-memory music backend. */
    struct ToneState
    {
            /** @brief Number of output-enable operations. */
            XWalkHal::uint32 enableCount{};
            /** @brief Number of volume operations. */
            XWalkHal::uint32 volumeCount{};
            /** @brief Last normalized volume value. */
            XWalkHal::float64 normalizedVolume{};
            /** @brief Number of generated tone payloads. */
            XWalkHal::uint32 toneCount{};
            /** @brief Total generated PCM bytes across the melody. */
            XWalkHal::size totalPcmBytes{};
            /** @brief Last generated sample rate. */
            XWalkHal::uint32 sampleRateHz{};
            /** @brief Last generated channel count. */
            XWalkHal::uint8 channelCount{};
            /** @brief Ordered measure reports. */
            XWalkHal::uint32vector measures;
    };

} /* namespace xwalk::source_types::xhal_rpi5cartonesequencetest */

#endif /* XHAL_RPI5CARTONESEQUENCETESTTYPES_H */
