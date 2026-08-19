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
#include "xControllerDeploymentConfig.h"
#include "xControllerRunner.h"
#include "xAgent_Rpi5CarBootRpi.h"
#include "xAgent_Rpi5CarPicarxConfiguration.h"

#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"
#include "xHal_Rpi5CarTrace.h"

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
    const xwalk::ctrl::XWalkAppConfig defaultConfig{XWALK_PICARX_CONFIG_FILE, XWALK_RUNTIME_DATA_DIRECTORY};

    const ctrl::boolean argumentsParsed = xwalk::ctrl::xWalkParseControllerApplicationArguments(
        argumentCount, arguments, defaultConfig, applicationArguments);
    if (argumentsParsed == false)
    {
        std::cerr << "Global options contain a missing or invalid value" << std::endl;
        return 2;
    }

    const ::ctrl::boolean configurationActionRequested =
        applicationArguments.validateConfiguration || applicationArguments.printEffectiveConfiguration ||
        applicationArguments.diagnose || applicationArguments.noHardware;
    if (configurationActionRequested)
    {
        const ::ctrl::boolean actionValid =
            applicationArguments.commandArguments.empty() &&
            (!applicationArguments.diagnose || applicationArguments.noHardware) &&
            (applicationArguments.validateConfiguration || applicationArguments.printEffectiveConfiguration ||
             applicationArguments.diagnose);
        if (actionValid == false)
        {
            std::cerr << "No-hardware configuration action is incomplete or conflicts with a command" << std::endl;
            return 2;
        }
        const xwalk::ctrl::XWalkDeploymentConfigReport report =
            xwalk::ctrl::XWALK_validateDeploymentConfig(applicationArguments.appConfig.configurationFilePath);
        for (const ctrl::string& line : report.lines)
        {
            std::cout << line << std::endl;
        }
        if (report.valid && applicationArguments.printEffectiveConfiguration)
        {
            for (const ctrl::string& line :
                 xwalk::ctrl::XWALK_effectiveDeploymentConfig(applicationArguments.appConfig.configurationFilePath))
            {
                std::cout << line << std::endl;
            }
        }
        return report.valid ? 0 : 2;
    }

    const ::ctrl::boolean traceConfigurationApplied = xwalk::ctrl::xWalkApplyTraceConfiguration(applicationArguments);
    if (traceConfigurationApplied == false)
    {
        std::cerr << "Trace configuration failed: " << xwalk::hal::XWalkTrace::globalTraceConfigurationError()
                  << std::endl;
        return 2;
    }

    const ctrl::stringvector& commandArguments = applicationArguments.commandArguments;
    const ::ctrl::boolean traceConfigurationOnly =
        commandArguments.empty() && !applicationArguments.traceArguments.empty();
    if (traceConfigurationOnly)
    {
        return 0;
    }
    const ::ctrl::boolean helpRequested =
        static_cast<::ctrl::boolean>(xwalk::ctrl::XWALK_isControllerHelpRequest(commandArguments));

    if (helpRequested)
    {
        std::cout << xwalk::ctrl::XWALK_controllerUsage() << std::endl;
        return 0;
    }

    const ::ctrl::boolean readableRegularFileNotMatched = static_cast<::ctrl::boolean>(
        !xwalk::hal::isReadableRegularFile(applicationArguments.appConfig.configurationFilePath));

    if (readableRegularFileNotMatched)
    {
        std::cerr << "Unreadable deployment configuration: " << applicationArguments.appConfig.configurationFilePath
                  << std::endl;
        return 2;
    }

    xwalk::ctrl::XWALK_resetOperationRequest();
    const ctrl::boolean signalHandlingPrepared = xwalk::ctrl::XWALK_prepareOperationSignalHandling();
    if (signalHandlingPrepared == false)
    {
        std::cerr << "Could not prepare graceful cancellation handling" << std::endl;
        return 2;
    }

    xwalk::ctrl::XWalkControllerBootContext bootContext{&commandArguments,
                                                        applicationArguments.appConfig.resourceDirectory};

    xwalk::agent::XWalkBootRpi boot(xwalk::ctrl::XWALK_selectBootMode(commandArguments),
                                    applicationArguments.appConfig.configurationFilePath);

    return boot.run(&bootContext, &xwalk::ctrl::XWALK_runController);
}
