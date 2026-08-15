/******************************************************************************
 * @file        xHal_Rpi5CarSpiSimulationArguments.cpp
 * @brief       Implements xWalkSpi simulation trace-option parsing.
 *
 * @details
 * Validates one optional trace selector and applies it through the shared
 * process-wide trace-control API.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Simulation
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

#include "xHal_Rpi5CarSpiSimulationArguments.h"

#include "xHal_Rpi5CarTrace.h"

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkSpi simulation support.
 */
namespace xwalk::hal::sim
{

    /**
     * @brief Parses the complete simulation process argument list.
     * @param[in] argumentCount Number of process arguments, including the binary name.
     * @param[in] argumentValues Non-null process argument array with `argumentCount` entries.
     */
    XWalkSpiSimulationArguments::XWalkSpiSimulationArguments(int32 argumentCount, charpointer argumentValues[])
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
        if (traceOptionProvided == false)
        {
            return;
        }
        parseSelector(argumentValues[2]);
    }

    /** @brief Destroys owned selector text without external side effects. */
    XWalkSpiSimulationArguments::~XWalkSpiSimulationArguments() = default;

    /**
     * @brief Reports whether the complete process argument list is valid.
     * @return `true` for no option, help, or one supported trace selector.
     */
    boolean XWalkSpiSimulationArguments::valid() const noexcept
    {
        return validValue;
    }

    /**
     * @brief Reports whether the caller requested command help.
     * @return `true` for `--help` or `-h`; otherwise `false`.
     */
    boolean XWalkSpiSimulationArguments::helpRequested() const noexcept
    {
        return helpRequestedValue;
    }

    /**
     * @brief Applies the parsed trace update to the configured global trace runtime.
     * @return `true` when no update is needed or registry application succeeds.
     */
    boolean XWalkSpiSimulationArguments::applyTraceUpdate() const
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
     * @brief Reports whether one selector target is `all`, `RPI`, or `RPI.<digits>`.
     * @param[in] target Candidate selector target without the operation suffix.
     * @return `true` when the target is accepted; otherwise `false`.
     */
    boolean XWalkSpiSimulationArguments::targetIsValid(stringview target) noexcept
    {
        const boolean moduleSelected = target == "RPI";
        const boolean allSelected = target == "all";
        if (moduleSelected || allSelected)
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
        const boolean numberMissing = number.empty();
        if (numberMissing)
        {
            return false;
        }
        for (const char digit : number)
        {
            const boolean digitInvalid = (digit < '0') || (digit > '9');
            if (digitInvalid)
            {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Parses one selector and commits it only when completely valid.
     * @param[in] selector Candidate trace selector or JSON configuration path.
     */
    void XWalkSpiSimulationArguments::parseSelector(stringview selector)
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
        const boolean traceTargetValid = targetIsValid(target);
        if (traceTargetValid == false)
        {
            return;
        }

        traceTargetValue = string(target);
        traceEnabledValue = enableRequested;
        traceUpdateRequestedValue = true;
        validValue = true;
    }

} /* namespace xwalk::hal::sim */
