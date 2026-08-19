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
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @return Status returned by `callback`.
     * @throws std::invalid_argument If `callback` is null.
     * @throws std::logic_error If this object already started once.
     */
    agent::int32 XWalkBootHostStub::run(agent::contextpointer context, bootapplicationcallback callback)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .075, "Host boot stub publishing simulated services");
        begin(callback);
        return callback(context, services);
    }

} /* namespace xwalk::agent */
