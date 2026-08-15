/******************************************************************************
 * @file        xControllerOnlineLlmTestHandler.cpp
 * @brief       Implements the OnlineLlmTestHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based
 *handler group.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-08-06
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

#include "xController.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

    /******************************************************************************
     * Member function definitions
     ******************************************************************************/

    /** @brief Executes one online text-only conversation command. */
    ::ctrl::int32 XWalkController::XWALK_handlerOnlineLlmTest(const XWalkLifecycleRequest& request)
    {
        if (onlineLlmTestObject == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Online-LLM-test backend unavailable");
            return 3;
        }
        if (request.action == XWalkLifecycleAction::Stop)
        {
            onlineLlmTestObject->stop();
            XWALK_CTRL_TRACE_UID0(CTRL .078, "Online LLM test stopped");
            return 0;
        }
        return onlineLlmTestObject->run();
    }

} /* namespace xwalk::ctrl */
