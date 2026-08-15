/******************************************************************************
 * @file        xHal_Rpi5CarPwmSimulationArguments.cpp
 * @brief       Implements persistent xWalkPwm trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarPwmSimulationArguments.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    XWalkPwmSimulationArguments::XWalkPwmSimulationArguments(int32 argumentCount, charpointer argumentValues[])
        : traceTargetValue{}, traceEnabledValue(true), traceUpdateRequestedValue(false), validValue(false),
          helpRequestedValue(false)
    {
        if (argumentCount == 1)
        {
            validValue = true;
            return;
        }
        const boolean helpShapeValid =
            (argumentCount == 2) && (argumentValues != nullptr) && (argumentValues[1] != nullptr);
        if (helpShapeValid)
        {
            const stringview option(argumentValues[1]);
            helpRequestedValue = (option == "--help") || (option == "-h");
            validValue = helpRequestedValue;
            return;
        }
        const boolean traceShapeValid = (argumentCount == 3) && (argumentValues != nullptr) &&
                                        (argumentValues[1] != nullptr) && (argumentValues[2] != nullptr) &&
                                        (stringview(argumentValues[1]) == "--trace");
        if (traceShapeValid)
        {
            parseSelector(argumentValues[2]);
        }
    }

    XWalkPwmSimulationArguments::~XWalkPwmSimulationArguments() = default;
    boolean XWalkPwmSimulationArguments::valid() const noexcept
    {
        return validValue;
    }
    boolean XWalkPwmSimulationArguments::helpRequested() const noexcept
    {
        return helpRequestedValue;
    }

    boolean XWalkPwmSimulationArguments::applyTraceUpdate() const
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

    boolean XWalkPwmSimulationArguments::targetIsValid(stringview target) noexcept
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

    void XWalkPwmSimulationArguments::parseSelector(stringview selector)
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
