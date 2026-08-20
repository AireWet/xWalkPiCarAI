/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiBase.cpp
 * @brief       Publishes the base Raspberry Pi PiCar-X service.
 * @details     Exposes only the caller-owned PiCar-X coordinator.
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

namespace xwalk::agent
{

    /**
     * @brief Runs the base PiCar-X service callback.
     * @param[in] parameters Non-owning application callback and PiCar-X
     * dependency valid through this synchronous composition.
     * @return Status returned by the configured callback.
     * @pre `parameters.callback` and `parameters.picarx` are non-null.
     */
    agent::int32 XWalkBootRpi::runBase(const xAgentContext& parameters)
    {
        XWalkPicarx& picarx = *parameters.picarx;
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .049, "Boot publishing base PiCar-X services");
        XWalkBootServices services{};
        services.picarx = &picarx;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
