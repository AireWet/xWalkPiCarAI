/******************************************************************************
 * @file        xAgent_Rpi5CarGptCarTest.cpp
 * @brief       Verifies the upstream GPT-car profile and JSON response parser.
 * @project     xWalk Firmware
 * @module      xWalkGptCar Host Test
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarGptCar.h"

#include <cassert>

/**
 * @brief Runs device-free profile and JSON parser assertions.
 * @return Zero after every assertion passes.
 */
int main()
{
    const xwalk::hal::XWalkVoiceAssistantConfiguration assistant =
        xwalk::agent::XWalkGptCar::assistantConfiguration();
    const xwalk::agent::XWalkVoiceActiveCarConfiguration car =
        xwalk::agent::XWalkGptCar::carConfiguration();
    assert(assistant.instructions.find("PaiCar-X") != xwalk::agent::string::npos);
    assert(assistant.instructions.find("\"actions\"") != xwalk::agent::string::npos);
    assert(car.withImage);
    assert(!car.sensorEnabled);
    assert(car.responseFormat ==
        xwalk::agent::XWalkVoiceActiveCarResponseFormat::Json);

    const xwalk::agent::XWalkVoiceActiveCarResponse response =
        xwalk::agent::XWalkVoiceActiveCar::parseJsonResponse(
            R"({"actions":["honking","wave hands"],"answer":"Hello!"})");
    assert(response.text == "Hello!");
    assert(response.actions.size() == 2U);
    assert(response.actions[0U] == "honking");
    assert(response.actions[1U] == "wave hands");
    return 0;
}
