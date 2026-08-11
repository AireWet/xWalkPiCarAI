/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelHostStub.h
 * @brief       Declares the network-free language-model host stub.
 * @details     Mirrors conversation callbacks and returns deterministic responses.
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LANGUAGE_MODEL_HOST_STUB_H
#define XHAL_RPI5CAR_LANGUAGE_MODEL_HOST_STUB_H
#include "xHal_Rpi5CarLanguageModel.h"
namespace xwalk::hal::sim
{
class XWalkLanguageModelHostStub final
{
    private:
        string instructionsValue;
        string welcomeValue;
        string promptValue;
        uint32 maximumMessagesValue{};
        uint32 messageCountValue{};
    protected:
        static void setInstructions(contextpointer context, stringview instructions);
        static void setWelcome(contextpointer context, stringview welcome);
        static void setMaximumMessages(contextpointer context, uint32 maximumMessages);
        static void addMessage(contextpointer context, XWalkLanguageModelRole role,
            stringview content, stringview imagePath);
        static string prompt(contextpointer context, stringview promptText, stringview imagePath);
    public:
        XWalkLanguageModelHostStub();
        ~XWalkLanguageModelHostStub();
        XWalkLanguageModelHostStub(const XWalkLanguageModelHostStub&) = delete;
        XWalkLanguageModelHostStub& operator=(const XWalkLanguageModelHostStub&) = delete;
        XWalkLanguageModelHostStub(XWalkLanguageModelHostStub&&) = delete;
        XWalkLanguageModelHostStub& operator=(XWalkLanguageModelHostStub&&) = delete;
        XWalkLanguageModelCallbacks callbacks() noexcept;
        string promptText() const;
        uint32 maximumMessages() const noexcept;
        uint32 messageCount() const noexcept;
};
} /* namespace xwalk::hal::sim */
#endif
