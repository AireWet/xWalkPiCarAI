/******************************************************************************
 * @file        xControllerVoiceControlledCarHandler.cpp
 * @brief       Implements the VoiceControlledCarHandler command responsibility.
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

    /** @brief Executes one wake-word voice-controlled-car command. */
    ::ctrl::int32 XWalkController::XWALK_handlerVoiceControlledCar(const XWalkLifecycleRequest& request)
    {
        if (voiceControlledCarObject == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Voice-controlled-car backend unavailable");
            return 3;
        }
        if (request.action == XWalkLifecycleAction::Stop)
        {
            voiceControlledCarObject->stop();
            XWALK_CTRL_TRACE_UID0(CTRL .084, "Voice-controlled car stopped");
            return 0;
        }
        return voiceControlledCarObject->run();
    }

} /* namespace xwalk::ctrl */
