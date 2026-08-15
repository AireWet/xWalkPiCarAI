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

namespace xwalk::agent
{

    /**
     * @brief Runs the base PiCar-X service callback.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] picarx Caller-owned coordinator valid through this call.
     * @return Status returned by `callback`.
     */
    agent::int32
    XWalkBootRpi::runBase(agent::contextpointer context, bootapplicationcallback callback, XWalkPicarx& picarx)
    {
        XWalkBootServices services{};
        services.picarx = &picarx;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
