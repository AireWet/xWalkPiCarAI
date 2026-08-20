/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiLineTracking.cpp
 * @brief       Composes Raspberry Pi line tracking.
 * @details     Publishes the foreground line-tracking coordinator with PiCar-X.
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"
#include "xHal_Rpi5CarTrace.h"

#include "xAgent_Rpi5CarLineTracking.h"

namespace xwalk::agent
{

    /**
     * @brief Runs line tracking with the caller-owned PiCar-X coordinator.
     * @param[in] parameters Non-owning application callback and PiCar-X
     * dependency valid through this synchronous composition.
     * @return Status returned by the configured callback.
     * @pre `parameters.callback` and `parameters.picarx` are non-null.
     */
    agent::int32 XWalkBootRpi::runLineTracking(const xAgentContext& parameters)
    {
        XWalkPicarx& picarx = *parameters.picarx;
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .055, "Boot composing line-tracking services");
        XWalkLineTracking lineTracking(picarx, nullptr, &delayMilliseconds);
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.lineTracking = &lineTracking;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
