/******************************************************************************
 * @file        xHal_Rpi5CarGrokExampleTest.cpp
 * @brief       Verifies the Grok example without credentials or network.
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

#include "xHal_Rpi5CarGrokExample.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>
#include "xHal_Rpi5CarGrokExampleTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using GrokExampleState = ::xwalk::source_types::xhal_rpi5cargrokexampletest::GrokExampleState;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory model, console, and host scenarios. */
namespace
{

    /** @brief Records system instructions. */
    void setInstructions(XWalkHal::contextpointer context, XWalkHal::stringview instructions)
    {
        static_cast<GrokExampleState*>(context)->instructions = XWalkHal::string(instructions);
    }

    /** @brief Records assistant welcome text. */
    void setWelcome(XWalkHal::contextpointer context, XWalkHal::stringview welcome)
    {
        static_cast<GrokExampleState*>(context)->welcome = XWalkHal::string(welcome);
    }

    /** @brief Records the retained-message limit. */
    void setMaximumMessages(XWalkHal::contextpointer context, XWalkHal::uint32 maximumMessages)
    {
        static_cast<GrokExampleState*>(context)->maximumMessages = maximumMessages;
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
        GrokExampleState& state = *static_cast<GrokExampleState*>(context);
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
        GrokExampleState& state = *static_cast<GrokExampleState*>(context);
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
        GrokExampleState& state = *static_cast<GrokExampleState*>(context);
        state.outputs.emplace_back(text);
        state.newlineFlags.push_back(appendNewline ? 1U : 0U);
        state.flushFlags.push_back(flushOutput ? 1U : 0U);
    }

    /** @brief Returns the complete in-memory console table. */
    xwalk::hal::example::XWalkGrokExampleCallbacks consoleCallbacks()
    {
        return {&readPrompt, &write};
    }

    /** @brief Verifies configuration, text-only prompting, and output behavior. */
    void testConversation()
    {
        GrokExampleState state;
        XWalkHal::XWalkLanguageModel model(&state, modelCallbacks());
        xwalk::hal::example::XWalkGrokExample example(model, &state, consoleCallbacks());

        example.run(3U);

        assert(state.maximumMessages == 20U);
        assert(state.instructions == "You are a helpful assistant.");
        assert(state.welcome == "Hello, I am a helpful assistant. How can I help you?");
        assert(state.prompts == XWalkHal::stringvector({"Hello", "How are you?"}));
        assert(state.imagePaths == XWalkHal::stringvector({"", ""}));
        assert(state.outputs ==
               XWalkHal::stringvector(
                   {"Hello, I am a helpful assistant. How can I help you?", "Hello from Grok", "", "I am ready", ""}));
        assert(state.newlineFlags == XWalkHal::uint32vector({1U, 0U, 1U, 0U, 1U}));
        assert(state.flushFlags == XWalkHal::uint32vector({0U, 1U, 0U, 1U, 0U}));
    }

    /** @brief Verifies constructor and bounded-count validation. */
    void testValidation()
    {
        GrokExampleState state;
        XWalkHal::XWalkLanguageModel model(&state, modelCallbacks());
        xwalk::hal::example::XWalkGrokExampleCallbacks incompleteCallbacks = consoleCallbacks();
        incompleteCallbacks.write = nullptr;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::example::XWalkGrokExample invalidExample(model, &state, incompleteCallbacks);
            });

        xwalk::hal::example::XWalkGrokExample example(model, &state, consoleCallbacks());
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
 * @brief Runs the host-safe Grok text example verification.
 * @return Zero after every assertion passes.
 */
int xWalkGrokExampleHostTest()
{
    testConversation();
    testValidation();
    return 0;
}
