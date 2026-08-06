/******************************************************************************
 * @file        xControllerKeyboardControlHandler.cpp
 * @brief       Implements the KeyboardControlHandler command responsibility.
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
 * @brief Runs interactive keyboard control ported from `3.keyboard_control.py`.
 * @param[in] request Validated empty request.
 * @return Zero after explicit exit or cancellation; three when the Agent is unavailable.
 * @warning Movement keys drive the physical car at 80-percent requested speed.
 */
::ctrl::int32 XWalkController::XWALK_handlerKeyboardControl(
    const XWalkNoArgumentRequest& request)
{
    static_cast<void>(request);
    if (keyboardControlObject == nullptr)
    {
        output("Keyboard-control backend unavailable.");
        return 3;
    }

    output("Keyboard controls: w/s forward/backward, a/d left/right, "
        "i/k tilt, j/l pan, q quit.");
    while (true)
    {
        const ::ctrl::string keyText = input("keyboard> ");
        if ((keyText == "q") || (keyText == "Q") || (keyText == "quit") ||
            (keyText == "exit") || (keyText == "skip"))
        {
            break;
        }
        const agent::XWalkKeyboardControlResult result = keyboardControlObject->handleKey(keyText);
        if (result == agent::XWalkKeyboardControlResult::Cancelled)
        {
            break;
        }
        if (result == agent::XWalkKeyboardControlResult::Ignored)
        {
            output("Ignored key; use w, s, a, d, i, k, j, l, or q.");
        }
    }
    keyboardControlObject->finish();
    output("Keyboard control stopped.");
    return 0;
}

} /* namespace xwalk::ctrl */
