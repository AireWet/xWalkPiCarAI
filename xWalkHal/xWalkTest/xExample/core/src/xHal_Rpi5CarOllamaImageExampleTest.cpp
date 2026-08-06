/******************************************************************************
 * @file        xHal_Rpi5CarOllamaImageExampleTest.cpp
 * @brief       Verifies Ollama camera chat without hardware or network access.
 *
 * @details
 * Checks conversation setup, capture ordering, image-path forwarding, output,
 * end-of-input handling, callback validation, and bounded prompt validation.
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

#include "xHal_Rpi5CarOllamaImageExample.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory camera, model, console, and host scenarios. */
namespace
{

/** @brief Records every operation from one Ollama image example run. */
struct OllamaImageExampleState
{
    /** @brief Configured system instructions. */
    XWalkHal::string instructions;
    /** @brief Configured assistant welcome text. */
    XWalkHal::string welcome;
    /** @brief Configured retained-message limit. */
    XWalkHal::uint32 maximumMessages{};
    /** @brief Input lines returned in order. */
    XWalkHal::stringvector inputs{"What do you see?", "And now?"};
    /** @brief Model responses returned in order. */
    XWalkHal::stringvector responses{"A test image", "Another test image"};
    /** @brief Prompt text received by the model. */
    XWalkHal::stringvector prompts;
    /** @brief Image paths received by the model. */
    XWalkHal::stringvector modelImagePaths;
    /** @brief Output paths requested from the camera. */
    XWalkHal::stringvector capturePaths;
    /** @brief Output fragments written by the example. */
    XWalkHal::stringvector outputs;
    /** @brief Newline flags corresponding to output fragments. */
    XWalkHal::uint32vector newlineFlags;
    /** @brief Flush flags corresponding to output fragments. */
    XWalkHal::uint32vector flushFlags;
    /** @brief Index of the next console input. */
    XWalkHal::size inputIndex{};
};

/** @brief Records one successful in-memory camera capture. */
XWalkHal::boolean captureImage(XWalkHal::contextpointer context,
    XWalkHal::stringview outputPath,
    const XWalkHal::XWalkCameraConfiguration& configuration)
{
    OllamaImageExampleState& state =
        *static_cast<OllamaImageExampleState*>(context);
    static_cast<void>(configuration);
    state.capturePaths.emplace_back(outputPath);
    return true;
}

/** @brief Records system instructions. */
void setInstructions(XWalkHal::contextpointer context,
    XWalkHal::stringview instructions)
{
    static_cast<OllamaImageExampleState*>(context)->instructions =
        XWalkHal::string(instructions);
}

/** @brief Records assistant welcome text. */
void setWelcome(XWalkHal::contextpointer context, XWalkHal::stringview welcome)
{
    static_cast<OllamaImageExampleState*>(context)->welcome = XWalkHal::string(welcome);
}

/** @brief Records the retained-message limit. */
void setMaximumMessages(
    XWalkHal::contextpointer context, XWalkHal::uint32 maximumMessages)
{
    static_cast<OllamaImageExampleState*>(context)->maximumMessages = maximumMessages;
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

/** @brief Records one image prompt and returns its configured response. */
XWalkHal::string prompt(XWalkHal::contextpointer context,
    XWalkHal::stringview promptText, XWalkHal::stringview imagePath)
{
    OllamaImageExampleState& state =
        *static_cast<OllamaImageExampleState*>(context);
    state.prompts.emplace_back(promptText);
    state.modelImagePaths.emplace_back(imagePath);
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
    OllamaImageExampleState& state =
        *static_cast<OllamaImageExampleState*>(context);
    const hal::boolean inputUnavailable =
        static_cast<hal::boolean>(
            state.inputIndex >= state.inputs.size());
    if (inputUnavailable)
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
    OllamaImageExampleState& state =
        *static_cast<OllamaImageExampleState*>(context);
    state.outputs.emplace_back(text);
    state.newlineFlags.push_back(appendNewline ? 1U : 0U);
    state.flushFlags.push_back(flushOutput ? 1U : 0U);
}

/** @brief Returns the complete in-memory console table. */
xwalk::hal::example::XWalkOllamaImageExampleCallbacks consoleCallbacks()
{
    return {&readPrompt, &write};
}

/** @brief Verifies configuration, capture order, image prompting, and output. */
void testConversation()
{
    OllamaImageExampleState state;
    XWalkHal::XWalkCamera camera(&state, &captureImage);
    XWalkHal::XWalkLanguageModel model(&state, modelCallbacks());
    xwalk::hal::example::XWalkOllamaImageExample example(
        camera, model, &state, consoleCallbacks());

    example.run(3U, "config/llm-img.jpg");

    assert(state.maximumMessages == 20U);
    assert(state.instructions == "You are a helpful assistant.");
    assert(state.welcome ==
        "Hello, I am a helpful assistant. How can I help you?");
    assert(state.prompts ==
        XWalkHal::stringvector({"What do you see?", "And now?"}));
    assert(state.capturePaths == XWalkHal::stringvector({
        "config/llm-img.jpg", "config/llm-img.jpg"}));
    assert(state.modelImagePaths == state.capturePaths);
    assert(state.outputs == XWalkHal::stringvector({
        "Hello, I am a helpful assistant. How can I help you?",
        "A test image", "", "Another test image", ""}));
    assert(state.newlineFlags ==
        XWalkHal::uint32vector({1U, 0U, 1U, 0U, 1U}));
    assert(state.flushFlags ==
        XWalkHal::uint32vector({0U, 1U, 0U, 1U, 0U}));
}

/** @brief Verifies constructor and bounded-count validation. */
void testValidation()
{
    OllamaImageExampleState state;
    XWalkHal::XWalkCamera camera(&state, &captureImage);
    XWalkHal::XWalkLanguageModel model(&state, modelCallbacks());
    xwalk::hal::example::XWalkOllamaImageExampleCallbacks incompleteCallbacks =
        consoleCallbacks();
    incompleteCallbacks.write = nullptr;
    XWalkHal::boolean rejectedCallbacks = false;
    try
    {
        xwalk::hal::example::XWalkOllamaImageExample invalidExample(
            camera, model, &state, incompleteCallbacks);
    }
    catch (const XWalkHal::invalidargument&)
    {
        rejectedCallbacks = true;
    }
    assert(rejectedCallbacks);

    xwalk::hal::example::XWalkOllamaImageExample example(
        camera, model, &state, consoleCallbacks());
    XWalkHal::boolean rejectedCount = false;
    try
    {
        example.run(0U, "config/llm-img.jpg");
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
 * @brief Runs the host-safe Ollama image example verification.
 * @return Zero after every assertion passes.
 */
int xWalkOllamaImageExampleHostTest()
{
    testConversation();
    testValidation();
    return 0;
}
