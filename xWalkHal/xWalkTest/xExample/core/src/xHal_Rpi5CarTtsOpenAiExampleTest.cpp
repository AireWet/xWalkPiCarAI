/******************************************************************************
 * @file        xHal_Rpi5CarTtsOpenAiExampleTest.cpp
 * @brief       Verifies OpenAI TTS flow without network or audio access.
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

#include "xHal_Rpi5CarTtsOpenAiExample.h"

#include <cassert>

namespace
{

/** @brief Records one request for exact-value verification. */
struct TtsOpenAiRequest
{
    XWalkHal::string model;
    XWalkHal::string voice;
    XWalkHal::string text;
    XWalkHal::string instructions;
};

/** @brief Records all deterministic requests and report messages. */
struct TtsOpenAiExampleState
{
    std::vector<TtsOpenAiRequest> requests;
    XWalkHal::stringvector reports;
};

/** @brief Records one injected OpenAI TTS request. */
void speak(XWalkHal::contextpointer context, XWalkHal::stringview model,
    XWalkHal::stringview voice, XWalkHal::stringview text,
    XWalkHal::stringview instructions)
{
    TtsOpenAiExampleState& state =
        *static_cast<TtsOpenAiExampleState*>(context);
    state.requests.push_back({XWalkHal::string(model), XWalkHal::string(voice),
        XWalkHal::string(text), XWalkHal::string(instructions)});
}

/** @brief Records one injected console report. */
void report(XWalkHal::contextpointer context, XWalkHal::stringview message)
{
    TtsOpenAiExampleState& state =
        *static_cast<TtsOpenAiExampleState*>(context);
    state.reports.emplace_back(message);
}

/** @brief Verifies exact request order, values, and source console text. */
void testRequests()
{
    TtsOpenAiExampleState state;
    xwalk::hal::example::XWalkTtsOpenAiExample example(
        &state, &speak, &report);

    example.run();

    assert(state.requests.size() == 3U);
    assert(state.reports.size() == 3U);
    assert(state.requests[0U].model == "gpt-4o-mini-tts");
    assert(state.requests[0U].voice == "alloy");
    assert(state.requests[0U].text == "Hello! I'm OpenAI TTS.");
    assert(state.requests[0U].instructions.empty());
    assert(state.requests[1U].text ==
        "with instructions, I can say word sadly");
    assert(state.requests[1U].instructions == "say it sadly");
    assert(state.requests[2U].text == "or say something dramaticly.");
    assert(state.requests[2U].instructions == "say it dramaticly");
    assert(state.reports[0U] == "Say: Hello! I'm OpenAI TTS.");
    assert(state.reports[1U] ==
        "Say: with instructions, I can say word sadly, with instructions: "
        "'say it sadly'");
    assert(state.reports[2U] ==
        "Say: or say something dramaticly., with instructions: "
        "'say it dramaticly'");
}

/** @brief Verifies rejection of either missing operation. */
void testValidation()
{
    XWalkHal::boolean rejectedSpeak = false;
    XWalkHal::boolean rejectedReport = false;
    try
    {
        xwalk::hal::example::XWalkTtsOpenAiExample invalid(
            nullptr, nullptr, &report);
    }
    catch (const XWalkHal::invalidargument&)
    {
        rejectedSpeak = true;
    }
    try
    {
        xwalk::hal::example::XWalkTtsOpenAiExample invalid(
            nullptr, &speak, nullptr);
    }
    catch (const XWalkHal::invalidargument&)
    {
        rejectedReport = true;
    }
    assert(rejectedSpeak);
    assert(rejectedReport);
}

} /* namespace */

/** @brief Runs the host-safe OpenAI TTS example verification. */
int xWalkTtsOpenAiExampleHostTest()
{
    testRequests();
    testValidation();
    return 0;
}
