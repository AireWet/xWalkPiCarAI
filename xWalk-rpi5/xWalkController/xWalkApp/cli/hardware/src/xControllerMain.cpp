/******************************************************************************
 * @file        xControllerMain.cpp
 * @brief       Provides the Raspberry Pi PiCar-X CLI entry point.
 *
 * @details
 * Configures the CMake-selected GStreamer plugin directory, parses process
 * arguments, selects one xWalkBoot mode, and runs the CLI through services
 * owned for the complete command lifetime.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-07-31
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

#include "xControllerRpi.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Configures the process runtime and performs one guarded RPi backend boot.
 * @details Runs the CMake-selected platform setup before delegating application flow to its modular runner.
 * @param[in] argumentCount Number of process arguments including the executable name.
 * @param[in] arguments Non-owning process argument array.
 * @return CLI or help status after successful completion.
 * @warning Non-help commands may claim physical I2C, GPIO, motor, servo, and audio resources.
 */
ctrl::int32 main(ctrl::int32 argumentCount, ctrl::charpointer arguments[])
{
    const ctrl::int32 platformResult = xwalk::ctrl::XWALK_initRpi();
    if (platformResult != 0)
    {
        return platformResult;
    }
    return xwalk::ctrl::xWalkRunRpiApplication(argumentCount, arguments);
}
