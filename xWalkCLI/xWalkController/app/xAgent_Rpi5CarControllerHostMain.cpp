/******************************************************************************
 * @file        xAgent_Rpi5CarControllerHostMain.cpp
 * @brief       Provides the device-free host CLI entry point.
 *
 * @details
 * Supports help and release-layout checks without claiming physical hardware.
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

#include "xAgent_Rpi5CarController.h"

#include "xHal_Rpi5CarFileFunctions.h"

#include <iostream>

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Prints help without constructing a hardware backend.
 * @param[in] argumentCount Number of process arguments including the executable name.
 * @param[in] arguments Non-owning process argument array.
 * @return Zero for help or three when a hardware command is requested from the host-only binary.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    XWalkHal::int32 commandIndex{1};
    while (commandIndex < argumentCount)
    {
        const XWalkHal::stringview option(arguments[commandIndex]);
        XWalkHal::stringview value;
        if ((option == "--deployment-config") || (option == "--resource-directory"))
        {
            if ((commandIndex + 1) >= argumentCount)
            {
                std::cerr << "Global options require absolute non-empty paths\n";
                return 2;
            }
            value = arguments[commandIndex + 1];
            commandIndex += 2;
        }
        else if ((option.rfind("--deployment-config=", 0U) == 0U) ||
            (option.rfind("--resource-directory=", 0U) == 0U))
        {
            const XWalkHal::size separator = option.find('=');
            value = option.substr(separator + 1U);
            ++commandIndex;
        }
        else
        {
            break;
        }
        if (value.empty() || !XWalkHal::filesystempath(value).is_absolute())
        {
            std::cerr << "Global options require absolute non-empty paths\n";
            return 2;
        }
    }
    if ((commandIndex >= argumentCount) || ((commandIndex + 1) == argumentCount &&
        ((XWalkHal::stringview(arguments[commandIndex]) == "--help") ||
         (XWalkHal::stringview(arguments[commandIndex]) == "-h") ||
         (XWalkHal::stringview(arguments[commandIndex]) == "help"))))
    {
        std::cout << xwalk::agent::XWalkController::usage() << '\n';
        return 0;
    }
    std::cerr << "Hardware backend unavailable in the host verification executable\n";
    return 3;
}
