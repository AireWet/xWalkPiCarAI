/******************************************************************************
 * @file        xHal_Rpi5CarUtilsHardwareTest.cpp
 * @brief       Verifies xWalk utilities on Raspberry Pi Linux.
 *
 * @details
 * Composes the Linux backend, checks essential platform services, prints a
 * diagnostic record, and applies a conservative PCM playback volume.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils Hardware Test
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarUtilsLinux.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Exercises the Linux utility backend on an approved Raspberry Pi.
 *
 * @return
 * Zero when the shell, loopback network, username, and mixer operations succeed;
 * otherwise the process fails or returns one.
 *
 * @warning
 * Running this function writes to the terminal and changes PCM playback volume
 * to fifty percent. Run it only after target safety approval.
 */
int main()
{
    xwalk::hal::XWalkUtilsLinux backend;
    xwalk::hal::XWalkUtils utilities(&backend, backend.utilityCallbacks());

    utilities.info("xWalkUtils Linux hardware test");
    const xwalk::hal::XWalkCommandResult commandResult =
        utilities.runCommand("printf 'xwalk-utils-ready'");
    const hal::boolean commandResultStatusOutputInvalid =
        static_cast<hal::boolean>(
            (commandResult.status != 0) || (commandResult.output != "xwalk-utils-ready") ||
        (!utilities.commandExists("amixer")) || (utilities.ipAddress("lo") != "127.0.0.1") ||
        utilities.username().empty());
    if (commandResultStatusOutputInvalid)
    {
        return 1;
    }

    utilities.setVolume(50);
    return 0;
}
