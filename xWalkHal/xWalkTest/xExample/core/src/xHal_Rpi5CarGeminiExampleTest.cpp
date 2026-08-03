/******************************************************************************
 * @file        xHal_Rpi5CarGeminiExampleTest.cpp
 * @brief       Verifies the Gemini example without credentials or network.
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

#include "xHal_Rpi5CarGeminiExample.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory model, console, and host scenarios. */
namespace
{

/** @brief Records model and console operations from one example run. */
struct GeminiExampleState
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
    XWalkHal::stringvector responses{"Hello from Gemini", "I am ready"};
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

/** @brief Records system instructions. */
void setInstructions(XWalkHal::contextpointer context,
    XWalkHal::stringview instructions)
{
    static_cast<GeminiExampleState*>(context)->instructions =
        XWalkHal::string(instructions);
}

/** @brief Records assistant welcome text. */
void setWelcome(XWalkHal::contextpointer context, XWalkHal::stringview welcome)
{
    static_cast<GeminiExampleState*>(context)->welcome = XWalkHal::string(welcome);
}

/** @brief Records the retained-message limit. */
void setMaximumMessages(
    XWalkHal::contextpointer context, XWalkHal::uint32 maximumMessages)
{
    static_cast<GeminiExampleState*>(context)->maximumMessages = maximumMessages;
}

/** @brief Accepts an unused explicit history message. */
void addMessage(XWalkHal::contextpointer context,
    XWalkHal::XWalkLanguageModelRole role, XWalkHal::stringview content,
    XWalkHal::stringview imagePath)
{
    static_cast<void>(context);
    static_cast<void>(role);
    static_cast<void>(content);
    static_cast<void>(imagePath);
}

/** @brief Records one prompt and returns its configured response. */
XWalkHal::string prompt(XWalkHal::contextpointer context,
    XWalkHal::stringview promptText, XWalkHal::stringview imagePath)
{
    GeminiExampleState& state = *static_cast<GeminiExampleState*>(context);
    state.prompts.emplace_back(promptText);
    state.imagePaths.emplace_back(imagePath);
    return state.responses[state.prompts.size() - 1U];
}

/** @brief Returns the complete in-memory language-model table. */
XWalkHal::XWalkLanguageModelCallbacks modelCallbacks()
{
    return {&setInstructions, &setWelcome, &setMaximumMessages,
        &addMessage, &prompt};
}

/** @brief Returns one configured input line and then reports end of input. */
XWalkHal::boolean readPrompt(
    XWalkHal::contextpointer context, XWalkHal::string& inputText)
{
    GeminiExampleState& state = *static_cast<GeminiExampleState*>(context);
    if (state.inputIndex >= state.inputs.size())
    {
        return false;
    }
    inputText = state.inputs[state.inputIndex];
    ++state.inputIndex;
    return true;
}

/** @brief Records one output fragment and its formatting flags. */
void write(XWalkHal::contextpointer context, XWalkHal::stringview text,
    XWalkHal::boolean appendNewline, XWalkHal::boolean flushOutput)
{
    GeminiExampleState& state = *static_cast<GeminiExampleState*>(context);
    state.outputs.emplace_back(text);
    state.newlineFlags.push_back(appendNewline ? 1U : 0U);
    state.flushFlags.push_back(flushOutput ? 1U : 0U);
}

/** @brief Returns the complete in-memory console table. */
xwalk::hal::example::XWalkGeminiExampleCallbacks consoleCallbacks()
{
    return {&readPrompt, &write};
}

/** @brief Verifies configuration, text-only prompting, and output behavior. */
void testConversation()
{
    GeminiExampleState state;
    XWalkHal::XWalkLanguageModel model(&state, modelCallbacks());
    xwalk::hal::example::XWalkGeminiExample example(
        model, &state, consoleCallbacks());

    example.run(3U);

    assert(state.maximumMessages == 20U);
    assert(state.instructions == "You are a helpful assistant.");
    assert(state.welcome ==
        "Hello, I am a helpful assistant. How can I help you?");
    assert(state.prompts ==
        XWalkHal::stringvector({"Hello", "How are you?"}));
    assert(state.imagePaths == XWalkHal::stringvector({"", ""}));
    assert(state.outputs == XWalkHal::stringvector({
        "Hello, I am a helpful assistant. How can I help you?",
        "Hello from Gemini", "", "I am ready", ""}));
    assert(state.newlineFlags ==
        XWalkHal::uint32vector({1U, 0U, 1U, 0U, 1U}));
    assert(state.flushFlags ==
        XWalkHal::uint32vector({0U, 1U, 0U, 1U, 0U}));
}

/** @brief Verifies constructor and bounded-count validation. */
void testValidation()
{
    GeminiExampleState state;
    XWalkHal::XWalkLanguageModel model(&state, modelCallbacks());
    xwalk::hal::example::XWalkGeminiExampleCallbacks incompleteCallbacks =
        consoleCallbacks();
    incompleteCallbacks.write = nullptr;
    XWalkHal::boolean rejectedCallbacks = false;
    try
    {
        xwalk::hal::example::XWalkGeminiExample invalidExample(
            model, &state, incompleteCallbacks);
    }
    catch (const XWalkHal::invalidargument&)
    {
        rejectedCallbacks = true;
    }
    assert(rejectedCallbacks);

    xwalk::hal::example::XWalkGeminiExample example(
        model, &state, consoleCallbacks());
    XWalkHal::boolean rejectedCount = false;
    try
    {
        example.run(0U);
    }
    catch (const XWalkHal::outofrange&)
    {
        rejectedCount = true;
    }
    assert(rejectedCount);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe Gemini text example verification.
 * @return Zero after every assertion passes.
 */
int xWalkGeminiExampleHostTest()
{
    testConversation();
    testValidation();
    return 0;
}
