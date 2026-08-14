/******************************************************************************
 * @file        xHal_Rpi5CarAudioSimulationArguments.cpp
 * @brief       Implements xWalkAudio simulation trace-option parsing.
 *
 * @details
 * Validates help, tag, numeric identifier, global, and JSON trace selectors and
 * delegates successful persistent updates to the global trace service.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio Host Simulation
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

#include "xHal_Rpi5CarAudioSimulationArguments.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim
{

XWalkAudioSimulationArguments::XWalkAudioSimulationArguments(
    int32 argumentCount, charpointer argumentValues[]):
    traceTargetValue{}, traceEnabledValue(true), traceUpdateRequestedValue(false),
    validValue(false), helpRequestedValue(false)
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
    const boolean argumentShapeValid =
        (argumentCount == 3) && (argumentValues != nullptr) &&
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

XWalkAudioSimulationArguments::~XWalkAudioSimulationArguments() = default;

boolean XWalkAudioSimulationArguments::valid() const noexcept { return validValue; }
boolean XWalkAudioSimulationArguments::helpRequested() const noexcept { return helpRequestedValue; }

boolean XWalkAudioSimulationArguments::applyTraceUpdate() const
{
    if (traceUpdateRequestedValue == false)
    {
        return true;
    }
    const boolean jsonSelected = (traceTargetValue.size() > 5U) &&
        (traceTargetValue.substr(traceTargetValue.size() - 5U) == ".json");
    const string argument = jsonSelected ? traceTargetValue : traceTargetValue +
        (traceEnabledValue ? ".enable" : ".disable");
    return XWalkTrace::applyGlobalTraceArgument(argument);
}

boolean XWalkAudioSimulationArguments::targetIsValid(stringview target) noexcept
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

void XWalkAudioSimulationArguments::parseSelector(stringview selector)
{
    const boolean jsonSelected = (selector.size() > 5U) &&
        (selector.substr(selector.size() - 5U) == ".json");
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
    const boolean operationValid = enableRequested || disableRequested;
    if (operationValid == false)
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
