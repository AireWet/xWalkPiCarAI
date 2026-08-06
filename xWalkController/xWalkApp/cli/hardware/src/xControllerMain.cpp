/******************************************************************************
 * @file        xControllerMain.cpp
 * @brief       Provides the Raspberry Pi PiCar-X CLI entry point.
 *
 * @details
 * Parses process arguments, selects one xWalkBoot mode, and runs the CLI
 * through services owned for the complete command lifetime.
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

#include "xControllerAppConfig.h"
#include "xControllerApplicationSupport.h"
#include "xControllerBootMode.h"
#include "xControllerCommands.h"
#include "xControllerRunner.h"
#include "xAgent_Rpi5CarBootRpi.h"
#include "xAgent_Rpi5CarPicarxConfiguration.h"

#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

#include <iostream>

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Parses process arguments and performs one guarded RPi backend boot.
 * @param[in] argumentCount Number of process arguments including the executable name.
 * @param[in] arguments Non-owning process argument array.
 * @return CLI or help status after successful completion.
 * @warning Non-help commands may claim physical I2C, GPIO, motor, servo, and audio resources.
 */
ctrl::int32 main(ctrl::int32 argumentCount, ctrl::charpointer arguments[])
{
    xwalk::ctrl::XWalkControllerApplicationArguments applicationArguments;
    const xwalk::ctrl::XWalkAppConfig defaultConfig{
                                    XWALK_PICARX_CONFIG_FILE,
                                    XWALK_RUNTIME_DATA_DIRECTORY
                                    };

    const ctrl::boolean argumentsParsed =
            xwalk::ctrl::XWALK_parseControllerApplicationArguments(
                                                            argumentCount,
                                                            arguments,
                                                            defaultConfig,
                                                            applicationArguments
                                                            );
    if (argumentsParsed == false)
    {
        std::cerr << "Global options require absolute non-empty paths" << std::endl;
        return 2;
    }

    const ctrl::stringvector& commandArguments = applicationArguments.commandArguments;
    const ::ctrl::boolean helpRequested =
        static_cast<::ctrl::boolean>(xwalk::ctrl::XWALK_isControllerHelpRequest(commandArguments));

    if (helpRequested)
    {
        std::cout << xwalk::ctrl::XWALK_controllerUsage() << std::endl;
        return 0;
    }

    const ::ctrl::boolean readableRegularFileNotMatched =
        static_cast<::ctrl::boolean>( !xwalk::hal::isReadableRegularFile(
        applicationArguments.appConfig.configurationFilePath
        ));

    if (readableRegularFileNotMatched)
    {
        std::cerr << "Unreadable deployment configuration: "
                  << applicationArguments.appConfig.configurationFilePath
                  << std::endl;
        return 2;
    }

    xwalk::ctrl::XWALK_resetOperationRequest();
    static_cast<void>(::signal(SIGINT, &xwalk::ctrl::XWALK_requestOperationStop));
    static_cast<void>(::signal(SIGTERM, &xwalk::ctrl::XWALK_requestOperationStop));

    xwalk::ctrl::XWalkControllerBootContext bootContext{
        &commandArguments,
        applicationArguments.appConfig.resourceDirectory
        };

    xwalk::agent::XWalkBootRpi boot(
        xwalk::ctrl::XWALK_selectBootMode(commandArguments),
        applicationArguments.appConfig.configurationFilePath
        );

    return boot.run(&bootContext, &xwalk::ctrl::XWALK_runController);
}
