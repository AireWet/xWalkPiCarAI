/******************************************************************************
 * @file        xHal_Rpi5CarDoubaoImageExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarDoubaoImageExampleTest.cpp.
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

#ifndef XHAL_RPI5CARDOUBAOIMAGEEXAMPLETESTTYPES_H
#define XHAL_RPI5CARDOUBAOIMAGEEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarDoubaoImageExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5cardoubaoimageexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5cardoubaoimageexampletest
{

    using namespace xwalk::hal;

    /** @brief Records every operation from one Doubao image example run. */
    struct DoubaoImageExampleState
    {
            /** @brief Configured system instructions. */
            XWalkHal::string instructions;
            /** @brief Configured assistant welcome text. */
            XWalkHal::string welcome;
            /** @brief Configured retained-message limit. */
            XWalkHal::uint32 maximumMessages{};
            /** @brief Input lines returned in order. */
            XWalkHal::stringvector inputs{"What do you see?", "And now?"};
            /** @brief Model responses returned in order. */
            XWalkHal::stringvector responses{"A test image", "Another test image"};
            /** @brief Prompt text received by the model. */
            XWalkHal::stringvector prompts;
            /** @brief Image paths received by the model. */
            XWalkHal::stringvector modelImagePaths;
            /** @brief Output paths requested from the camera. */
            XWalkHal::stringvector capturePaths;
            /** @brief Output fragments written by the example. */
            XWalkHal::stringvector outputs;
            /** @brief Newline flags corresponding to output fragments. */
            XWalkHal::uint32vector newlineFlags;
            /** @brief Flush flags corresponding to output fragments. */
            XWalkHal::uint32vector flushFlags;
            /** @brief Index of the next console input. */
            XWalkHal::size inputIndex{};
    };

} /* namespace xwalk::source_types::xhal_rpi5cardoubaoimageexampletest */

#endif /* XHAL_RPI5CARDOUBAOIMAGEEXAMPLETESTTYPES_H */
