/******************************************************************************
 * @file        xAgent_Rpi5CarOnlineLlmTestTypes.h
 * @brief       Declares online-LLM example callbacks and configuration.
 * @project     xWalk Firmware
 * @module      xWalkOnlineLlmTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_ONLINE_LLM_TEST_TYPES_H
#define XAGENT_RPI5CAR_ONLINE_LLM_TEST_TYPES_H

#include "xHal_Rpi5CarTypes.h"

/** @namespace xwalk::agent @brief Contains application coordinators for xWalk firmware. */
namespace xwalk::agent
{

/** @brief Writes one complete welcome or final response line. */
using onlinellmtestoutputcallback = void (*)(agent::contextpointer, agent::stringview);
/** @brief Reads one typed prompt from caller-owned input. */
using onlinellmtestinputcallback = agent::string (*)(agent::contextpointer, agent::stringview);
/** @brief Reports whether the foreground prompt loop may continue. */
using onlinellmtestcontinuecallback = agent::boolean (*)(agent::contextpointer);

/** @brief Stores the complete synchronous application callback boundary. */
struct XWalkOnlineLlmTestCallbacks
{
    onlinellmtestoutputcallback output{nullptr}; /**< Writes welcome and responses. */
    onlinellmtestinputcallback input{nullptr}; /**< Reads one prompt. */
    onlinellmtestcontinuecallback shouldContinue{nullptr}; /**< Controls the prompt loop. */
};

/** @brief Stores source-compatible example-18 conversation settings. */
struct XWalkOnlineLlmTestConfiguration
{
    agent::string instructions{"You are a helpful assistant."}; /**< Source system instructions. */
    agent::string welcome{"Hello, I am a helpful assistant. How can I help you?"}; /**< Source greeting. */
    agent::string promptText{">>> "}; /**< Interactive input prompt. */
    agent::uint32 maximumMessages{20U}; /**< Non-zero retained-message count. */
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_ONLINE_LLM_TEST_TYPES_H */
