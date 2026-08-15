/******************************************************************************
 * @file        xHal_Rpi5CarDeepseekExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarDeepseekExampleTest.cpp.
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

#ifndef XHAL_RPI5CARDEEPSEEKEXAMPLETESTTYPES_H
#define XHAL_RPI5CARDEEPSEEKEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarDeepseekExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5cardeepseekexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5cardeepseekexampletest
{

    using namespace xwalk::hal;

    /** @brief Records model and console operations from one example run. */
    struct DeepseekExampleState
    {
            /** @brief Configured system instructions. */
            XWalkHal::string instructions;
            /** @brief Configured assistant welcome text. */
            XWalkHal::string welcome;
            /** @brief Configured retained-message limit. */
            XWalkHal::uint32 maximumMessages{};
            /** @brief Input lines returned in order. */
            XWalkHal::stringvector inputs{"Hello", ""};
            /** @brief Model responses returned in order. */
            XWalkHal::stringvector responses{"Hello from the model", ""};
            /** @brief Prompt text received by the model. */
            XWalkHal::stringvector prompts;
            /** @brief Output fragments written by the example. */
            XWalkHal::stringvector outputs;
            /** @brief Newline flags corresponding to output fragments. */
            XWalkHal::uint32vector newlineFlags;
            /** @brief Flush flags corresponding to output fragments. */
            XWalkHal::uint32vector flushFlags;
            /** @brief Index of the next console input. */
            XWalkHal::size inputIndex{};
    };

} /* namespace xwalk::source_types::xhal_rpi5cardeepseekexampletest */

#endif /* XHAL_RPI5CARDEEPSEEKEXAMPLETESTTYPES_H */
