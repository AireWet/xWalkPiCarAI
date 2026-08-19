/******************************************************************************
 * @file        xAgent_Rpi5CarOnlineLlmTest.cpp
 * @brief       Implements the example-18 online text prompt loop.
 * @project     xWalk Firmware
 * @module      xWalkOnlineLlmTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarOnlineLlmTest.h"

#include "xHal_Rpi5CarTrace.h"

/** @namespace xwalk::agent @brief Contains application coordinators for xWalk firmware. */
namespace xwalk::agent
{

    /** @brief Runs the configured text-only conversation. @return Zero after cancellation. */
    agent::int32 XWalkOnlineLlmTest::run()
    {
        XWALK_RPIAGENT_TRACE_UID1(
            RPIAGENT .039, "Online language-model loop started with a %u-message bound", configuration.maximumMessages);
        languageModelObject->setMaximumMessages(configuration.maximumMessages);
        languageModelObject->setInstructions(configuration.instructions);
        languageModelObject->setWelcome(configuration.welcome);
        callbacks.output(callbackContext, configuration.welcome);
        const agent::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const agent::boolean operationMayContinue =
                static_cast<agent::boolean>(callbacks.shouldContinue(callbackContext));
            if (operationMayContinue == false)
            {
                break;
            }
            const agent::string inputText = callbacks.input(callbackContext, configuration.promptText);
            callbacks.output(callbackContext, languageModelObject->prompt(inputText));
        }
        return 0;
    }

    /** @brief Requires no persistent provider shutdown because each request is synchronous. */
    void XWalkOnlineLlmTest::stop() noexcept
    {
    }

} /* namespace xwalk::agent */
