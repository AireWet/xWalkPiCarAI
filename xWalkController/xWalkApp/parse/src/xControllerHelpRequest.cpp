/******************************************************************************
 * @file        xControllerHelpRequest.cpp
 * @brief       Implements Controller help-request classification.
 *
 * @details
 * Recognizes the exact standalone help spellings accepted before hardware composition.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
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

#include "xControllerCommands.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/**
 * @brief Reports whether one parsed command requests generated help.
 *
 * @param[in] arguments Command arguments after global-option parsing.
 *
 * @return
 * `true` only for one `help`, `-h`, or `--help` argument.
 */
::ctrl::boolean XWALK_isControllerHelpRequest(
    const ::ctrl::stringvector& arguments) noexcept
{
    return (arguments.size() == 1U) &&
        ((arguments[0U] == "-h") || (arguments[0U] == "--help") ||
         (arguments[0U] == "help"));
}

} /* namespace xwalk::ctrl */
