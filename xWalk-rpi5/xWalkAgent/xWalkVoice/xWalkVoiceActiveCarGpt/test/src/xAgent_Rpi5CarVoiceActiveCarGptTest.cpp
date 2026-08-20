/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarGptTest.cpp
 * @brief       Verifies the provider-neutral Jarvis voice-active-car profile.
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
    assert(assistant.instructions.find("Address the user as Joxy in every RESPONSE_TEXT reply") !=
           xwalk::agent::string::npos);
    assert(assistant.welcome == "Hi, I'm Jarvis. Wake me up with: hey jarvis");
    assert(car.tooCloseCm == 10.0);
    assert(!car.withImage);
    assert(xwalk::agent::XWalkVoiceActiveCarGpt::MAXIMUM_OUTPUT_TOKENS == 256U);
    assert(car.listenTimeoutMs == 30'000U);
    assert(car.wakeEnabled);
    assert(car.wakeWord == "hey jarvis");
    assert(xwalk::agent::XWalkVoiceActiveCar::matchesWakePhrase("noise HEY JARVIS trailing", car.wakeWord));
    assert(car.answerOnWake == "Systems online. Ready when you are, Joxy.");
    assert(car.continuousConversationEnabled);
    assert(car.conversationIdleTimeoutMs == 30'000U);
    assert(car.conversationMaximumRounds == 10U);
    assert(car.conversationMaximumMisses == 3U);
    assert(!car.webSearchEnabled);
    assert(car.sleepPhrases == xwalk::agent::stringvector({"goodbye jarvis", "go to sleep", "stop listening"}));
    assert(assistant.instructions.find("concise and speech-friendly") != xwalk::agent::string::npos);
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::NAME) == "Jarvis");
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::MODEL_PROVIDER) == "ollama");
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::MODEL_NAME) == "llama3.2:3b");
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::API_KEY_ENVIRONMENT).empty());
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::MODEL_ENDPOINT) ==
           "http://127.0.0.1:11434/api/chat");
    assert(xwalk::agent::XWalkVoiceActiveCarGpt::MODEL_TIMEOUT_MS == 120'000U);
    assert(xwalk::agent::XWalkVoiceActiveCarGpt::MAXIMUM_MESSAGES == 20U);
    assert(xwalk::agent::XWalkVoiceActiveCarGpt::WEB_SEARCH_MAXIMUM_RESULTS == 3U);
    assert(xwalk::agent::XWalkVoiceActiveCarGpt::WEB_SEARCH_TIMEOUT_MS == 5'000U);
    assert(xwalk::agent::string(xwalk::agent::XWalkVoiceActiveCarGpt::SPEECH_VOICE) ==
           "/usr/share/xwalk/models/piper/en_GB-alan-medium.onnx");
    return 0;
}
