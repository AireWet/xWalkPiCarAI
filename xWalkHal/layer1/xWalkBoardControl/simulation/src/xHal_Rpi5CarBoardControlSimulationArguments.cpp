/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlSimulationArguments.cpp
 * @brief       Implements persistent BoardControl trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBoardControlSimulationArguments.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
XWalkBoardControlSimulationArguments::XWalkBoardControlSimulationArguments(
    int32 count, charpointer values[]):
    traceTargetValue{}, traceEnabledValue(true), traceUpdateRequestedValue(false),
    validValue(false), helpRequestedValue(false)
{
    if (count == 1) { validValue = true; return; }
    const boolean helpValid = (count == 2) && (values != nullptr) &&
        (values[1] != nullptr);
    if (helpValid)
    {
        const stringview option(values[1]);
        helpRequestedValue = (option == "--help") || (option == "-h");
        validValue = helpRequestedValue; return;
    }
    const boolean traceValid = (count == 3) && (values != nullptr) &&
        (values[1] != nullptr) && (values[2] != nullptr) &&
        (stringview(values[1]) == "--trace");
    if (traceValid) { parseSelector(values[2]); }
}
XWalkBoardControlSimulationArguments::~XWalkBoardControlSimulationArguments() = default;
boolean XWalkBoardControlSimulationArguments::valid() const noexcept { return validValue; }
boolean XWalkBoardControlSimulationArguments::helpRequested() const noexcept
{
    return helpRequestedValue;
}
boolean XWalkBoardControlSimulationArguments::applyTraceUpdate() const
{
    if (!traceUpdateRequestedValue) { return true; }
    const boolean json = (traceTargetValue.size() > 5U) &&
        (traceTargetValue.substr(traceTargetValue.size() - 5U) == ".json");
    const string argument = json ? traceTargetValue : traceTargetValue +
        (traceEnabledValue ? ".enable" : ".disable");
    return XWalkTrace::applyGlobalTraceArgument(argument);
}
boolean XWalkBoardControlSimulationArguments::targetIsValid(stringview target) noexcept
{
    if ((target == "RPI") || (target == "all")) { return true; }
    const stringview prefix("RPI.");
    if (target.substr(0U, prefix.size()) != prefix) { return false; }
    const stringview number = target.substr(prefix.size()); if (number.empty()) { return false; }
    for (const char digit : number) { if ((digit < '0') || (digit > '9')) { return false; } }
    return true;
}
void XWalkBoardControlSimulationArguments::parseSelector(stringview selector)
{
    const boolean json = (selector.size() > 5U) &&
        (selector.substr(selector.size() - 5U) == ".json");
    if (json)
    {
        traceTargetValue = string(selector); traceUpdateRequestedValue = true;
        validValue = true; return;
    }
    const stringview enable(".enable"); const stringview disable(".disable");
    const boolean enabling = selector.size() > enable.size() &&
        selector.substr(selector.size() - enable.size()) == enable;
    const boolean disabling = selector.size() > disable.size() &&
        selector.substr(selector.size() - disable.size()) == disable;
    if (!enabling && !disabling) { return; }
    const size suffixLength = enabling ? enable.size() : disable.size();
    const stringview target = selector.substr(0U, selector.size() - suffixLength);
    if (!targetIsValid(target)) { return; }
    traceTargetValue = string(target); traceEnabledValue = enabling;
    traceUpdateRequestedValue = true; validValue = true;
}
} /* namespace xwalk::hal::sim */
