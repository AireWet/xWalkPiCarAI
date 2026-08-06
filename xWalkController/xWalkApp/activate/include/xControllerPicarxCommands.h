/******************************************************************************
 * @file        xControllerPicarxCommands.h
 * @brief       Declares PiCar-X Controller application command dispatch.
 *
 * @details
 * Provides one application-owned free function that routes a validated command
 * to the protected handler responsible for that command group.
 *
 * @project     xWalk Firmware
 * @module      xWalkApp
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

#ifndef XCONTROLLER_PICARX_COMMANDS_H
#define XCONTROLLER_PICARX_COMMANDS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xController.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Function declarations
 ******************************************************************************/

/**
 * @brief Routes one PiCar-X command to its protected Controller handler.
 *
 * @param[in,out] controller Controller whose injected dependencies remain valid
 * for the complete call.
 * @param[in] request Parsed command and validated non-empty arguments.
 *
 * @return
 * Zero on success or three when a command-specific backend is unavailable.
 *
 * @throws std::invalid_argument
 * The argument list is empty or the selected command group is not supported.
 */
::ctrl::int32 XWALK_runPicarxControllerCommand(XWalkController& controller,
    const XWalkControllerCommandRequest& request);

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_PICARX_COMMANDS_H */
