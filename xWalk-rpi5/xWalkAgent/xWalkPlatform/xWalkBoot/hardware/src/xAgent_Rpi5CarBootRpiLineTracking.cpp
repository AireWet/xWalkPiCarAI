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
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @return Status returned by `callback`.
     */
    agent::int32
    XWalkBootRpi::runLineTracking(agent::contextpointer context, bootapplicationcallback callback, XWalkPicarx& picarx)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .055, "Boot composing line-tracking services");
        XWalkLineTracking lineTracking(picarx, nullptr, &delayMilliseconds);
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.lineTracking = &lineTracking;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
