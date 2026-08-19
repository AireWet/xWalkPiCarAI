/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarGptTest.cpp
 * @brief       Verifies the Gemini-backed Jarvis voice-active-car profile.
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCarGpt Host Test
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"

#include <cassert>

/**
 * @brief Runs deterministic example-21 profile assertions.
 * @return Zero after every source-compatible default is verified.
 */
int main()
{
    const auto assistant = xwalk::agent::XWalkVoiceActiveCarGpt::assistantConfiguration();
    const auto car = xwalk::agent::XWalkVoiceActiveCarGpt::carConfiguration();
    assert(assistant.instructions.find("Your name is Jarvis") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("ACTIONS: ACTION1, ACTION2") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("forward") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("start engine") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("play background music") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("Emoji may appear only in RESPONSE_TEXT") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("Never invent an action name") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("Answer safe general-knowledge") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("A question does not need to request a robot action") !=
           xwalk::agent::string::npos);
    assert(assistant.welcome == "Hi, I'm Jarvis. Wake me up with: hey jarvis");
    assert(car.tooCloseCm == 10.0);
    assert(car.withImage);
    assert(car.listenTimeoutMs == 30'000U);
    assert(car.wakeEnabled);
    assert(car.wakeWord == "hey jarvis");
    assert(car.answerOnWake == "Systems online. Ready when you are, Joxy.");
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::NAME) == "Jarvis");
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::MODEL_NAME) == "gemini-3.7-flash");
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::API_KEY_ENVIRONMENT) == "GEMINI_API_KEY");
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::MODEL_ENDPOINT) ==
           "https://generativelanguage.googleapis.com/v1beta/openai/chat/completions");
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::SPEECH_VOICE) ==
           "/usr/share/xwalk/models/piper/en_GB-alan-medium.onnx");
    return 0;
}
