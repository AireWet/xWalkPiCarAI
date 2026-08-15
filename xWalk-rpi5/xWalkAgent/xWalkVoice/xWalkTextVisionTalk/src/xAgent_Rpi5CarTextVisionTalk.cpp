/******************************************************************************
 * @file        xAgent_Rpi5CarTextVisionTalk.cpp
 * @brief       Implements the example-17 image-grounded prompt loop.
 * @project     xWalk Firmware
 * @module      xWalkTextVisionTalk
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarTextVisionTalk.h"

/** @namespace xwalk::agent @brief Contains application coordinators for xWalk firmware. */
namespace xwalk::agent
{

    /**
     * @brief Runs the configured image-grounded text conversation.
     * @return Zero after exit, quit, or foreground cancellation.
     */
    agent::int32 XWalkTextVisionTalk::run()
    {
        languageModelObject->setMaximumMessages(configuration.maximumMessages);
        languageModelObject->setInstructions(configuration.instructions);
        languageModelObject->setWelcome(configuration.welcome);
        callbacks.delay(callbackContext, configuration.cameraWarmupMs);
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
            const agent::string normalized = normalize(inputText);
            if ((normalized == "exit") || (normalized == "quit"))
            {
                break;
            }
            const agent::string imagePath = cameraCaptureObject->capture();
            const agent::string response = languageModelObject->prompt(inputText, imagePath);
            callbacks.output(callbackContext, response);
        }
        return 0;
    }

    /**
     * @brief Requires no persistent provider shutdown because every operation is synchronous.
     */
    void XWalkTextVisionTalk::stop() noexcept
    {
    }

} /* namespace xwalk::agent */
