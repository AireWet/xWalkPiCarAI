/******************************************************************************
 * @file        xHal_Rpi5CarDoubaoExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarDoubaoExampleTest.cpp.
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

#ifndef XHAL_RPI5CARDOUBAOEXAMPLETESTTYPES_H
#define XHAL_RPI5CARDOUBAOEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarDoubaoExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5cardoubaoexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5cardoubaoexampletest
{

    using namespace xwalk::hal;

    /** @brief Records model and console operations from one example run. */
    struct DoubaoExampleState
    {
            /** @brief Configured system instructions. */
            XWalkHal::string instructions;
            /** @brief Configured assistant welcome text. */
            XWalkHal::string welcome;
            /** @brief Configured retained-message limit. */
            XWalkHal::uint32 maximumMessages{};
            /** @brief Input lines returned in order. */
            XWalkHal::stringvector inputs{"Hello", "How are you?"};
            /** @brief Model responses returned in order. */
            XWalkHal::stringvector responses{"Hello from Doubao", "I am ready"};
            /** @brief Prompt text received by the model. */
            XWalkHal::stringvector prompts;
            /** @brief Image paths received by the text-only model flow. */
            XWalkHal::stringvector imagePaths;
            /** @brief Output fragments written by the example. */
            XWalkHal::stringvector outputs;
            /** @brief Newline flags corresponding to output fragments. */
            XWalkHal::uint32vector newlineFlags;
            /** @brief Flush flags corresponding to output fragments. */
            XWalkHal::uint32vector flushFlags;
            /** @brief Index of the next console input. */
            XWalkHal::size inputIndex{};
    };

} /* namespace xwalk::source_types::xhal_rpi5cardoubaoexampletest */

#endif /* XHAL_RPI5CARDOUBAOEXAMPLETESTTYPES_H */
