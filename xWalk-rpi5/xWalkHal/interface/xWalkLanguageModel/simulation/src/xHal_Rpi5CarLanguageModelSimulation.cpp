/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelSimulation.cpp
 * @brief       Implements the network-free language-model simulation.
 * @details     Exercises conversation and prompt dispatch through the host
 *stub.
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarLanguageModelSimulation.h"
#include "xHal_Rpi5CarLanguageModelHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    int32 runLanguageModelSimulation()
    {
        XWalkLanguageModelHostStub stub;
        XWalkLanguageModel model(&stub, stub.callbacks());
        model.setInstructions("Answer briefly");
        model.setWelcome("Ready");
        model.setMaximumMessages(4U);
        model.addMessage(XWalkLanguageModelRole::Assistant, "Prior response");
        const string response = model.prompt("Status");
        const boolean succeeded = (response == "simulated response") && (stub.promptText() == "Status") &&
                                  (stub.maximumMessages() == 4U) && (stub.messageCount() == 1U);
        XWALK_HAL_TRACE_UID0(RPI .150, "xWalkLanguageModel host simulation completed");
        return succeeded ? 0 : 1;
    }
} // namespace xwalk::hal::sim
