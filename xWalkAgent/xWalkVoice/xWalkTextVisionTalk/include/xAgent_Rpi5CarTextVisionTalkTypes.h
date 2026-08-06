/******************************************************************************
 * @file        xAgent_Rpi5CarTextVisionTalkTypes.h
 * @brief       Declares text-vision-talk callbacks and configuration.
 * @project     xWalk Firmware
 * @module      xWalkTextVisionTalk
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_TEXT_VISION_TALK_TYPES_H
#define XAGENT_RPI5CAR_TEXT_VISION_TALK_TYPES_H

#include "xHal_Rpi5CarTypes.h"

/** @namespace xwalk::agent @brief Contains application coordinators for xWalk firmware. */
namespace xwalk::agent
{

/** @brief Writes one complete status or response line. */
using textvisiontalkoutputcallback = void (*)(agent::contextpointer, agent::stringview);
/** @brief Reads one prompt from the caller-owned interactive input. */
using textvisiontalkinputcallback = agent::string (*)(agent::contextpointer, agent::stringview);
/** @brief Suspends the calling thread for a requested millisecond duration. */
using textvisiontalkdelaycallback = void (*)(agent::contextpointer, agent::uint32);
/** @brief Reports whether the foreground conversation may continue. */
using textvisiontalkcontinuecallback = agent::boolean (*)(agent::contextpointer);

/** @brief Stores the complete synchronous application callback boundary. */
struct XWalkTextVisionTalkCallbacks
{
    textvisiontalkoutputcallback output{nullptr}; /**< Writes status and final model responses. */
    textvisiontalkinputcallback input{nullptr}; /**< Reads one typed prompt. */
    textvisiontalkdelaycallback delay{nullptr}; /**< Performs camera warm-up timing. */
    textvisiontalkcontinuecallback shouldContinue{nullptr}; /**< Controls the prompt loop. */
};

/** @brief Stores source-compatible example-17 conversation settings. */
struct XWalkTextVisionTalkConfiguration
{
    agent::string instructions{"You are a helpful assistant."}; /**< Source system instructions. */
    agent::string welcome{"Hello, I am a helpful assistant. How can I help you?"}; /**< Source greeting. */
    agent::string promptText{">>> "}; /**< Interactive input prompt. */
    agent::uint32 maximumMessages{20U}; /**< Non-zero retained-message count. */
    agent::uint32 cameraWarmupMs{2'000U}; /**< Non-zero warm-up duration in milliseconds. */
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_TEXT_VISION_TALK_TYPES_H */
