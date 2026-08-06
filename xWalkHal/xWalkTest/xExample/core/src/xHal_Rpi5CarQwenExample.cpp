/******************************************************************************
 * @file        xHal_Rpi5CarQwenExample.cpp
 * @brief       Implements the bounded interactive Qwen example flow.
 *
 * @project     xWalk Firmware
 * @module      xExample
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

#include "xHal_Rpi5CarQwenExample.h"

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
 * @brief Binds one language model and validates the console table.
 * @param[in,out] languageModel Caller-owned model.
 * @param[in,out] context Non-owning console context.
 * @param[in] consoleCallbacks Complete console table.
 * @throws std::invalid_argument If either callback is null.
 */
XWalkQwenExample::XWalkQwenExample(XWalkLanguageModel& languageModel,
    contextpointer context, const XWalkQwenExampleCallbacks& consoleCallbacks):
    languageModelObject(&languageModel), consoleContext(context),
    callbacks(consoleCallbacks)
{
    if ((callbacks.readPrompt == nullptr) || (callbacks.write == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Qwen example requires complete console callbacks");
    }
}

/**
 * @brief Runs the configured welcome and bounded prompt loop.
 * @param[in] maximumPrompts Prompt limit from one through 100.
 * @throws std::out_of_range If `maximumPrompts` is outside its range.
 * @warning Prompt operations may contact a remote model provider.
 */
void XWalkQwenExample::run(uint32 maximumPrompts)
{
    if ((maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_QWEN_EXAMPLE_MAXIMUM_PROMPTS))
    {
        XHAL_THROW_OUT_OF_RANGE(
            "Qwen example prompt count is outside its range");
    }

    constexpr stringview instructions{"You are a helpful assistant."};
    constexpr stringview welcome{
        "Hello, I am a helpful assistant. How can I help you?"};
    languageModelObject->setMaximumMessages(20U);
    languageModelObject->setInstructions(instructions);
    languageModelObject->setWelcome(welcome);
    callbacks.write(consoleContext, welcome, true, false);

    for (uint32 promptIndex = 0U; promptIndex < maximumPrompts; ++promptIndex)
    {
        string inputText;
        const hal::boolean promptRead = callbacks.readPrompt(consoleContext, inputText);
        if (promptRead == false)
        {
            break;
        }
        const string response = languageModelObject->prompt(inputText);
        const hal::boolean responseAvailable =
            static_cast<hal::boolean>(
                !response.empty());
        if (responseAvailable)
        {
            callbacks.write(consoleContext, response, false, true);
        }
        callbacks.write(consoleContext, {}, true, false);
    }
}

} /* namespace xwalk::hal::example */
