/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarTest.cpp
 * @brief       Verifies voice-active-car response parsing without devices.
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCar Host Test
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarVoiceActiveCar.h"
#include "xAgent_Rpi5CarVoiceControlledCar.h"

#include <cassert>

/** @brief Runs deterministic response-parser assertions. @return Zero on success. */
int main()
{
    const xwalk::agent::XWalkVoiceActiveCarResponse response =
        xwalk::agent::XWalkVoiceActiveCar::parseResponse(
            "Hello there\nACTIONS: wave hands, honking");
    assert(response.text == "Hello there");
    assert(response.actions.size() == 2U);
    assert(response.actions[0U] == "wave hands");
    assert(response.actions[1U] == "honking");

    const xwalk::agent::XWalkVoiceActiveCarResponse fallback =
        xwalk::agent::XWalkVoiceActiveCar::parseResponse("No movement");
    assert(fallback.text == "No movement");
    assert(fallback.actions.size() == 1U);
    assert(fallback.actions[0U] == "stop");

    using xwalk::agent::XWalkVoiceControlledCar;
    using xwalk::agent::XWalkVoiceControlledCarCommand;
    assert(XWalkVoiceControlledCar::containsWakeWord("HEY ROBOT, please wake up"));
    assert(!XWalkVoiceControlledCar::containsWakeWord("hello car"));
    assert(XWalkVoiceControlledCar::classifyCommand("please go FORWARD") ==
        XWalkVoiceControlledCarCommand::Forward);
    assert(XWalkVoiceControlledCar::classifyCommand("move backward") ==
        XWalkVoiceControlledCarCommand::Backward);
    assert(XWalkVoiceControlledCar::classifyCommand("turn left") ==
        XWalkVoiceControlledCarCommand::Left);
    assert(XWalkVoiceControlledCar::classifyCommand("turn right") ==
        XWalkVoiceControlledCarCommand::Right);
    assert(XWalkVoiceControlledCar::classifyCommand("go to sleep") ==
        XWalkVoiceControlledCarCommand::Sleep);
    assert(XWalkVoiceControlledCar::classifyCommand("dance") ==
        XWalkVoiceControlledCarCommand::Unknown);
    return 0;
}
