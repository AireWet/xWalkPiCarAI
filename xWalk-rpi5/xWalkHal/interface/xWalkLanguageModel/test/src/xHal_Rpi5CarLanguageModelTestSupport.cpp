/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelTestSupport.cpp
 * @brief       Implements reusable language-model host-test support.
 * @details     Records callback inputs and returns deterministic response text.
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarLanguageModelTestSupport.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::test::language_model
{
    void setInstructions(contextpointer context, stringview instructions)
    {
        auto& backend = *static_cast<TestLanguageModelBackend*>(context);
        ++backend.instructionCount;
        backend.instructions = string(instructions);
    }
    void setWelcome(contextpointer context, stringview welcome)
    {
        auto& backend = *static_cast<TestLanguageModelBackend*>(context);
        ++backend.welcomeCount;
        backend.welcome = string(welcome);
    }
    void setMaximumMessages(contextpointer context, uint32 maximumMessages)
    {
        auto& backend = *static_cast<TestLanguageModelBackend*>(context);
        ++backend.limitCount;
        backend.maximumMessages = maximumMessages;
    }
    void addMessage(contextpointer context, XWalkLanguageModelRole role, stringview content, stringview imagePath)
    {
        auto& backend = *static_cast<TestLanguageModelBackend*>(context);
        ++backend.messageCount;
        backend.role = role;
        backend.messageContent = string(content);
        backend.messageImagePath = string(imagePath);
    }
    string prompt(contextpointer context, stringview promptText, stringview imagePath)
    {
        auto& backend = *static_cast<TestLanguageModelBackend*>(context);
        ++backend.promptCount;
        backend.promptText = string(promptText);
        backend.promptImagePath = string(imagePath);
        if (backend.failPrompt)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Test language-model prompt failed");
        }
        return backend.promptResult;
    }
    XWalkLanguageModelCallbacks backendCallbacks()
    {
        return {&setInstructions, &setWelcome, &setMaximumMessages, &addMessage, &prompt};
    }
} // namespace xwalk::hal::test::language_model
