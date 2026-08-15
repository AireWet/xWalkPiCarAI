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
#include "xHal_Rpi5CarFileFunctions.h"
#include <cassert>

/** @brief Runs deterministic response-parser assertions. @return Zero on success. */
int main()
{
    const xwalk::agent::XWalkVoiceActiveCarResponse response =
        xwalk::agent::XWalkVoiceActiveCar::parseResponse("Hello there\nACTIONS: wave hands, honking");
    assert(response.text == "Hello there");
    assert(response.actions.size() == 2U);
    assert(response.actions[0U] == "wave hands");
    assert(response.actions[1U] == "honking");

    const xwalk::agent::XWalkVoiceActiveCarResponse fallback =
        xwalk::agent::XWalkVoiceActiveCar::parseResponse("No movement");
    assert(fallback.text == "No movement");
    assert(fallback.actions.size() == 1U);
    assert(fallback.actions[0U] == "stop");

    const xwalk::agent::XWalkVoiceActiveCarResponse whitespace =
        xwalk::agent::XWalkVoiceActiveCar::parseResponse("  Hello  \nACTIONS: forward, stop  ");
    assert(whitespace.text == "Hello");
    assert(whitespace.actions.size() == 2U);
    const xwalk::agent::XWalkVoiceActiveCarResponse empty = xwalk::agent::XWalkVoiceActiveCar::parseResponse(" \t\r\n");
    assert(empty.text.empty());
    assert(empty.actions == xwalk::agent::stringvector({"stop"}));

    const xwalk::agent::XWalkVoiceActiveCarResponse json =
        xwalk::agent::XWalkVoiceActiveCar::parseJsonResponse("{\n \"answer\" : \"Line one\\nLine two\\tend\", "
                                                             "\"actions\" : [ \"forward\", \"stop\" ] }");
    assert(json.text == "Line one\nLine two\tend");
    assert(json.actions == xwalk::agent::stringvector({"forward", "stop"}));
    const xwalk::agent::XWalkVoiceActiveCarResponse escaped = xwalk::agent::XWalkVoiceActiveCar::parseJsonResponse(
        "{\"answer\":\"say \\\"hello\\\"\",\"actions\":[\"wave hands\"]}");
    assert(escaped.text == "say \"hello\"");
    assert(escaped.actions == xwalk::agent::stringvector({"wave hands"}));
    const xwalk::agent::XWalkVoiceActiveCarResponse noActions =
        xwalk::agent::XWalkVoiceActiveCar::parseJsonResponse("{\"answer\":\"answer only\"}");
    assert(noActions.text == "answer only");
    assert(noActions.actions.empty());
    const xwalk::agent::XWalkVoiceActiveCarResponse raw =
        xwalk::agent::XWalkVoiceActiveCar::parseJsonResponse("not json");
    assert(raw.text == "not json");
    assert(raw.actions.empty());
    const xwalk::agent::XWalkVoiceActiveCarResponse malformedAnswer =
        xwalk::agent::XWalkVoiceActiveCar::parseJsonResponse("{\"answer\": missing, \"actions\": [broken]}");
    assert(malformedAnswer.text == "{\"answer\": missing, \"actions\": [broken]}");
    assert(malformedAnswer.actions.empty());
    const xwalk::agent::XWalkVoiceActiveCarResponse truncated =
        xwalk::agent::XWalkVoiceActiveCar::parseJsonResponse("{\"answer\":\"unterminated,\"actions\":[\"forward\"");
    assert(truncated.text == "unterminated,");
    assert(truncated.actions == xwalk::agent::stringvector({"forward"}));

    const xwalk::agent::XWalkVoiceActiveCarConfiguration carConfiguration =
        xwalk::agent::XWalkVoiceActiveCar::carConfiguration();
    assert(carConfiguration.tooCloseCm == 10.0);
    assert(carConfiguration.withImage);
    assert(carConfiguration.wakeEnabled);
    assert(carConfiguration.wakeWord == "hey rolly");
    assert(carConfiguration.answerOnWake == "Hi there");
    assert(xwalk::agent::XWalkVoiceActiveCar::KEYBOARD_ENABLED);
    const xwalk::hal::XWalkVoiceAssistantConfiguration assistantConfiguration =
        xwalk::agent::XWalkVoiceActiveCar::assistantConfiguration();
    assert(assistantConfiguration.instructions.find("Your name is Rolly.") != xwalk::agent::string::npos);
    assert(assistantConfiguration.instructions.find("ACTIONS: ACTION1") != xwalk::agent::string::npos);
    assert(assistantConfiguration.welcome == "Hi, I'm Rolly. Wake me up with: hey rolly");
    const xwalk::agent::filesystempath configuredImage =
        xwalk::agent::filesystempath(XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY) / "voice-active-car.jpg";
    assert(xwalk::hal::isReadableRegularFile(configuredImage));
    const xwalk::agent::string imageContents = xwalk::hal::readFileContents(configuredImage);
    assert(imageContents.size() > 3U);
    assert(static_cast<unsigned char>(imageContents[0U]) == 0xFFU);
    assert(static_cast<unsigned char>(imageContents[1U]) == 0xD8U);
    assert(static_cast<unsigned char>(imageContents[2U]) == 0xFFU);

    return 0;
}
