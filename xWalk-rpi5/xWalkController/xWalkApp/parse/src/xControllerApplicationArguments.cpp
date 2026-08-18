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
        if ((tag != "RPI") && (tag != "CTRL") && (tag != "RPIAGENT") && (tag != "LIB"))
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

    /** @brief Validates one canonical trace module token. */
    ::ctrl::boolean validTraceModule(::ctrl::stringview module) noexcept
    {
        if (module.empty() || (module[0U] < 'A') || (module[0U] > 'Z'))
        {
            return false;
        }
        for (const char character : module)
        {
            const ::ctrl::boolean validCharacter = ((character >= 'A') && (character <= 'Z')) ||
                                                   ((character >= '0') && (character <= '9')) || (character == '_');
            if (validCharacter == false)
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
     * `true` when every application-global option is complete, path values are
     * non-empty, and trace selectors are valid; otherwise `false`.
     */
    ::ctrl::boolean xWalkParseControllerApplicationArguments(::ctrl::int32 argumentCount,
                                                             ::ctrl::charpointer arguments[],
                                                             const XWalkAppConfig& defaultConfig,
                                                             XWalkControllerApplicationArguments& applicationArguments)
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
            const ::ctrl::boolean commandArgumentsAvailable = static_cast<::ctrl::boolean>(!commandArguments.empty());
            if (commandArgumentsAvailable == false)
            {
                break;
            }
            const ::ctrl::string option = commandArguments[0U];
            const ::ctrl::boolean flagOption = (option == "--validate-config") ||
                                               (option == "--print-effective-config") || (option == "--diagnose") ||
                                               (option == "--no-hardware");
            if (flagOption)
            {
                applicationArguments.validateConfiguration =
                    applicationArguments.validateConfiguration || (option == "--validate-config");
                applicationArguments.printEffectiveConfiguration =
                    applicationArguments.printEffectiveConfiguration || (option == "--print-effective-config");
                applicationArguments.diagnose = applicationArguments.diagnose || (option == "--diagnose");
                applicationArguments.noHardware = applicationArguments.noHardware || (option == "--no-hardware");
                commandArguments.erase(commandArguments.begin());
                continue;
            }
            const ::ctrl::fixedarray<::ctrl::stringview, 5U> optionNames{
                {"--deployment-config", "--resource-directory", "--trace", "--trace-enable", "--trace-disable"}};
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
            const ::ctrl::boolean legacyTraceOption =
                (matchedOption == "--trace-enable") || (matchedOption == "--trace-disable");
            const ::ctrl::boolean unifiedTraceOption = matchedOption == "--trace";
            const ::ctrl::boolean traceOption = legacyTraceOption || unifiedTraceOption;
            ::ctrl::string traceTarget = value;
            ::ctrl::boolean traceEnabled = matchedOption == "--trace-enable";
            const ::ctrl::boolean jsonTraceOption =
                unifiedTraceOption && (value.size() > 5U) && (value.compare(value.size() - 5U, 5U, ".json") == 0);
            if (unifiedTraceOption && (jsonTraceOption == false))
            {
                const ::ctrl::stringview enableSuffix(".enable");
                const ::ctrl::stringview disableSuffix(".disable");
                const ::ctrl::boolean enableRequested =
                    value.size() > enableSuffix.size() &&
                    value.compare(value.size() - enableSuffix.size(), enableSuffix.size(), enableSuffix) == 0;
                const ::ctrl::boolean disableRequested =
                    value.size() > disableSuffix.size() &&
                    value.compare(value.size() - disableSuffix.size(), disableSuffix.size(), disableSuffix) == 0;
                const ::ctrl::boolean operationValid = enableRequested || disableRequested;
                if (operationValid == false)
                {
                    return false;
                }
                const ::ctrl::size suffixLength = enableRequested ? enableSuffix.size() : disableSuffix.size();
                traceTarget = value.substr(0U, value.size() - suffixLength);
                traceEnabled = enableRequested;
            }
            const ::ctrl::boolean allTracesSelected = traceTarget == "all";
            const ::ctrl::boolean moduleSelected = validTraceModule(traceTarget);
            const ::ctrl::boolean traceTargetValid = allTracesSelected || moduleSelected || validTraceUid(traceTarget);
            if (traceOption && (jsonTraceOption == false) && (traceTargetValid == false))
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
            else if (traceOption)
            {
                const ::ctrl::string selector =
                    jsonTraceOption ? value : traceTarget + (traceEnabled ? ".enable" : ".disable");
                applicationArguments.traceArguments.push_back(selector);
            }
            commandArguments.erase(commandArguments.begin(),
                                   commandArguments.begin() +
                                       static_cast<::ctrl::stringvector::difference_type>(consumed));
        }
        return true;
    }

} /* namespace xwalk::ctrl */
