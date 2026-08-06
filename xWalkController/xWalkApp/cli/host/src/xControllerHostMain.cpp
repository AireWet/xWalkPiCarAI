/******************************************************************************
 * @file        xControllerHostMain.cpp
 * @brief       Provides the device-free host CLI entry point.
 *
 * @details
 * Executes the shared Controller application lifecycle through portable and
 * host-stub module implementations without claiming physical hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-08-02
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
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the shared Controller lifecycle with host-only module code.
 * @param[in] argumentCount Number of process arguments including the executable name.
 * @param[in] arguments Non-owning process argument array.
 * @return Controller application status returned by the host-only lifecycle.
 */
ctrl::int32 main(ctrl::int32 argumentCount, ctrl::charpointer arguments[])
{
    return xwalk::ctrl::XWALK_runHostControllerApplication(
        argumentCount, arguments);
}
