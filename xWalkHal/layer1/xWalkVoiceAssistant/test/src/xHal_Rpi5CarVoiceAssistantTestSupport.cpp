/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantTestSupport.cpp
 * @brief       Implements reusable voice-assistant host-test callbacks.
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarVoiceAssistantTestSupport.h"
namespace xwalk::hal::test::voiceassistant
{
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue)
{
    static_cast<TestGpioBackend*>(context)->value = initialValue;
    static_cast<void>(pin); static_cast<void>(mode); static_cast<void>(pull);
}
boolean readGpio(contextpointer context, uint8 pin)
{
    static_cast<void>(pin); return static_cast<TestGpioBackend*>(context)->value;
}
void writeGpio(contextpointer context, uint8 pin, boolean value)
{
    static_cast<TestGpioBackend*>(context)->value = value; static_cast<void>(pin);
}
void registerInterrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler)
{
    static_cast<void>(context); static_cast<void>(pin); static_cast<void>(edge);
    static_cast<void>(debounceMs); static_cast<void>(handlerContext); static_cast<void>(handler);
}
void cancelInterrupt(contextpointer context, uint8 pin)
{
    static_cast<void>(context); static_cast<void>(pin);
}
XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
}
boolean probeI2c(contextpointer context, uint8 address)
{
    static_cast<void>(context); static_cast<void>(address); return true;
}
void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
{
    static_cast<void>(context); static_cast<void>(address); static_cast<void>(reg);
    static_cast<void>(data);
}
bytevector readI2c(contextpointer context, uint8 address, size length)
{
    ++static_cast<TestI2cBackend*>(context)->readCount;
    static_cast<void>(address); static_cast<void>(length); return {};
}
void primeSpeaker(contextpointer context, uint32 durationMs)
{
    ++static_cast<TestPrimeBackend*>(context)->callCount; static_cast<void>(durationMs);
}
boolean recognitionReady(contextpointer context)
{
    static_cast<void>(context); return true;
}
string recognizeSpeech(contextpointer context, uint32 timeoutMs)
{
    TestRecognitionBackend& backend = *static_cast<TestRecognitionBackend*>(context);
    ++backend.listenCount; static_cast<void>(timeoutMs); return backend.result;
}
string transcribeFile(contextpointer context, stringview filePath)
{
    static_cast<void>(filePath); return static_cast<TestRecognitionBackend*>(context)->result;
}
void stopRecognition(contextpointer context)
{
    ++static_cast<TestRecognitionBackend*>(context)->stopCount;
}
XWalkSpeechToTextCallbacks recognitionCallbacks()
{
    return {&recognitionReady, &recognizeSpeech, &transcribeFile, &stopRecognition};
}
void setInstructions(contextpointer context, stringview instructions)
{
    static_cast<TestModelBackend*>(context)->instructions = string(instructions);
}
void setWelcome(contextpointer context, stringview welcome)
{
    static_cast<void>(context); static_cast<void>(welcome);
}
void setMaximumMessages(contextpointer context, uint32 maximumMessages)
{
    static_cast<void>(context); static_cast<void>(maximumMessages);
}
void addMessage(contextpointer context, XWalkLanguageModelRole role,
    stringview content, stringview imagePath)
{
    static_cast<void>(context); static_cast<void>(role); static_cast<void>(content);
    static_cast<void>(imagePath);
}
string promptModel(contextpointer context, stringview promptText, stringview imagePath)
{
    TestModelBackend& backend = *static_cast<TestModelBackend*>(context);
    ++backend.promptCount; backend.promptText = string(promptText);
    backend.imagePath = string(imagePath); return backend.response;
}
XWalkLanguageModelCallbacks modelCallbacks()
{
    return {&setInstructions, &setWelcome, &setMaximumMessages, &addMessage, &promptModel};
}
void synthesizeText(contextpointer context, stringview text)
{
    TestOutputBackend& backend = *static_cast<TestOutputBackend*>(context);
    ++backend.callCount; backend.text = string(text);
}
void onStart(contextpointer context) { ++static_cast<TestHookBackend*>(context)->startCount; }
void onHeard(contextpointer context, stringview text)
{
    static_cast<TestHookBackend*>(context)->heard = string(text);
}
string parseResponse(contextpointer context, stringview response)
{
    static_cast<void>(context); return string("parsed: ").append(response);
}
void afterSay(contextpointer context, stringview text)
{
    static_cast<TestHookBackend*>(context)->spoken = string(text);
}
void onRoundComplete(contextpointer context)
{
    ++static_cast<TestHookBackend*>(context)->roundCount;
}
void onStop(contextpointer context) { ++static_cast<TestHookBackend*>(context)->stopCount; }
} /* namespace xwalk::hal::test::voiceassistant */
