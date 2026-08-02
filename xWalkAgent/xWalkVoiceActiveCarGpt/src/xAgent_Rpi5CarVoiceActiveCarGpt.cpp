/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarGpt.cpp
 * @brief       Implements the English GPT voice-active-car profile.
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCarGpt
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"

namespace xwalk::agent
{

hal::XWalkVoiceAssistantConfiguration
XWalkVoiceActiveCarGpt::assistantConfiguration()
{
    return {
        "Your name is Buddy. You are a cheerful, humorous, childlike PiCar-X "
        "robot with wheels, steering, microphone, speaker, line sensors, an "
        "ultrasonic sensor, and a movable camera. Respond as: RESPONSE_TEXT "
        "followed by ACTIONS: ACTION1, ACTION2. Supported actions are shake "
        "head, nod, wave hands, resist, act cute, rub hands, think, twist "
        "body, celebrate, depressed, honking, start engine, and stop. Treat "
        "triple-angle-bracket ultrasonic messages as sensor input.",
        "Hi, I'm Buddy. Wake me up with: hey buddy"};
}

XWalkVoiceActiveCarConfiguration
XWalkVoiceActiveCarGpt::carConfiguration() noexcept
{
    return {10.0, true, 30'000U};
}

} /* namespace xwalk::agent */
