/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarGptTest.cpp
 * @brief       Verifies the English GPT voice-active-car profile.
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
    const auto assistant =
        xwalk::agent::XWalkVoiceActiveCarGpt::assistantConfiguration();
    const auto car = xwalk::agent::XWalkVoiceActiveCarGpt::carConfiguration();
    assert(assistant.instructions.find("Your name is Buddy") !=
        xwalk::agent::string::npos);
    assert(assistant.instructions.find("ACTIONS: ACTION1, ACTION2") !=
        xwalk::agent::string::npos);
    assert(assistant.instructions.find("forward") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("start engine") !=
        xwalk::agent::string::npos);
    assert(assistant.welcome == "Hi, I'm Buddy. Wake me up with: hey buddy");
    assert(car.tooCloseCm == 10.0);
    assert(car.withImage);
    assert(car.listenTimeoutMs == 30'000U);
    assert(car.wakeEnabled);
    assert(car.wakeWord == "hey buddy");
    assert(car.answerOnWake == "Hi there");
    assert(xwalk::agent::string(
        xwalk::agent::XWalkVoiceActiveCarGpt::MODEL_NAME) == "gpt-4o-mini");
    assert(xwalk::agent::string(
        xwalk::agent::XWalkVoiceActiveCarGpt::SPEECH_VOICE) ==
        "en_US-ryan-low");
    return 0;
}
