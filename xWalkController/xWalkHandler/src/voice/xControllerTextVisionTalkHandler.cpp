/******************************************************************************
 * @file        xControllerTextVisionTalkHandler.cpp
 * @brief       Implements the TextVisionTalkHandler command responsibility.
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

/** @brief Executes one image-grounded typed conversation command. */
::ctrl::int32 XWalkController::XWALK_handlerTextVisionTalk(
    const XWalkLifecycleRequest& request)
{
    if (textVisionTalkObject == nullptr)
    {
        output("Text-vision-talk backend unavailable");
        return 3;
    }
    if (request.action == XWalkLifecycleAction::Stop)
    {
        textVisionTalkObject->stop();
        output("Text vision talk stopped");
        return 0;
    }
    return textVisionTalkObject->run();
}

} /* namespace xwalk::ctrl */
