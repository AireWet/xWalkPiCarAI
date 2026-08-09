/******************************************************************************
 * @file        xControllerHostApplication.cpp
 * @brief       Implements the device-free host Controller application.
 *
 * @details
 * Processes global options, installs process callbacks, and executes the shared
 * Controller runner through the device-free xWalkBoot host stub.
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

#include "xControllerAppConfig.h"
#include "xControllerApplicationSupport.h"
#include "xControllerCommands.h"
#include "xControllerRunner.h"

#include "xAgent_Rpi5CarBootHostStub.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include <iostream>

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
 * @brief Runs the device-free host Controller application.
 *
 * @param[in] argumentCount Number of process arguments including the executable name.
 * @param[in] arguments Non-owning process argument array.
 *
 * @return
 * Zero for generated help, two for invalid global options, or three when a
 * command requires unavailable hardware.
 *
 * @post No physical hardware backend has been constructed or accessed.
 */
::ctrl::int32 XWALK_runHostControllerApplication(
    ::ctrl::int32 argumentCount, ::ctrl::charpointer arguments[])
{
    XWalkControllerApplicationArguments applicationArguments;
    const XWalkAppConfig defaultConfig{{}, XWALK_RUNTIME_DATA_DIRECTORY};
    const ::ctrl::boolean argumentsParsed =
        XWALK_parseControllerApplicationArguments(
            argumentCount, arguments, defaultConfig, applicationArguments);
    if (argumentsParsed == false)
    {
        std::cerr << "Global options contain a missing or invalid value\n";
        return 2;
    }
    const ::ctrl::boolean traceConfigurationApplied =
        XWALK_applyTraceConfiguration(applicationArguments);
    if (traceConfigurationApplied == false)
    {
        std::cerr << "Trace configuration failed\n";
        return 2;
    }
    const ::ctrl::stringvector& commandArguments =
        applicationArguments.commandArguments;
    const ::ctrl::boolean traceConfigurationOnly =
        commandArguments.empty() &&
        (!applicationArguments.traceEnableUids.empty() ||
         !applicationArguments.traceDisableUids.empty());
    if (traceConfigurationOnly)
    {
        return 0;
    }
    const ::ctrl::boolean helpRequested =
        static_cast<::ctrl::boolean>(
            XWALK_isControllerHelpRequest(commandArguments));
    if (helpRequested)
    {
        std::cout << XWALK_controllerUsage() << '\n';
        return 0;
    }

    XWALK_resetOperationRequest();
    static_cast<void>(::signal(SIGINT, &XWALK_requestOperationStop));
    static_cast<void>(::signal(SIGTERM, &XWALK_requestOperationStop));
    XWalkControllerBootContext bootContext{
        &commandArguments, applicationArguments.appConfig.resourceDirectory};
    agent::XWalkBootServices hostServices{};
    agent::XWalkBootHostStub boot(hostServices);
    return boot.run(&bootContext, &XWALK_runController);
}

} /* namespace xwalk::ctrl */
