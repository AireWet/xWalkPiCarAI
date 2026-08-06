/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantExampleTest.cpp
 * @brief       Verifies voice-assistant flow without devices or services.
 *
 * @project     xWalk Firmware
 * @module      xExample Host Test
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarVoiceAssistantExample.h"

#include <cassert>

namespace
{

/** @brief Records deterministic assistant operations and supplied values. */
struct VoiceAssistantExampleState
{
    xwalk::hal::example::XWalkVoiceAssistantExampleConfiguration configuration;
    XWalkHal::stringvector keyboardInputs;
    XWalkHal::stringvector microphoneInputs;
    XWalkHal::stringvector spokenTexts;
    XWalkHal::stringvector spokenModels;
    XWalkHal::stringvector reports;
    XWalkHal::string promptModel;
    XWalkHal::string promptText;
    XWalkHal::string promptImage;
    XWalkHal::size keyboardIndex{};
    XWalkHal::size microphoneIndex{};
    XWalkHal::uint32 configureCount{};
    XWalkHal::uint32 captureCount{};
    XWalkHal::uint32 promptCount{};
};

/** @brief Records the exact active source configuration. */
void configure(XWalkHal::contextpointer context,
    const xwalk::hal::example::XWalkVoiceAssistantExampleConfiguration& config)
{
    VoiceAssistantExampleState& state =
        *static_cast<VoiceAssistantExampleState*>(context);
    state.configuration = config;
    ++state.configureCount;
}

/** @brief Supplies one keyboard prompt or end of input. */
XWalkHal::boolean readKeyboard(
    XWalkHal::contextpointer context, XWalkHal::string& inputText)
{
    VoiceAssistantExampleState& state =
        *static_cast<VoiceAssistantExampleState*>(context);
    const hal::boolean keyboardInputUnavailable =
        static_cast<hal::boolean>(
            state.keyboardIndex >= state.keyboardInputs.size());
    if (keyboardInputUnavailable)
    {
        return false;
    }
    inputText = state.keyboardInputs[state.keyboardIndex];
    ++state.keyboardIndex;
    return true;
}

/** @brief Supplies one recognized microphone phrase. */
XWalkHal::string listen(XWalkHal::contextpointer context,
    XWalkHal::uint32 timeoutMs, XWalkHal::stringview language)
{
    VoiceAssistantExampleState& state =
        *static_cast<VoiceAssistantExampleState*>(context);
    assert(timeoutMs == 1000U);
    assert(language == "en-us");
    const XWalkHal::string text = state.microphoneInputs[state.microphoneIndex];
    ++state.microphoneIndex;
    return text;
}

/** @brief Records one successful image capture. */
XWalkHal::boolean capture(
    XWalkHal::contextpointer context, XWalkHal::stringview imagePath)
{
    VoiceAssistantExampleState& state =
        *static_cast<VoiceAssistantExampleState*>(context);
    assert(imagePath == "/tmp/voice-assistant.jpg");
    ++state.captureCount;
    return true;
}

/** @brief Records one multimodal model prompt and returns a response. */
XWalkHal::string prompt(XWalkHal::contextpointer context,
    XWalkHal::stringview model, XWalkHal::stringview text,
    XWalkHal::stringview imagePath)
{
    VoiceAssistantExampleState& state =
        *static_cast<VoiceAssistantExampleState*>(context);
    state.promptModel = model;
    state.promptText = text;
    state.promptImage = imagePath;
    ++state.promptCount;
    return "I can help.";
}

/** @brief Records one Piper model and spoken text. */
void speak(XWalkHal::contextpointer context, XWalkHal::stringview model,
    XWalkHal::stringview text)
{
    VoiceAssistantExampleState& state =
        *static_cast<VoiceAssistantExampleState*>(context);
    state.spokenModels.emplace_back(model);
    state.spokenTexts.emplace_back(text);
}

/** @brief Records one source-visible welcome or response. */
void report(XWalkHal::contextpointer context, XWalkHal::stringview text)
{
    static_cast<VoiceAssistantExampleState*>(context)->reports.emplace_back(text);
}

/** @brief Returns the complete in-memory operation table. */
xwalk::hal::example::XWalkVoiceAssistantExampleCallbacks callbacks()
{
    return {&configure, &readKeyboard, &listen, &capture,
        &prompt, &speak, &report};
}

/** @brief Verifies exact configuration and one keyboard-driven image round. */
void testKeyboardRound()
{
    VoiceAssistantExampleState state;
    state.keyboardInputs = {"what do you see?"};
    xwalk::hal::example::XWalkVoiceAssistantExample example(
        &state, callbacks());

    example.run(
        xwalk::hal::example::XWalkVoiceAssistantExampleInputMode::Keyboard,
        2U, 1000U, "/tmp/voice-assistant.jpg");

    assert(state.configureCount == 1U);
    assert(state.configuration.name == "Buddy");
    assert(state.configuration.ttsModel == "en_US-ryan-low");
    assert(state.configuration.languageModel == "gpt-4o-mini");
    assert(state.configuration.speechLanguage == "en-us");
    assert(state.configuration.wakeWord == "hey buddy");
    assert(state.configuration.answerOnWake == "Hi there");
    assert(state.configuration.welcome ==
        "Hi, I'm Buddy. Wake me up with: hey buddy");
    assert(state.configuration.instructions ==
        "\nYou are a helpful assistant, named Buddy.\n");
    assert(state.configuration.withImage);
    assert(state.configuration.keyboardEnabled);
    assert(state.configuration.wakeEnabled);
    assert(state.captureCount == 1U);
    assert(state.promptCount == 1U);
    assert(state.promptModel == "gpt-4o-mini");
    assert(state.promptText == "what do you see?");
    assert(state.promptImage == "/tmp/voice-assistant.jpg");
    assert(state.spokenTexts == XWalkHal::stringvector({
        "Hi, I'm Buddy. Wake me up with: hey buddy", "I can help."}));
}

/** @brief Verifies wake rejection, acknowledgement, and recognized prompting. */
void testWakeRound()
{
    VoiceAssistantExampleState state;
    state.microphoneInputs = {"background", "hey buddy", "where are we?"};
    xwalk::hal::example::XWalkVoiceAssistantExample example(
        &state, callbacks());

    example.run(
        xwalk::hal::example::XWalkVoiceAssistantExampleInputMode::WakeWord,
        2U, 1000U, "/tmp/voice-assistant.jpg");

    assert(state.microphoneIndex == 3U);
    assert(state.captureCount == 1U);
    assert(state.promptText == "where are we?");
    assert(state.spokenTexts == XWalkHal::stringvector({
        "Hi, I'm Buddy. Wake me up with: hey buddy", "Hi there",
        "I can help."}));
    assert(state.spokenModels == XWalkHal::stringvector({
        "en_US-ryan-low", "en_US-ryan-low", "en_US-ryan-low"}));
}

/** @brief Verifies callback, bound, and image-path validation. */
void testValidation()
{
    VoiceAssistantExampleState state;
    auto incomplete = callbacks();
    incomplete.capture = nullptr;
    XWalkHal::boolean rejectedCallbacks = false;
    try
    {
        xwalk::hal::example::XWalkVoiceAssistantExample invalid(
            &state, incomplete);
    }
    catch (const XWalkHal::invalidargument&)
    {
        rejectedCallbacks = true;
    }
    assert(rejectedCallbacks);

    xwalk::hal::example::XWalkVoiceAssistantExample example(
        &state, callbacks());
    XWalkHal::boolean rejectedRun = false;
    try
    {
        example.run(
            xwalk::hal::example::XWalkVoiceAssistantExampleInputMode::Keyboard,
            0U, 1000U, "/tmp/voice-assistant.jpg");
    }
    catch (const XWalkHal::outofrange&)
    {
        rejectedRun = true;
    }
    assert(rejectedRun);
}

} /* namespace */

/** @brief Runs every host-safe voice-assistant example verification. */
int xWalkVoiceAssistantExampleHostTest()
{
    testKeyboardRound();
    testWakeRound();
    testValidation();
    return 0;
}
