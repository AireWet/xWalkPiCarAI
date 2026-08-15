/******************************************************************************
 * @file        xHal_Rpi5CarOpenAiExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarOpenAiExampleTest.cpp.
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

#ifndef XHAL_RPI5CAROPENAIEXAMPLETESTTYPES_H
#define XHAL_RPI5CAROPENAIEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarOpenAiExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5caropenaiexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5caropenaiexampletest
{

    using namespace xwalk::hal;

    /** @brief Records OpenAI model and console operations from one example run. */
    struct OpenAiExampleState
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
            XWalkHal::stringvector responses{"Hello from OpenAI", "I am ready"};
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

} /* namespace xwalk::source_types::xhal_rpi5caropenaiexampletest */

#endif /* XHAL_RPI5CAROPENAIEXAMPLETESTTYPES_H */
