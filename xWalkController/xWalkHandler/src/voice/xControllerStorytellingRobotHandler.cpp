/******************************************************************************
 * @file        xControllerStorytellingRobotHandler.cpp
 * @brief       Implements the StorytellingRobotHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based handler group.
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

#include "xHal_Rpi5CarExceptions.h"

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

/** @brief Executes one Piper storytelling movement demonstration. */
::ctrl::int32 XWalkController::XWALK_handlerStorytellingRobot(
    const XWalkLifecycleRequest& request)
{
    if (storytellingRobotObject == nullptr)
    {
        output("Storytelling-robot backend unavailable");
        return 3;
    }
    if (request.action == XWalkLifecycleAction::Stop)
    {
        storytellingRobotObject->stop();
        output("Storytelling robot stopped");
        return 0;
    }
    const ::ctrl::int32 status = storytellingRobotObject->run();
    output("Storytelling robot demonstration stopped");
    return status;
}

} /* namespace xwalk::ctrl */
