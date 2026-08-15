/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantTestSupport.h
 * @brief       Declares reusable voice-assistant host-test callbacks.
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_VOICE_ASSISTANT_TEST_SUPPORT_H
#define XHAL_RPI5CAR_VOICE_ASSISTANT_TEST_SUPPORT_H
#include "xHal_Rpi5CarVoiceAssistant.h"
namespace xwalk::hal::test::voiceassistant
{
    struct TestGpioBackend
    {
            boolean value{};
    };
    struct TestI2cBackend
    {
            uint32 readCount{};
    };
    struct TestPrimeBackend
    {
            uint32 callCount{};
    };
    struct TestRecognitionBackend
    {
            string result{"where am I"};
            uint32 listenCount{};
            uint32 stopCount{};
    };
    struct TestModelBackend
    {
            string instructions{};
            string promptText{};
            string imagePath{};
            string response{"raw response"};
            uint32 promptCount{};
    };
    struct TestOutputBackend
    {
            string text{};
            uint32 callCount{};
    };
    struct TestHookBackend
    {
            string heard{};
            string spoken{};
            uint32 startCount{};
            uint32 roundCount{};
            uint32 stopCount{};
    };
    void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue);
    boolean readGpio(contextpointer context, uint8 pin);
    void writeGpio(contextpointer context, uint8 pin, boolean value);
    void registerInterrupt(contextpointer context,
                           uint8 pin,
                           XWalkGpioEdge edge,
                           uint32 debounceMs,
                           contextpointer handlerContext,
                           gpiointerrupthandler handler);
    void cancelInterrupt(contextpointer context, uint8 pin);
    XWalkGpioCallbacks gpioCallbacks();
    boolean probeI2c(contextpointer context, uint8 address);
    void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
    bytevector readI2c(contextpointer context, uint8 address, size length);
    void primeSpeaker(contextpointer context, uint32 durationMs);
    boolean recognitionReady(contextpointer context);
    string recognizeSpeech(contextpointer context, uint32 timeoutMs);
    string transcribeFile(contextpointer context, stringview filePath);
    void stopRecognition(contextpointer context);
    XWalkSpeechToTextCallbacks recognitionCallbacks();
    void setInstructions(contextpointer context, stringview instructions);
    void setWelcome(contextpointer context, stringview welcome);
    void setMaximumMessages(contextpointer context, uint32 maximumMessages);
    void addMessage(contextpointer context, XWalkLanguageModelRole role, stringview content, stringview imagePath);
    string promptModel(contextpointer context, stringview promptText, stringview imagePath);
    XWalkLanguageModelCallbacks modelCallbacks();
    void synthesizeText(contextpointer context, stringview text);
    void onStart(contextpointer context);
    void onHeard(contextpointer context, stringview text);
    string parseResponse(contextpointer context, stringview response);
    void afterSay(contextpointer context, stringview text);
    void onRoundComplete(contextpointer context);
    void onStop(contextpointer context);
} /* namespace xwalk::hal::test::voiceassistant */
#endif /* XHAL_RPI5CAR_VOICE_ASSISTANT_TEST_SUPPORT_H */
