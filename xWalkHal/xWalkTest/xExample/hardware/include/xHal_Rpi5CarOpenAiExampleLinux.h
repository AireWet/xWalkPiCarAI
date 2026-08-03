/******************************************************************************
 * @file        xHal_Rpi5CarOpenAiExampleLinux.h
 * @brief       Declares console and HTTPS composition for the OpenAI example.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_OPEN_AI_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_OPEN_AI_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarOpenAiExample.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains contracts and adapters for ported example programs.
 */
namespace xwalk::hal::example
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/** @brief Composes the OpenAI example with libcurl and terminal input/output. */
class XWalkOpenAiExampleLinux final
{
protected:

    /** @brief Prints `>>> ` and reads one terminal input line. */
    static boolean readPrompt(contextpointer context, string& inputText);
    /** @brief Writes one welcome or response fragment to standard output. */
    static void write(contextpointer context, stringview text,
        boolean appendNewline, boolean flushOutput);

public:

    /**
     * @brief Runs bounded interactive chat through the OpenAI API.
     * @param[in] apiKey Non-empty credential sourced outside process arguments.
     * @param[in] maximumPrompts Prompt limit from one through 100.
     * @warning Sends user prompts to a remote service over HTTPS.
     */
    void run(stringview apiKey, uint32 maximumPrompts);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_OPEN_AI_EXAMPLE_LINUX_H */
