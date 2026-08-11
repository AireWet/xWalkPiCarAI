/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelTestSupport.h
 * @brief       Declares reusable language-model host-test support.
 * @details     Defines deterministic callback state outside scenario sources.
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LANGUAGE_MODEL_TEST_SUPPORT_H
#define XHAL_RPI5CAR_LANGUAGE_MODEL_TEST_SUPPORT_H
#include "xHal_Rpi5CarLanguageModel.h"
namespace xwalk::hal::test::language_model
{
struct TestLanguageModelBackend
{
    string instructions{};
    string welcome{};
    string messageContent{};
    string messageImagePath{};
    string promptText{};
    string promptImagePath{};
    string promptResult{"model response"};
    XWalkLanguageModelRole role{XWalkLanguageModelRole::System};
    uint32 maximumMessages{};
    uint32 instructionCount{};
    uint32 welcomeCount{};
    uint32 limitCount{};
    uint32 messageCount{};
    uint32 promptCount{};
    boolean failPrompt{};
};
void setInstructions(contextpointer context, stringview instructions);
void setWelcome(contextpointer context, stringview welcome);
void setMaximumMessages(contextpointer context, uint32 maximumMessages);
void addMessage(contextpointer context, XWalkLanguageModelRole role,
    stringview content, stringview imagePath);
string prompt(contextpointer context, stringview promptText, stringview imagePath);
XWalkLanguageModelCallbacks backendCallbacks();
}
#endif
