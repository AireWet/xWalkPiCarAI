/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelOllamaTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarLanguageModelOllamaTest.cpp.
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

#ifndef XHAL_RPI5CARLANGUAGEMODELOLLAMATESTTYPES_H
#define XHAL_RPI5CARLANGUAGEMODELOLLAMATESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarLanguageModel.h"
#include "xHal_Rpi5CarLanguageModelSimulationConfig.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include <fstream>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carlanguagemodelollamatest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carlanguagemodelollamatest
{

    using namespace xwalk::hal;

    /** @brief Records one injected Ollama HTTP transport. */
    struct TestTransport
    {
            /** @brief Most recently requested endpoint. */
            XWalkHal::string endpoint{};
            /** @brief Most recently serialized JSON request. */
            XWalkHal::string request{};
            /** @brief Most recently supplied authorization header. */
            XWalkHal::string authorizationHeader{};
            /** @brief JSON response returned by the next request. */
            XWalkHal::string response{"{\"message\":{\"role\":\"assistant\",\"content\":"
                                      "\"ready\"},\"done\":true}"};
            /** @brief Most recently requested timeout in milliseconds. */
            XWalkHal::uint32 timeoutMs{};
            /** @brief Most recently supplied response bound in bytes. */
            XWalkHal::size maximumResponseBytes{};
            /** @brief Number of JSON POST operations. */
            XWalkHal::uint32 requestCount{};
            /** @brief Makes the next transport call report failure when true. */
            XWalkHal::boolean fail{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carlanguagemodelollamatest */

#endif /* XHAL_RPI5CARLANGUAGEMODELOLLAMATESTTYPES_H */
