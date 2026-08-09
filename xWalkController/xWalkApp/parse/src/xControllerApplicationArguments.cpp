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
 * Anonymous namespace
 ******************************************************************************/

namespace
{

/** @brief Validates one CLI trace UID without constructing the trace runtime. */
::ctrl::boolean validTraceUid(::ctrl::stringview uid) noexcept
{
    const ::ctrl::size separator = uid.find('.');
    if ((separator == ::ctrl::stringview::npos) || (separator == 0U))
    {
        return false;
    }
    const ::ctrl::stringview tag = uid.substr(0U, separator);
    if ((tag != "RPI") && (tag != "CTRL"))
    {
        return false;
    }
    const ::ctrl::stringview number = uid.substr(separator + 1U);
    if (number.empty())
    {
        return false;
    }
    for (const char digit : number)
    {
        if ((digit < '0') || (digit > '9'))
        {
            return false;
        }
    }
    return true;
}

} /* namespace */

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
        const ::ctrl::string option = commandArguments[0U];
        const ::ctrl::fixedarray<::ctrl::stringview, 4U> optionNames{{
            "--deployment-config", "--resource-directory",
            "--trace-enable", "--trace-disable"}};
        ::ctrl::stringview matchedOption;
        ::ctrl::string value;
        ::ctrl::size consumed{};
        for (const ::ctrl::stringview optionName : optionNames)
        {
            if (option == optionName)
            {
                if (commandArguments.size() < 2U)
                {
                    return false;
                }
                matchedOption = optionName;
                value = commandArguments[1U];
                consumed = 2U;
                break;
            }
            const ::ctrl::string assignmentPrefix = ::ctrl::string(optionName) + "=";
            if (option.rfind(assignmentPrefix, 0U) == 0U)
            {
                matchedOption = optionName;
                value = option.substr(assignmentPrefix.size());
                consumed = 1U;
                break;
            }
        }
        if (matchedOption.empty())
        {
            break;
        }
        if (value.empty())
        {
            return false;
        }
        const ::ctrl::boolean traceOption =
            (matchedOption == "--trace-enable") ||
            (matchedOption == "--trace-disable");
        if (traceOption && (validTraceUid(value) == false))
        {
            return false;
        }
        if ((traceOption == false) &&
            (::ctrl::filesystempath(value).is_absolute() == false))
        {
            return false;
        }
        if (matchedOption == "--deployment-config")
        {
            applicationArguments.appConfig.configurationFilePath = value;
        }
        else if (matchedOption == "--resource-directory")
        {
            applicationArguments.appConfig.resourceDirectory = value;
        }
        else if (matchedOption == "--trace-enable")
        {
            applicationArguments.traceEnableUids.push_back(value);
        }
        else
        {
            applicationArguments.traceDisableUids.push_back(value);
        }
        commandArguments.erase(commandArguments.begin(), commandArguments.begin() +
            static_cast<::ctrl::stringvector::difference_type>(consumed));
    }
    return true;
}

} /* namespace xwalk::ctrl */
