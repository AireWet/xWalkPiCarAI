/******************************************************************************
 * @file        xControllerMoveExampleHandler.cpp
 * @brief       Implements the MoveExampleHandler command responsibility.
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

/**
 * @brief Runs the bounded movement sequence ported from `2.move.py`.
 * @return Zero after completion or cancellation with the motors stopped.
 * @warning Moves both drive motors, steering, and camera servos.
 */
::ctrl::int32 XWalkController::XWALK_handlerMoveExample()
{
    if (moveExampleObject == nullptr)
    {
        output("Move example backend unavailable.");
        return 3;
    }
    const ::ctrl::boolean completed = moveExampleObject->run();
    if (completed == false)
    {
        output("Move example incomplete.");
        return 2;
    }
    output("Move example complete!");
    return 0;
}

} /* namespace xwalk::ctrl */
