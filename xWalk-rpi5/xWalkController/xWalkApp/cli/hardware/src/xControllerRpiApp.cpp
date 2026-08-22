/******************************************************************************
 * @file        xControllerRpiApp.cpp
 * @brief       Implements modular Raspberry Pi application orchestration.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerRpi.h"

#include "xControllerAppConfig.h"
#include "xControllerApplicationSupport.h"
#include "xControllerCommands.h"
#include "xControllerDeploymentConfig.h"
#include "xControllerScheduledRunner.h"

#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarTrace.h"

#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::ctrl
{

    /**************************************************************************
     * Local function definitions
     **************************************************************************/

    /** @brief Selects one mutually exclusive application action. */
    static XWalkRpiAction selectAction(const XWalkControllerApplicationArguments& appArgs)
    {
        if (appArgs.validateConfiguration || appArgs.printEffectiveConfiguration || appArgs.diagnose ||
            appArgs.noHardware)
        {
            return XWALK_RPI_CONFIG_ACTION;
        }
        const ::ctrl::boolean commandArgumentsEmpty = appArgs.commandArguments.empty();
        const ::ctrl::boolean traceArgumentsEmpty = appArgs.traceArguments.empty();
        if (commandArgumentsEmpty && !traceArgumentsEmpty)
        {
            return XWALK_RPI_TRACE_ACTION;
        }
        const ::ctrl::boolean helpRequested = XWALK_isControllerHelpRequest(appArgs.commandArguments);
        if (helpRequested)
        {
            return XWALK_RPI_HELP_ACTION;
        }
        return XWALK_RPI_COMMAND_ACTION;
    }

    /** @brief Validates and reports one no-hardware configuration action. */
    static ::ctrl::int32 runConfig(const XWalkControllerApplicationArguments& appArgs)
    {
        const ::ctrl::boolean validAction =
            appArgs.commandArguments.empty() && (!appArgs.diagnose || appArgs.noHardware) &&
            (appArgs.validateConfiguration || appArgs.printEffectiveConfiguration || appArgs.diagnose);
        if (validAction == false)
        {
            std::cerr << "No-hardware configuration action is incomplete or conflicts with a command" << std::endl;
            return 2;
        }

        const XWalkDeploymentConfigReport report =
            XWALK_validateDeploymentConfig(appArgs.appConfig.configurationFilePath);
        for (const ::ctrl::string& line : report.lines)
        {
            std::cout << line << std::endl;
        }
        if (report.valid && appArgs.printEffectiveConfiguration)
        {
            for (const ::ctrl::string& line : XWALK_effectiveDeploymentConfig(appArgs.appConfig.configurationFilePath))
            {
                std::cout << line << std::endl;
            }
        }
        return report.valid ? 0 : 2;
    }

    /** @brief Validates command prerequisites and submits one scheduler request. */
    static ::ctrl::int32 runCommand(XWalkControllerApplicationArguments* appArgs)
    {
        const ::ctrl::boolean appArgsMissing = appArgs == nullptr;
        const ::ctrl::boolean configurationReadable =
            !appArgsMissing && hal::isReadableRegularFile(appArgs->appConfig.configurationFilePath);
        if (appArgsMissing || !configurationReadable)
        {
            if (appArgs != nullptr)
            {
                std::cerr << "Unreadable deployment configuration: " << appArgs->appConfig.configurationFilePath
                          << std::endl;
            }
            return 2;
        }

        XWALK_resetOperationRequest();
        const ::ctrl::boolean signalHandlingPrepared = XWALK_prepareOperationSignalHandling();
        if (signalHandlingPrepared == false)
        {
            std::cerr << "Could not prepare graceful cancellation handling" << std::endl;
            return 2;
        }

        XWalkRunArgs runArgs{
            &appArgs->commandArguments, appArgs->appConfig.resourceDirectory, appArgs->appConfig.configurationFilePath};
        return XWALK_runRpi(&runArgs);
    }

    /**************************************************************************
     * Public function definitions
     **************************************************************************/

    ::ctrl::int32 xWalkRunRpiApplication(::ctrl::int32 argumentCount, ::ctrl::charpointer arguments[])
    {
        XWalkControllerApplicationArguments appArgs;
        const XWalkAppConfig defaults{XWALK_PICARX_CONFIG_FILE, XWALK_RUNTIME_DATA_DIRECTORY};
        const ::ctrl::boolean argumentsParsed =
            xWalkParseControllerApplicationArguments(argumentCount, arguments, defaults, appArgs);
        if (argumentsParsed == false)
        {
            std::cerr << "Global options contain a missing or invalid value" << std::endl;
            return 2;
        }

        const XWalkRpiAction action = selectAction(appArgs);
        if (action == XWALK_RPI_CONFIG_ACTION)
        {
            return runConfig(appArgs);
        }
        const ::ctrl::boolean traceConfigurationApplied = xWalkApplyTraceConfiguration(appArgs);
        if (traceConfigurationApplied == false)
        {
            std::cerr << "Trace configuration failed: " << hal::XWalkTrace::globalTraceConfigurationError()
                      << std::endl;
            return 2;
        }

        switch (action)
        {
            case XWALK_RPI_TRACE_ACTION:
                return 0;
            case XWALK_RPI_HELP_ACTION:
                std::cout << XWALK_controllerUsage() << std::endl;
                return 0;
            case XWALK_RPI_COMMAND_ACTION:
                return runCommand(&appArgs);
            case XWALK_RPI_CONFIG_ACTION:
            default:
                return 2;
        }
    }

} /* namespace xwalk::ctrl */
