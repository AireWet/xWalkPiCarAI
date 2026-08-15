/******************************************************************************
 * @file        xHal_Rpi5CarPiperStreamSequenceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarPiperStreamSequenceTest.cpp.
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

#ifndef XHAL_RPI5CARPIPERSTREAMSEQUENCETESTTYPES_H
#define XHAL_RPI5CARPIPERSTREAMSEQUENCETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarPiperStreamSequence.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carpiperstreamsequencetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carpiperstreamsequencetest
{

    using namespace xwalk::hal;

    /** @brief Records all observable sequence operations in call order. */
    struct TestState
    {
            /** @brief Model values received by the simulated Piper provider. */
            XWalkHal::stringvector models;
            /** @brief Text values received by the simulated Piper provider. */
            XWalkHal::stringvector texts;
            /** @brief Ordered literal messages and synthesized reporting events. */
            XWalkHal::stringvector events;
            /** @brief Durations reported for streamed and buffered synthesis. */
            XWalkHal::float64vector durations;
            /** @brief Deterministic monotonic values returned in call order. */
            XWalkHal::fixedarray<XWalkHal::float64, 4U> times{{10.0, 12.5, 20.0, 23.0}};
            /** @brief Stream-mode flags received by the two provider calls. */
            XWalkHal::boolean streamModes[2U]{};
            /** @brief Number of provider calls recorded so far. */
            XWalkHal::uint32 speakCount{};
            /** @brief Index of the next deterministic clock value. */
            XWalkHal::size timeIndex{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carpiperstreamsequencetest */

#endif /* XHAL_RPI5CARPIPERSTREAMSEQUENCETESTTYPES_H */
