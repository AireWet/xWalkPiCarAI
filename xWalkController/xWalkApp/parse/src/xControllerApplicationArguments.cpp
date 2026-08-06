/******************************************************************************
 * @file        xControllerApplicationArguments.cpp
 * @brief       Implements Controller process-argument parsing.
 *
 * @details
 * Separates application-global paths from command arguments for host and hardware entry points.
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
 * @brief Parses process arguments shared by host and hardware applications.
 *
 * @param[in] argumentCount Number of process arguments including the executable name.
 * @param[in] arguments Non-owning process argument array.
 * @param[in] defaultConfig Default deployment and packaged-resource paths.
 * @param[out] applicationArguments Validated paths and remaining command arguments.
 *
 * @return
 * `true` when every application-global option is complete, non-empty, and
 * absolute; otherwise `false`.
 */
::ctrl::boolean XWALK_parseControllerApplicationArguments(
                                    ::ctrl::int32 argumentCount, ::ctrl::charpointer arguments[],
                                    const XWalkAppConfig& defaultConfig,
                                    XWalkControllerApplicationArguments& applicationArguments
                                    )
{
    applicationArguments = {};
    applicationArguments.appConfig = defaultConfig;
    for (::ctrl::int32 index = 1; index < argumentCount; ++index)
    {
        applicationArguments.commandArguments.emplace_back(arguments[index]);
    }

    ::ctrl::stringvector& commandArguments = applicationArguments.commandArguments;
    const ::ctrl::boolean applicationOptionParsingRequested{true};
    while (applicationOptionParsingRequested)
    {
        const ::ctrl::boolean commandArgumentsAvailable =
            static_cast<::ctrl::boolean>(
                !commandArguments.empty());
        if (commandArgumentsAvailable == false)
        {
            break;
        }
        ::ctrl::size consumed{};
        ::ctrl::string value;
        if ((commandArguments[0U] == "--deployment-config") ||
            (commandArguments[0U] == "--resource-directory"))
        {
            const ::ctrl::boolean optionValueMissing =
                static_cast<::ctrl::boolean>(
                    commandArguments.size() < 2U);
            if (optionValueMissing)
            {
                return false;
            }
            value = commandArguments[1U];
            consumed = 2U;
        }
        else
        {
            const ::ctrl::boolean deploymentConfigAssignmentMatched =
                static_cast<::ctrl::boolean>(
                    commandArguments[0U].rfind("--deployment-config=", 0U) == 0U);
            if (deploymentConfigAssignmentMatched)
            {
                value = commandArguments[0U].substr(20U);
                consumed = 1U;
            }
            else
            {
                const ::ctrl::boolean resourceDirectoryAssignmentMatched =
                    static_cast<::ctrl::boolean>(
                        commandArguments[0U].rfind("--resource-directory=", 0U) == 0U);
                if (resourceDirectoryAssignmentMatched)
                {
                    value = commandArguments[0U].substr(21U);
                    consumed = 1U;
                }
                else
                {
                    break;
                }
            }
        }
        const ::ctrl::boolean optionPathInvalid =
            static_cast<::ctrl::boolean>(
                value.empty() || !::ctrl::filesystempath(value).is_absolute());
        if (optionPathInvalid)
        {
            return false;
        }
        const ::ctrl::boolean deploymentConfigOptionMatched =
            static_cast<::ctrl::boolean>(
                commandArguments[0U].rfind("--deployment-config", 0U) == 0U);
        if (deploymentConfigOptionMatched)
        {
            applicationArguments.appConfig.configurationFilePath = value;
        }
        else
        {
            applicationArguments.appConfig.resourceDirectory = value;
        }
        commandArguments.erase(commandArguments.begin(), commandArguments.begin() +
            static_cast<::ctrl::stringvector::difference_type>(consumed));
    }
    return true;
}

} /* namespace xwalk::ctrl */
