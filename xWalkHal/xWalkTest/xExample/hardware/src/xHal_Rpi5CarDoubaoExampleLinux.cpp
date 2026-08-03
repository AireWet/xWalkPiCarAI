/******************************************************************************
 * @file        xHal_Rpi5CarDoubaoExampleLinux.cpp
 * @brief       Implements console and HTTPS composition for the Doubao example.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarDoubaoExampleLinux.h"

#include "xHal_Rpi5CarLanguageModelOllama.h"

#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains contracts and adapters for ported example programs.
 */
namespace xwalk::hal::example
{

/**
 * @brief Runs bounded interactive chat through the compatible HTTP provider.
 * @param[in] apiKey Non-empty Doubao API credential.
 * @param[in] maximumPrompts Prompt limit from one through 100.
 * @warning Sends entered prompt text to the configured remote endpoint.
 */
void XWalkDoubaoExampleLinux::run(stringview apiKey, uint32 maximumPrompts)
{
    constexpr stringview endpoint{
        "https://ark.cn-beijing.volces.com/api/v3/chat/completions"};
    constexpr stringview modelName{"doubao-seed-1-6-250615"};
    XWalkLanguageModelHttp backend(
        XWalkLanguageModelHttpDialect::OpenAiChatCompletions,
        endpoint, modelName, apiKey);
    XWalkLanguageModel languageModel(&backend, backend.callbacks());
    const XWalkDoubaoExampleCallbacks callbacks{&readPrompt, &write};
    XWalkDoubaoExample example(languageModel, this, callbacks);
    example.run(maximumPrompts);
}

/**
 * @brief Prints the source prompt and reads one terminal line.
 * @param[in,out] context Unused callback context.
 * @param[out] inputText Entered line without its delimiter.
 * @return `true` after a line is read, or `false` at end of input.
 */
boolean XWalkDoubaoExampleLinux::readPrompt(
    contextpointer context, string& inputText)
{
    static_cast<void>(context);
    std::cout << ">>> " << std::flush;
    return static_cast<boolean>(std::getline(std::cin, inputText));
}

/**
 * @brief Writes one source-compatible output fragment.
 * @param[in,out] context Unused callback context.
 * @param[in] text Text to write without modification.
 * @param[in] appendNewline Whether to append one newline.
 * @param[in] flushOutput Whether to flush after writing.
 */
void XWalkDoubaoExampleLinux::write(contextpointer context, stringview text,
    boolean appendNewline, boolean flushOutput)
{
    static_cast<void>(context);
    std::cout << text;
    if (appendNewline)
    {
        std::cout << '\n';
    }
    if (flushOutput)
    {
        std::cout << std::flush;
    }
}

} /* namespace xwalk::hal::example */
