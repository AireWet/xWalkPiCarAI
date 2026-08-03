/******************************************************************************
 * @file        xHal_Rpi5CarOllamaExampleLinux.h
 * @brief       Declares console and native HTTP composition for Ollama chat.
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

#ifndef XHAL_RPI5CAR_OLLAMA_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_OLLAMA_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarOllamaExample.h"

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

/** @brief Composes Ollama chat with the native provider and terminal I/O. */
class XWalkOllamaExampleLinux final
{
protected:

    /** @brief Prints `>>> ` and reads one terminal input line. */
    static boolean readPrompt(contextpointer context, string& inputText);
    /** @brief Writes one welcome or response fragment to standard output. */
    static void write(contextpointer context, stringview text,
        boolean appendNewline, boolean flushOutput);

public:

    /**
     * @brief Runs bounded interactive chat through the configured Ollama API.
     * @param[in] maximumPrompts Prompt limit from one through 100.
     * @warning Sends user prompts to the configured Ollama service.
     */
    void run(uint32 maximumPrompts);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_OLLAMA_EXAMPLE_LINUX_H */
