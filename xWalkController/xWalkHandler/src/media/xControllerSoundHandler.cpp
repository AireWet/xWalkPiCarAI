/******************************************************************************
 * @file        xControllerSoundHandler.cpp
 * @brief       Implements the SoundHandler command responsibility.
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
 * @brief Executes the sound command.
 * @param[in] request Validated sound action, file path, and optional volume.
 * @return Zero when accepted or three when the platform audio backend is unavailable.
 */
::ctrl::int32 XWalkController::XWALK_handlerSound(const XWalkSoundRequest& request)
{
    const ::ctrl::boolean soundPerformed = callbacks.sound(callbackContext, request);
    if (soundPerformed == false)
    {
        output("Sound backend unavailable");
        return 3;
    }
    return 0;
}

} /* namespace xwalk::ctrl */
