/******************************************************************************
 * @file        xAgent_Rpi5CarLocalVoiceChatbotTest.cpp
 * @brief       Verifies the device-free local voice-chatbot coordinator.
 *
 * @project     xWalk Firmware
 * @module      xWalkLocalVoiceChatbot Host Test
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xAgent_Rpi5CarLocalVoiceChatbot.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarI2c.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

using namespace xwalk::hal;

struct TestState
{
    stringvector outputLines{};
    stringvector spokenText{};
    uint32 listenCount{};
    uint32 continueCount{};
    uint32 delayCount{};
};

void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
    static_cast<void>(initialValue);
}

boolean readGpio(contextpointer context, uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    return true;
}

void writeGpio(contextpointer context, uint8 pin, boolean value)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(value);
}

void registerInterrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(edge);
    static_cast<void>(debounceMs);
    static_cast<void>(handlerContext);
    static_cast<void>(handler);
}

void cancelInterrupt(contextpointer context, uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

boolean probeI2c(contextpointer context, uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

void writeI2c(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
}

bytevector readI2c(contextpointer context, uint8 address, size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return bytevector(length, 0U);
}

void primeSpeaker(contextpointer context, uint32 durationMs)
{
    static_cast<void>(context);
    static_cast<void>(durationMs);
}

boolean recognitionReady(contextpointer context)
{
    static_cast<void>(context);
    return true;
}

string recognizeSpeech(contextpointer context, uint32 timeoutMs)
{
    TestState& state = *static_cast<TestState*>(context);
    ++state.listenCount;
    assert(timeoutMs == 10'000U);
    return (state.listenCount == 1U) ? string("hello") : string{};
}

string transcribeFile(contextpointer context, stringview filePath)
{
    static_cast<void>(context);
    static_cast<void>(filePath);
    return {};
}

void stopRecognition(contextpointer context)
{
    static_cast<void>(context);
}

void setModelText(contextpointer context, stringview text)
{
    static_cast<void>(context);
    static_cast<void>(text);
}

void setMaximumMessages(contextpointer context, uint32 maximumMessages)
{
    static_cast<void>(context);
    assert(maximumMessages > 0U);
}

void addMessage(contextpointer context, XWalkLanguageModelRole role,
    stringview content, stringview imagePath)
{
    static_cast<void>(context);
    static_cast<void>(role);
    static_cast<void>(content);
    static_cast<void>(imagePath);
}

string promptModel(contextpointer context, stringview promptText,
    stringview imagePath)
{
    static_cast<void>(context);
    assert(promptText == "hello");
    assert(imagePath.empty());
    return "<think>hidden</think> Answer.";
}

void speak(contextpointer context, stringview text)
{
    static_cast<TestState*>(context)->spokenText.emplace_back(text);
}

void output(contextpointer context, stringview line)
{
    static_cast<TestState*>(context)->outputLines.emplace_back(line);
}

boolean shouldContinue(contextpointer context)
{
    TestState& state = *static_cast<TestState*>(context);
    ++state.continueCount;
    return state.continueCount <= 2U;
}

void delay(contextpointer context, uint32 durationMs)
{
    TestState& state = *static_cast<TestState*>(context);
    ++state.delayCount;
    assert((durationMs == 50U) || (durationMs == 100U));
}

void testChatbotLoop()
{
    TestState state;
    const XWalkGpioCallbacks gpioCallbacks{&configureGpio, &readGpio, &writeGpio,
        &registerInterrupt, &cancelInterrupt};
    XWalkGpio resetGpio(nullptr, gpioCallbacks, "MCURST");
    XWalkGpio speakerGpio(nullptr, gpioCallbacks,
        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
    XWalkI2c i2c(nullptr, &probeI2c, &writeI2c, &readI2c);
    XWalkAdc batteryAdc(i2c, "A4");
    XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc,
        nullptr, &primeSpeaker);
    const XWalkSpeechToTextCallbacks recognitionCallbacks{&recognitionReady,
        &recognizeSpeech, &transcribeFile, &stopRecognition};
    XWalkSpeechToText speechToText(&state, recognitionCallbacks);
    const XWalkLanguageModelCallbacks modelCallbacks{&setModelText, &setModelText,
        &setMaximumMessages, &addMessage, &promptModel};
    XWalkLanguageModel languageModel(nullptr, modelCallbacks);
    XWalkTextToSpeech textToSpeech(boardControl, &state, &speak);
    const XWalkVoiceAssistantConfiguration assistantConfiguration{
        xwalk::agent::XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_INSTRUCTIONS,
        xwalk::agent::XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME};
    XWalkVoiceAssistant assistant(speechToText, languageModel, textToSpeech,
        assistantConfiguration);
    const xwalk::agent::XWalkLocalVoiceChatbotCallbacks callbacks{
        &output, &shouldContinue, &delay};
    xwalk::agent::XWalkLocalVoiceChatbot chatbot(assistant, &state, callbacks);

    assert(chatbot.run() == 0);
    assert(state.listenCount == 2U);
    assert(state.delayCount == 2U);
    assert(state.spokenText.size() == 3U);
    assert(state.spokenText[0U] ==
        xwalk::agent::XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME);
    assert(state.spokenText[1U] == "Answer.");
    assert(state.spokenText[2U] == "Goodbye!");
    assert(state.outputLines.back() == "Bye.");
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

int main()
{
    assert(xwalk::agent::XWalkLocalVoiceChatbot::stripThinking(
        "[thinking]x[/thinking] Clean") == "x Clean");
    assert(xwalk::agent::XWalkLocalVoiceChatbot::stripThinking(
        "before ```thinking secret``` after") == "before  after");
    testChatbotLoop();
    return 0;
}
