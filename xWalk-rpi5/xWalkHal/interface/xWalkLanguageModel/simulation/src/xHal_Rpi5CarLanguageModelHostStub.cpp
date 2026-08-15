/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelHostStub.cpp
 * @brief       Implements the network-free language-model host stub.
 * @details     Stores callback inputs in memory and returns fixed response
 *text.
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarLanguageModelHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    XWalkLanguageModelHostStub::XWalkLanguageModelHostStub() = default;
    XWalkLanguageModelHostStub::~XWalkLanguageModelHostStub() = default;
    void XWalkLanguageModelHostStub::setInstructions(contextpointer context, stringview instructions)
    {
        static_cast<XWalkLanguageModelHostStub*>(context)->instructionsValue = string(instructions);
    }
    void XWalkLanguageModelHostStub::setWelcome(contextpointer context, stringview welcome)
    {
        static_cast<XWalkLanguageModelHostStub*>(context)->welcomeValue = string(welcome);
    }
    void XWalkLanguageModelHostStub::setMaximumMessages(contextpointer context, uint32 maximumMessages)
    {
        static_cast<XWalkLanguageModelHostStub*>(context)->maximumMessagesValue = maximumMessages;
    }
    void XWalkLanguageModelHostStub::addMessage(contextpointer context,
                                                XWalkLanguageModelRole role,
                                                stringview content,
                                                stringview imagePath)
    {
        static_cast<void>(role);
        static_cast<void>(content);
        static_cast<void>(imagePath);
        ++static_cast<XWalkLanguageModelHostStub*>(context)->messageCountValue;
    }
    string XWalkLanguageModelHostStub::prompt(contextpointer context, stringview promptText, stringview imagePath)
    {
        static_cast<void>(imagePath);
        auto& stub = *static_cast<XWalkLanguageModelHostStub*>(context);
        stub.promptValue = string(promptText);
        XWALK_HAL_TRACE_UID0(RPI .149, "Host language-model prompt mirrored without network access");
        return "simulated response";
    }
    XWalkLanguageModelCallbacks XWalkLanguageModelHostStub::callbacks() noexcept
    {
        return {&setInstructions, &setWelcome, &setMaximumMessages, &addMessage, &prompt};
    }
    string XWalkLanguageModelHostStub::promptText() const
    {
        return promptValue;
    }
    uint32 XWalkLanguageModelHostStub::maximumMessages() const noexcept
    {
        return maximumMessagesValue;
    }
    uint32 XWalkLanguageModelHostStub::messageCount() const noexcept
    {
        return messageCountValue;
    }
} /* namespace xwalk::hal::sim */
