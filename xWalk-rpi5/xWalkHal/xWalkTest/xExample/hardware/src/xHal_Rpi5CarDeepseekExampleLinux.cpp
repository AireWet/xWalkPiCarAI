/******************************************************************************
 * @file        xHal_Rpi5CarDeepseekExampleLinux.cpp
 * @brief       Implements console and HTTP composition for the DeepSeek example.
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

#include "xHal_Rpi5CarDeepseekExampleLinux.h"

#include "xHal_Rpi5CarLanguageModelOllama.h"

#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::example
{

    /**
     * @brief Runs bounded interactive chat through the compatible HTTP provider.
     *
     * @param[in] apiKey Non-empty DeepSeek API credential.
     * @param[in] maximumPrompts Prompt limit from one through 100.
     * @warning Sends entered prompt text to the configured remote endpoint.
     */
    void XWalkDeepseekExampleLinux::run(stringview apiKey, uint32 maximumPrompts)
    {
        constexpr stringview endpoint{"https://api.deepseek.com/chat/completions"};
        constexpr stringview modelName{"deepseek-chat"};
        XWalkLanguageModelHttp backend(
            XWalkLanguageModelHttpDialect::OpenAiChatCompletions, endpoint, modelName, apiKey);
        XWalkLanguageModel languageModel(&backend, backend.callbacks());
        const XWalkDeepseekExampleCallbacks callbacks{&readPrompt, &write};
        XWalkDeepseekExample example(languageModel, this, callbacks);
        example.run(maximumPrompts);
    }

    /**
     * @brief Prints the source prompt and reads one terminal line.
     *
     * @param[in,out] context Unused callback context.
     * @param[out] inputText Entered line without its delimiter.
     * @return `true` after a line is read, or `false` at end of input.
     */
    boolean XWalkDeepseekExampleLinux::readPrompt(contextpointer context, string& inputText)
    {
        static_cast<void>(context);
        std::cout << ">>> " << std::flush;
        return static_cast<boolean>(std::getline(std::cin, inputText));
    }

    /**
     * @brief Writes one source-compatible output fragment.
     *
     * @param[in,out] context Unused callback context.
     * @param[in] text Text to write without modification.
     * @param[in] appendNewline Whether to append one newline.
     * @param[in] flushOutput Whether to flush after writing.
     */
    void XWalkDeepseekExampleLinux::write(contextpointer context,
                                          stringview text,
                                          boolean appendNewline,
                                          boolean flushOutput)
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
