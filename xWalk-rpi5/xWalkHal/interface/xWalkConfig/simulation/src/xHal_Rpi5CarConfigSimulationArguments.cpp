/******************************************************************************
 * @file        xHal_Rpi5CarConfigSimulationArguments.cpp
 * @brief       Implements xWalkConfig simulation trace-option parsing.
 *
 * @details
 * Validates help, tag, numeric identifier, global, and JSON selectors before
 * delegating persistent changes to the process-wide trace service.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
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

#include "xHal_Rpi5CarConfigSimulationArguments.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim
{

    /**
     * @brief Parses the complete standalone Config command line.
     * @param[in] argumentCount Number of process arguments including the binary name.
     * @param[in] argumentValues Process arguments valid throughout construction.
     */
    XWalkConfigSimulationArguments::XWalkConfigSimulationArguments(int32 argumentCount, charpointer argumentValues[])
        : traceTargetValue{}, traceEnabledValue(true), traceUpdateRequestedValue(false), validValue(false),
          helpRequestedValue(false)
    {
        const boolean noOptionProvided = argumentCount == 1;
        if (noOptionProvided)
        {
            validValue = true;
            return;
        }
        const boolean singleOptionProvided =
            (argumentCount == 2) && (argumentValues != nullptr) && (argumentValues[1] != nullptr);
        if (singleOptionProvided)
        {
            const stringview option(argumentValues[1]);
            const boolean helpOptionProvided = (option == "--help") || (option == "-h");
            if (helpOptionProvided)
            {
                validValue = true;
                helpRequestedValue = true;
            }
            return;
        }
        const boolean argumentShapeValid = (argumentCount == 3) && (argumentValues != nullptr) &&
                                           (argumentValues[1] != nullptr) && (argumentValues[2] != nullptr);
        if (argumentShapeValid == false)
        {
            return;
        }
        const boolean traceOptionProvided = stringview(argumentValues[1]) == "--trace";
        if (traceOptionProvided)
        {
            parseSelector(argumentValues[2]);
        }
    }

    /** @brief Destroys owned selector state without external side effects. */
    XWalkConfigSimulationArguments::~XWalkConfigSimulationArguments() = default;

    /**
     * @brief Returns whether the command-line shape and selector are valid.
     * @return `true` for no option, help, or one supported trace selector.
     */
    boolean XWalkConfigSimulationArguments::valid() const noexcept
    {
        return validValue;
    }

    /**
     * @brief Returns whether help was requested.
     * @return `true` for `--help` or `-h`; otherwise `false`.
     */
    boolean XWalkConfigSimulationArguments::helpRequested() const noexcept
    {
        return helpRequestedValue;
    }

    /**
     * @brief Applies and persists the requested trace update.
     * @return `true` when no update is needed or the requested update succeeds.
     */
    boolean XWalkConfigSimulationArguments::applyTraceUpdate() const
    {
        if (traceUpdateRequestedValue == false)
        {
            return true;
        }
        const boolean jsonSelected =
            (traceTargetValue.size() > 5U) && (traceTargetValue.substr(traceTargetValue.size() - 5U) == ".json");
        const string argument =
            jsonSelected ? traceTargetValue : traceTargetValue + (traceEnabledValue ? ".enable" : ".disable");
        return XWalkTrace::applyGlobalTraceArgument(argument);
    }

    /**
     * @brief Validates one selector target without its operation suffix.
     * @param[in] target Candidate `all`, `RPI`, or `RPI.<digits>` target.
     * @return `true` when the complete target is supported.
     */
    boolean XWalkConfigSimulationArguments::targetIsValid(stringview target) noexcept
    {
        if ((target == "RPI") || (target == "all"))
        {
            return true;
        }
        const stringview prefix("RPI.");
        const boolean prefixValid = target.substr(0U, prefix.size()) == prefix;
        if (prefixValid == false)
        {
            return false;
        }
        const stringview number = target.substr(prefix.size());
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

    /**
     * @brief Parses one complete trace selector or JSON path.
     * @param[in] selector Candidate selector retained only when valid.
     */
    void XWalkConfigSimulationArguments::parseSelector(stringview selector)
    {
        const boolean jsonSelected = (selector.size() > 5U) && (selector.substr(selector.size() - 5U) == ".json");
        if (jsonSelected)
        {
            traceTargetValue = string(selector);
            traceUpdateRequestedValue = true;
            validValue = true;
            return;
        }
        const stringview enableSuffix(".enable");
        const stringview disableSuffix(".disable");
        const boolean enableRequested = selector.size() > enableSuffix.size() &&
                                        selector.substr(selector.size() - enableSuffix.size()) == enableSuffix;
        const boolean disableRequested = selector.size() > disableSuffix.size() &&
                                         selector.substr(selector.size() - disableSuffix.size()) == disableSuffix;
        if ((enableRequested == false) && (disableRequested == false))
        {
            return;
        }
        const size suffixLength = enableRequested ? enableSuffix.size() : disableSuffix.size();
        const stringview target = selector.substr(0U, selector.size() - suffixLength);
        if (targetIsValid(target) == false)
        {
            return;
        }
        traceTargetValue = string(target);
        traceEnabledValue = enableRequested;
        traceUpdateRequestedValue = true;
        validValue = true;
    }

} /* namespace xwalk::hal::sim */
