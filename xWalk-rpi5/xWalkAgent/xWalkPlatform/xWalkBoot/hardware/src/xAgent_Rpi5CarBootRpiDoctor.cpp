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
     * @param[in] parameters Application context and non-null callback retained
     * through this synchronous composition.
     * @return Status returned by the configured callback.
     * @pre `parameters.callback` is non-null.
     */
    agent::int32 XWalkBootRpi::runDoctor(const xAgentContext& parameters)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .052, "Boot starting bounded Doctor composition");
        const agent::stringvector doctorLines = XWalkDoctorLinux::inspect(configurationFilePath);
        XWalkBootServices services{};
        services.doctorLines = &doctorLines;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
