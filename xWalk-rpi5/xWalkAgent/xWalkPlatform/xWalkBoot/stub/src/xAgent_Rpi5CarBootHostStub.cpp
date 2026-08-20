/******************************************************************************
 * @file        xAgent_Rpi5CarBootHostStub.cpp
 * @brief       Implements the device-free xWalkBoot host stub.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot Host Stub
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBootHostStub.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs one host stub around caller-owned simulated services.
     * @param[in] simulatedServices Service pointers whose targets outlive this object.
     */
    XWalkBootHostStub::XWalkBootHostStub(const XWalkBootServices& simulatedServices) noexcept
        : services(simulatedServices)
    {
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Executes one application callback with simulated services.
     * @param[in] parameters Application context and non-null callback retained
     * through the synchronous host dispatch.
     * @return Status returned by the configured callback.
     * @throws std::invalid_argument If the configured callback is null.
     * @throws std::logic_error If this object already started once.
     */
    agent::int32 XWalkBootHostStub::run(const xAgentContext& parameters)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .075, "Host boot stub publishing simulated services");
        begin(parameters.callback);
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
