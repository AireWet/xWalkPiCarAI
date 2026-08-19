/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiDoctor.cpp
 * @brief       Composes the bounded Raspberry Pi Doctor preflight.
 *
 * @details
 * Produces the deployment inspection service after the MCU-reset-only
 * preflight without claiming actuator or media resources.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"
#include "xHal_Rpi5CarTrace.h"

#include "xAgent_Rpi5CarDoctorLinux.h"

namespace xwalk::agent
{

    /**
     * @brief Runs the bounded MCU-reset Doctor preflight.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runDoctor(agent::contextpointer context, bootapplicationcallback callback)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .052, "Boot starting bounded Doctor composition");
        const agent::stringvector doctorLines = XWalkDoctorLinux::inspect(configurationFilePath);
        XWalkBootServices services{};
        services.doctorLines = &doctorLines;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
