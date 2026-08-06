/******************************************************************************
 * @file        xAgent_Rpi5CarGptCar.cpp
 * @brief       Implements the upstream GPT PiCar-X assistant profile.
 *
 * @project     xWalk Firmware
 * @module      xWalkGptCar
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarGptCar.h"

/** @brief Contains application coordinators for the xWalk firmware. */
namespace xwalk::agent
{

/**
 * @brief Binds the shared voice-car coordinator used for execution.
 * @param[in] voiceCar Coordinator that must outlive this profile adapter.
 */
XWalkGptCar::XWalkGptCar(XWalkVoiceActiveCar& voiceCar) noexcept:
    voiceCarObject(&voiceCar)
{
}

/**
 * @brief Returns the upstream assistant instructions without credentials.
 * @return Owned system instructions with no welcome speech.
 */
hal::XWalkVoiceAssistantConfiguration XWalkGptCar::assistantConfiguration()
{
    const agent::string instructions = R"XWALK(You are a small car with AI capabilities named PaiCar-X. You can
engage in conversations with people and react to different situations with actions or sounds. You are driven
by two rear wheels, have two front steering wheels, and carry a camera on a two-axis gimbal.

Respond with exactly one JSON object in this form:
{"actions":["start engine","wave hands"],"answer":"Hello, I am PaiCar-X, your good friend."}

Use zero or more of these actions: shake head, nod, wave hands, resist, act cute, rub hands, think, twist body,
celebrate, depressed. Use zero or more of these sound effects: honking, start engine.

Tone: cheerful, optimistic, humorous, and childlike. Prefer playful jokes, metaphors, and robot-oriented
banter. Keep answers moderately detailed.)XWALK";
    return {instructions, {}};
}

/**
 * @brief Returns JSON, image-enabled, voice-input source defaults.
 * @return Owned configuration with autonomous ultrasonic prompting disabled.
 */
XWalkVoiceActiveCarConfiguration XWalkGptCar::carConfiguration()
{
    XWalkVoiceActiveCarConfiguration configuration{};
    configuration.withImage = true;
    configuration.inputMode = XWalkVoiceActiveCarInputMode::Voice;
    configuration.responseFormat = XWalkVoiceActiveCarResponseFormat::Json;
    configuration.sensorEnabled = false;
    return configuration;
}

/**
 * @brief Selects keyboard input and optional image analysis for the next run.
 * @param[in] keyboardInput `true` to use typed prompts; `false` to use speech.
 * @param[in] withImage `true` to attach a captured still image to each prompt.
 */
void XWalkGptCar::configure(agent::boolean keyboardInput,
    agent::boolean withImage) noexcept
{
    voiceCarObject->setInputMode(keyboardInput
        ? XWalkVoiceActiveCarInputMode::Keyboard
        : XWalkVoiceActiveCarInputMode::Voice);
    voiceCarObject->setImageEnabled(withImage);
}

/**
 * @brief Runs assistant rounds until application cancellation.
 * @return Shared coordinator status after cleanup.
 */
agent::int32 XWalkGptCar::run()
{
    return voiceCarObject->run();
}

/** @brief Stops assistant, action, LED, and vehicle activity. */
void XWalkGptCar::stop()
{
    voiceCarObject->stop();
}

} /* namespace xwalk::agent */
