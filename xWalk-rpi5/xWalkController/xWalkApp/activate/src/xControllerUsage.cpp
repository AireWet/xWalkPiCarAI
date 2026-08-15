/******************************************************************************
 * @file        xControllerUsage.cpp
 * @brief       Implements generated Controller usage access.
 *
 * @details
 * Returns the build-selected Linux-style command help as owned text.
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

#include "xControllerHelp.h"

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
     * @brief Returns the generated Linux-style command help.
     *
     * @return
     * Owned multi-line help text describing commands, options, and examples.
     */
    ::ctrl::string XWALK_controllerUsage()
    {
        return XCONTROLLER_HELP;
    }

} /* namespace xwalk::ctrl */
