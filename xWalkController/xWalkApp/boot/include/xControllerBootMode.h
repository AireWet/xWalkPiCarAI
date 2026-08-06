/******************************************************************************
 * @file        xControllerBootMode.h
 * @brief       Declares Controller command-to-boot-mode selection.
 *
 * @details
 * Exposes the application boundary that selects the minimum Agent boot graph
 * required for one parsed Controller command.
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

#ifndef XCONTROLLER_BOOT_MODE_H
#define XCONTROLLER_BOOT_MODE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBootTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller application boot selection for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Function declarations
 ******************************************************************************/

/**
 * @brief Selects the minimum boot graph required by one parsed command group.
 * @param[in] commandArguments Complete command arguments excluding the executable name.
 * @return Command-specific Agent boot mode, or Base when no specialized service is required.
 */
agent::uint8 XWALK_selectBootMode(
    const ::ctrl::stringvector& commandArguments) noexcept;

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_BOOT_MODE_H */
