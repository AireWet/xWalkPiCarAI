/******************************************************************************
 * @file        xHal_Rpi5CarQwenExampleTest.cpp
 * @brief       Verifies the Qwen example without credentials or network.
 *
 * @details
 * Checks conversation setup, bounded prompt order, response formatting, early
 * input completion, callback validation, and prompt-count validation in memory.
 *
 * @project     xWalk Firmware
 * @module      xExample Host Test
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
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>
#include "xHal_Rpi5CarQwenExampleTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using QwenExampleState = ::xwalk::source_types::xhal_rpi5carqwenexampletest::QwenExampleState;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory model, console, and host scenarios. */
namespace
{

    /** @brief Records system instructions. */
    void setInstructions(XWalkHal::contextpointer context, XWalkHal::stringview instructions)
    {
        static_cast<QwenExampleState*>(context)->instructions = XWalkHal::string(instructions);
    }

    /** @brief Records assistant welcome text. */
    void setWelcome(XWalkHal::contextpointer context, XWalkHal::stringview welcome)
    {
        static_cast<QwenExampleState*>(context)->welcome = XWalkHal::string(welcome);
    }

    /** @brief Records the retained-message limit. */
    void setMaximumMessages(XWalkHal::contextpointer context, XWalkHal::uint32 maximumMessages)
    {
        static_cast<QwenExampleState*>(context)->maximumMessages = maximumMessages;
    }

    /** @brief Accepts an unused explicit history message. */
    void addMessage(XWalkHal::contextpointer context,
                    XWalkHal::XWalkLanguageModelRole role,
                    XWalkHal::stringview content,
                    XWalkHal::stringview imagePath)
    {
        static_cast<void>(context);
        static_cast<void>(role);
        static_cast<void>(content);
        static_cast<void>(imagePath);
    }

    /** @brief Records one prompt and returns its configured response. */
    XWalkHal::string
    prompt(XWalkHal::contextpointer context, XWalkHal::stringview promptText, XWalkHal::stringview imagePath)
    {
        QwenExampleState& state = *static_cast<QwenExampleState*>(context);
        state.prompts.emplace_back(promptText);
        state.imagePaths.emplace_back(imagePath);
        return state.responses[state.prompts.size() - 1U];
    }

    /** @brief Returns the complete in-memory language-model table. */
    XWalkHal::XWalkLanguageModelCallbacks modelCallbacks()
    {
        return {&setInstructions, &setWelcome, &setMaximumMessages, &addMessage, &prompt};
    }

    /** @brief Returns one configured input line and then reports end of input. */
    XWalkHal::boolean readPrompt(XWalkHal::contextpointer context, XWalkHal::string& inputText)
    {
        QwenExampleState& state = *static_cast<QwenExampleState*>(context);
        const hal::boolean inputUnavailable = static_cast<hal::boolean>(state.inputIndex >= state.inputs.size());
        if (inputUnavailable)
        {
            return false;
        }
        inputText = state.inputs[state.inputIndex];
        ++state.inputIndex;
        return true;
    }

    /** @brief Records one output fragment and its formatting flags. */
    void write(XWalkHal::contextpointer context,
               XWalkHal::stringview text,
               XWalkHal::boolean appendNewline,
               XWalkHal::boolean flushOutput)
    {
        QwenExampleState& state = *static_cast<QwenExampleState*>(context);
        state.outputs.emplace_back(text);
        state.newlineFlags.push_back(appendNewline ? 1U : 0U);
        state.flushFlags.push_back(flushOutput ? 1U : 0U);
    }

    /** @brief Returns the complete in-memory console table. */
    xwalk::hal::example::XWalkQwenExampleCallbacks consoleCallbacks()
    {
        return {&readPrompt, &write};
    }

    /** @brief Verifies configuration, text-only prompting, and output behavior. */
    void testConversation()
    {
        QwenExampleState state;
        XWalkHal::XWalkLanguageModel model(&state, modelCallbacks());
        xwalk::hal::example::XWalkQwenExample example(model, &state, consoleCallbacks());

        example.run(3U);

        assert(state.maximumMessages == 20U);
        assert(state.instructions == "You are a helpful assistant.");
        assert(state.welcome == "Hello, I am a helpful assistant. How can I help you?");
        assert(state.prompts == XWalkHal::stringvector({"Hello", "How are you?"}));
        assert(state.imagePaths == XWalkHal::stringvector({"", ""}));
        assert(state.outputs ==
               XWalkHal::stringvector(
                   {"Hello, I am a helpful assistant. How can I help you?", "Hello from Qwen", "", "I am ready", ""}));
        assert(state.newlineFlags == XWalkHal::uint32vector({1U, 0U, 1U, 0U, 1U}));
        assert(state.flushFlags == XWalkHal::uint32vector({0U, 1U, 0U, 1U, 0U}));
    }

    /** @brief Verifies constructor and bounded-count validation. */
    void testValidation()
    {
        QwenExampleState state;
        XWalkHal::XWalkLanguageModel model(&state, modelCallbacks());
        xwalk::hal::example::XWalkQwenExampleCallbacks incompleteCallbacks = consoleCallbacks();
        incompleteCallbacks.write = nullptr;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::example::XWalkQwenExample invalidExample(model, &state, incompleteCallbacks);
            });

        xwalk::hal::example::XWalkQwenExample example(model, &state, consoleCallbacks());
        xwalk::hal::test::expectFailure(
            [&]()
            {
                example.run(0U);
            });
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe Qwen text example verification.
 * @return Zero after every assertion passes.
 */
int xWalkQwenExampleHostTest()
{
    testConversation();
    testValidation();
    return 0;
}
