/******************************************************************************
 * @file        xControllerVoiceChatHandler.cpp
 * @brief       Implements the VoiceChatHandler command responsibility.
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

    /**
     * @brief Executes the foreground local voice-chatbot command.
     * @param[in] request Validated lifecycle action.
     * @return Zero after cancellation or three when the chatbot is unavailable.
     */
    ::ctrl::int32 XWalkController::XWALK_handlerVoiceChat(const XWalkLifecycleRequest& request)
    {
        if (localVoiceChatbotObject == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Local voice-chatbot backend unavailable");
            return 3;
        }
        if (request.action == XWalkLifecycleAction::Stop)
        {
            localVoiceChatbotObject->stop();
            XWALK_CTRL_TRACE_UID0(CTRL .083, "Local voice chatbot stopped");
            return 0;
        }
        return localVoiceChatbotObject->run();
    }

} /* namespace xwalk::ctrl */
