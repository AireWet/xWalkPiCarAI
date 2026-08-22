/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelSimulationArguments.cpp
 * @brief       Implements language-model simulation trace argument parsing.
 * @details     Validates and applies persistent tag, UID, global, and JSON selectors.
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarLanguageModelSimulationArguments.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    XWalkLanguageModelSimulationArguments::XWalkLanguageModelSimulationArguments(int32 count, charpointer values[])
        : traceTargetValue{}, traceEnabledValue(true), traceUpdateRequestedValue(false), validValue(false),
          helpRequestedValue(false)
    {
        if (count == 1)
        {
            validValue = true;
            return;
        }
        const boolean single = (count == 2) && (values != nullptr) && (values[1] != nullptr);
        if (single)
        {
            const stringview option(values[1]);
            helpRequestedValue = (option == "--help") || (option == "-h");
            validValue = helpRequestedValue;
            return;
        }
        const boolean shape = (count == 3) && (values != nullptr) && (values[1] != nullptr) && (values[2] != nullptr);
        const boolean traceShape = shape && (stringview(values[1]) == "--trace");
        if (traceShape)
        {
            parseSelector(values[2]);
        }
    }
    XWalkLanguageModelSimulationArguments::~XWalkLanguageModelSimulationArguments() = default;
    boolean XWalkLanguageModelSimulationArguments::valid() const noexcept
    {
        return validValue;
    }
    boolean XWalkLanguageModelSimulationArguments::helpRequested() const noexcept
    {
        return helpRequestedValue;
    }
    boolean XWalkLanguageModelSimulationArguments::applyTraceUpdate() const
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
    boolean XWalkLanguageModelSimulationArguments::targetIsValid(stringview target) noexcept
    {
        if ((target == "RPI") || (target == "all"))
        {
            return true;
        }
        const stringview prefix("RPI.");
        const stringview targetPrefix = target.substr(0U, prefix.size());
        if (targetPrefix != prefix)
        {
            return false;
        }
        const stringview number = target.substr(prefix.size());
        const boolean numberEmpty = number.empty();
        if (numberEmpty)
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
    void XWalkLanguageModelSimulationArguments::parseSelector(stringview selector)
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
        const boolean targetValid = targetIsValid(target);
        if (targetValid)
        {
            traceTargetValue = string(target);
            traceEnabledValue = enable;
            traceUpdateRequestedValue = true;
            validValue = true;
        }
    }
} // namespace xwalk::hal::sim
