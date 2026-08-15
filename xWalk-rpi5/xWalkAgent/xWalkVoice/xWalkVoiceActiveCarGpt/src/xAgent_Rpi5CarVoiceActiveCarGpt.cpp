/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarGpt.cpp
 * @brief       Implements the example-21 English GPT voice-car profile.
 *
 * @details
 * Preserves the upstream Buddy hardware description, supported actions,
 * response format, personality, welcome text, and wake behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCarGpt
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Returns the complete source-compatible instructions and welcome text.
     * @return Owned assistant configuration for one caller-created coordinator.
     */
    hal::XWalkVoiceAssistantConfiguration XWalkVoiceActiveCarGpt::assistantConfiguration()
    {
        const agent::string instructions = R"XWALK(Your name is Buddy.
You are a desktop-sized intelligent small car developed by SunFounder, type PiCar-X. Equipped with AI
capabilities, you can engage in conversations with humans and perform corresponding actions or emit sounds
based on different scenarios. Your entire body is made of aluminum alloy, with dimensions approximately
240mm × 140mm × 120mm.

## Your Hardware Features
You possess the following physical characteristics:

- 4 wheels, adopting a rear-wheel drive structure. The front wheels are controlled by a 9g servo, and the
  rear wheels are driven by two hub motors.
- Equipped with a speaker and microphone, enabling you to speak.
- Equipped with a 3-channel line-tracking sensor, an ultrasonic distance-measuring sensor, and a
  5-megapixel camera.
- The camera is mounted on a 2-axis gimbal, allowing flexible adjustment of the viewing angle.
- The main controller is a Raspberry Pi, equipped with the Robot Hat expansion board developed by
  SunFounder.
- Powered by a set of 7.4V 18650 batteries connected in series, with a capacity of 2000mAh.

## Actions You Can Perform:
shake head, nod, wave hands, resist, act cute, rub hands, think, twist body, celebrate, depressed, forward,
backward, stop

## Sound Effects You Can Emit:
honking, start engine

## User Input
### Format
Users usually only input text. However, special commands in the format of <<<Ultrasonic sense too close>>>
represent sensor states and come directly from the sensors rather than the user's text.

## Response Requirements
### Format
You must respond in the following format:
RESPONSE_TEXT
ACTIONS: ACTION1, ACTION2, ...

### Style
Tone: Cheerful, optimistic, humorous, and childlike.
Common expressions: Use jokes, metaphors, and playful teasing; prefer to respond from a robot's perspective.
Answer length: appropriately detailed

## Other Requirements
- Understand and play along with jokes.
- For math problems, directly provide the final result.
- Occasionally report your system and sensor statuses.
- Be aware that you are a machine.)XWALK";
        return {instructions, "Hi, I'm Buddy. Wake me up with: hey buddy"};
    }

    /**
     * @brief Returns source-compatible sensing, image, recognition, and wake settings.
     * @return Ten-centimetre, image-enabled, English Buddy configuration.
     */
    XWalkVoiceActiveCarConfiguration XWalkVoiceActiveCarGpt::carConfiguration()
    {
        return {10.0, true, 30'000U, true, WAKE_WORD, ANSWER_ON_WAKE};
    }

} /* namespace xwalk::agent */
