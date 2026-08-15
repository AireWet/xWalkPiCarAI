/******************************************************************************
 * @file        xHal_Rpi5CarUtilsSimulationArguments.cpp
 * @brief       Implements xWalkUtils simulation trace-option parsing.
 * @details     Validates and applies persistent tag, UID, global, and JSON selectors.
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarUtilsSimulationArguments.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    XWalkUtilsSimulationArguments::XWalkUtilsSimulationArguments(int32 argumentCount, charpointer argumentValues[])
        : traceTargetValue{}, traceEnabledValue(true), traceUpdateRequestedValue(false), validValue(false),
          helpRequestedValue(false)
    {
        if (argumentCount == 1)
        {
            validValue = true;
            return;
        }
        const boolean singleOption =
            (argumentCount == 2) && (argumentValues != nullptr) && (argumentValues[1] != nullptr);
        if (singleOption)
        {
            const stringview option(argumentValues[1]);
            helpRequestedValue = (option == "--help") || (option == "-h");
            validValue = helpRequestedValue;
            return;
        }
        const boolean shapeValid = (argumentCount == 3) && (argumentValues != nullptr) &&
                                   (argumentValues[1] != nullptr) && (argumentValues[2] != nullptr);
        if (shapeValid && (stringview(argumentValues[1]) == "--trace"))
        {
            parseSelector(argumentValues[2]);
        }
    }
    XWalkUtilsSimulationArguments::~XWalkUtilsSimulationArguments() = default;
    boolean XWalkUtilsSimulationArguments::valid() const noexcept
    {
        return validValue;
    }
    boolean XWalkUtilsSimulationArguments::helpRequested() const noexcept
    {
        return helpRequestedValue;
    }
    boolean XWalkUtilsSimulationArguments::applyTraceUpdate() const
    {
        if (traceUpdateRequestedValue == false)
        {
            return true;
        }
        const boolean json =
            (traceTargetValue.size() > 5U) && (traceTargetValue.substr(traceTargetValue.size() - 5U) == ".json");
        const string argument =
            json ? traceTargetValue : traceTargetValue + (traceEnabledValue ? ".enable" : ".disable");
        return XWalkTrace::applyGlobalTraceArgument(argument);
    }
    boolean XWalkUtilsSimulationArguments::targetIsValid(stringview target) noexcept
    {
        if ((target == "RPI") || (target == "all"))
        {
            return true;
        }
        const stringview prefix("RPI.");
        if (target.substr(0U, prefix.size()) != prefix)
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
    void XWalkUtilsSimulationArguments::parseSelector(stringview selector)
    {
        const boolean json = (selector.size() > 5U) && (selector.substr(selector.size() - 5U) == ".json");
        if (json)
        {
            traceTargetValue = string(selector);
            traceUpdateRequestedValue = true;
            validValue = true;
            return;
        }
        const stringview enableSuffix(".enable");
        const stringview disableSuffix(".disable");
        const boolean enable = selector.size() > enableSuffix.size() &&
                               selector.substr(selector.size() - enableSuffix.size()) == enableSuffix;
        const boolean disable = selector.size() > disableSuffix.size() &&
                                selector.substr(selector.size() - disableSuffix.size()) == disableSuffix;
        if ((enable == false) && (disable == false))
        {
            return;
        }
        const size suffixLength = enable ? enableSuffix.size() : disableSuffix.size();
        const stringview target = selector.substr(0U, selector.size() - suffixLength);
        if (targetIsValid(target))
        {
            traceTargetValue = string(target);
            traceEnabledValue = enable;
            traceUpdateRequestedValue = true;
            validValue = true;
        }
    }
} /* namespace xwalk::hal::sim */
