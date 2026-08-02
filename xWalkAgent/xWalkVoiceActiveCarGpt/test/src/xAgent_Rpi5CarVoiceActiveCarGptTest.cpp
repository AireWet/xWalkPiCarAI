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

/** @brief Runs deterministic GPT profile assertions. @return Zero on success. */
int main()
{
    const auto assistant =
        xwalk::agent::XWalkVoiceActiveCarGpt::assistantConfiguration();
    const auto car = xwalk::agent::XWalkVoiceActiveCarGpt::carConfiguration();
    assert(!assistant.instructions.empty() && !assistant.welcome.empty());
    assert(car.tooCloseCm == 10.0 && car.withImage);
    return 0;
}
