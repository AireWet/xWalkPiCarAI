#include "xHal_Rpi5CarVoiceAssistantSimulationArguments.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    XWalkVoiceAssistantSimulationArguments::XWalkVoiceAssistantSimulationArguments(int32 count, charpointer values[])
        : targetValue{}, enabledValue(true), updateValue(false), validValue(false), helpValue(false)
    {
        if (count == 1)
        {
            validValue = true;
            return;
        }
        const boolean helpValid = (count == 2) && (values != nullptr) && (values[1] != nullptr);
        if (helpValid)
        {
            const stringview option(values[1]);
            helpValue = (option == "--help") || (option == "-h");
            validValue = helpValue;
            return;
        }
        const boolean traceValid = (count == 3) && (values != nullptr) && (values[1] != nullptr) &&
                                   (values[2] != nullptr) && (stringview(values[1]) == "--trace");
        if (traceValid)
        {
            parseSelector(values[2]);
        }
    }
    XWalkVoiceAssistantSimulationArguments::~XWalkVoiceAssistantSimulationArguments() = default;
    boolean XWalkVoiceAssistantSimulationArguments::valid() const noexcept
    {
        return validValue;
    }
    boolean XWalkVoiceAssistantSimulationArguments::helpRequested() const noexcept
    {
        return helpValue;
    }
    boolean XWalkVoiceAssistantSimulationArguments::applyTraceUpdate() const
    {
        if (!updateValue)
        {
            return true;
        }
        const boolean json = (targetValue.size() > 5U) && (targetValue.substr(targetValue.size() - 5U) == ".json");
        const string argument = json ? targetValue : targetValue + (enabledValue ? ".enable" : ".disable");
        return XWalkTrace::applyGlobalTraceArgument(argument);
    }
    boolean XWalkVoiceAssistantSimulationArguments::targetIsValid(stringview target) noexcept
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
    void XWalkVoiceAssistantSimulationArguments::parseSelector(stringview selector)
    {
        const boolean json = (selector.size() > 5U) && (selector.substr(selector.size() - 5U) == ".json");
        if (json)
        {
            targetValue = string(selector);
            updateValue = true;
            validValue = true;
            return;
        }
        const stringview enable(".enable");
        const stringview disable(".disable");
        const boolean enabling =
            selector.size() > enable.size() && selector.substr(selector.size() - enable.size()) == enable;
        const boolean disabling =
            selector.size() > disable.size() && selector.substr(selector.size() - disable.size()) == disable;
        if (!enabling && !disabling)
        {
            return;
        }
        const size suffix = enabling ? enable.size() : disable.size();
        const stringview target = selector.substr(0U, selector.size() - suffix);
        const boolean targetValid = targetIsValid(target);
        if (targetValid == false)
        {
            return;
        }
        targetValue = string(target);
        enabledValue = enabling;
        updateValue = true;
        validValue = true;
    }
} /* namespace xwalk::hal::sim */
