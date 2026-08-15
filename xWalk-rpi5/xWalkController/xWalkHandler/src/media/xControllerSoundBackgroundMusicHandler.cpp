/******************************************************************************
 * @file        xControllerSoundBackgroundMusicHandler.cpp
 * @brief       Implements the SoundBackgroundMusicHandler command
 *responsibility.
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

    ::ctrl::int32 XWalkController::XWALK_handlerSoundBackgroundMusic(const XWalkNoArgumentRequest& request)
    {
        static_cast<void>(request);
        if (soundBackgroundMusicObject == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Sound-background-music backend unavailable");
            return 3;
        }
        const ::ctrl::boolean started = soundBackgroundMusicObject->start();
        if (started == false)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Sound-background-music resources could not be opened");
            return 2;
        }

        XWALK_CTRL_TRACE_UID0(CTRL .022,
                              "Sound/music keys: space horn; c background horn; "
                              "q play/stop music; x exit.");
        const ::ctrl::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const ::ctrl::boolean operationAllowed = static_cast<::ctrl::boolean>(operationMayContinue());
            if (operationAllowed == false)
            {
                break;
            }
            const ::ctrl::string key = input("sound-music> ");
            if ((key == "x") || (key == "X") || (key == "exit") || (key == "quit") || (key == "skip"))
            {
                break;
            }
            const ::ctrl::boolean soundCommandHandled = static_cast<::ctrl::boolean>(
                soundBackgroundMusicObject->handleKey(key).event == agent::XWalkSoundBackgroundMusicEvent::Cancelled);
            if (soundCommandHandled)
            {
                break;
            }
        }
        soundBackgroundMusicObject->finish();
        XWALK_CTRL_TRACE_UID0(CTRL .023, "Sound and music stopped");
        return 0;
    }

} /* namespace xwalk::ctrl */
