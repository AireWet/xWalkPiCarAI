/******************************************************************************
 * @file        xHal_Rpi5CarTraceControl.cpp
 * @brief       Implements ordered in-memory trace configuration control.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 * @author      Joxy John
 * @date        2026-08-09
 * @version     3.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarTrace.h"

#include <json-c/json.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

/** @brief Maximum accepted trace JSON size in bytes. */
constexpr std::uintmax_t XWALK_TRACE_MAXIMUM_JSON_BYTES{1'048'576U};

/** @brief Decodes one exact JSON trace state into a Boolean value. */
xwalk::hal::boolean jsonTraceState(json_object* value,
    xwalk::hal::boolean& enabled)
{
    if ((value == nullptr) || (json_object_get_type(value) != json_type_string))
    {
        return false;
    }
    const xwalk::hal::stringview state(json_object_get_string(value));
    if (state == "enable")
    {
        enabled = true;
        return true;
    }
    if (state == "disable")
    {
        enabled = false;
        return true;
    }
    return false;
}

} /* namespace */

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/** @brief Applies one exact known trace state. */
boolean XWalkTrace::setTraceEnabled(stringview uid, boolean enabled)
{
    const auto trace = traceSourceLocations.find(string(uid));
    if ((isValidUid(uid) == false) || (trace == traceSourceLocations.end()))
    {
        traceConfigurationErrorValue = "Unknown trace ID: " + string(uid);
        return false;
    }
    traceEnabledValues[string(uid)] = enabled;
    traceConfigurationErrorValue.clear();
    return true;
}

/** @brief Applies one module state and removes earlier tag overrides in it. */
boolean XWalkTrace::setModuleTracesEnabled(stringview module, boolean enabled)
{
    boolean moduleKnown = false;
    const string prefix = string(module) + ".";
    for (const auto& trace : traceSourceLocations)
    {
        if (trace.first.rfind(prefix, 0U) == 0U)
        {
            moduleKnown = true;
            break;
        }
    }
    if (moduleKnown == false)
    {
        traceConfigurationErrorValue = "Unknown trace module: " + string(module);
        return false;
    }
    moduleEnabledValues[string(module)] = enabled;
    auto trace = traceEnabledValues.begin();
    while (trace != traceEnabledValues.end())
    {
        if (trace->first.rfind(prefix, 0U) == 0U)
        {
            trace = traceEnabledValues.erase(trace);
        }
        else
        {
            ++trace;
        }
    }
    traceConfigurationErrorValue.clear();
    return true;
}

/** @brief Applies one global state and removes every earlier override. */
boolean XWalkTrace::setAllTracesEnabled(boolean enabled)
{
    globalTraceEnabledValue = enabled;
    moduleEnabledValues.clear();
    traceEnabledValues.clear();
    traceConfigurationErrorValue.clear();
    return true;
}

/** @brief Loads, validates, and applies one ordered JSON configuration. */
boolean XWalkTrace::loadJsonConfiguration(const filesystempath& configurationPath)
{
    errorcode fileError;
    const std::uintmax_t fileBytes = std::filesystem::file_size(
        configurationPath, fileError);
    if (fileError || (fileBytes > XWALK_TRACE_MAXIMUM_JSON_BYTES))
    {
        traceConfigurationErrorValue =
            "Trace JSON file is missing, unreadable, or too large: " +
            configurationPath.string();
        return false;
    }
    json_object* root = json_object_from_file(configurationPath.string().c_str());
    if ((root == nullptr) || (json_object_get_type(root) != json_type_object))
    {
        if (root != nullptr)
        {
            json_object_put(root);
        }
        traceConfigurationErrorValue =
            "Trace JSON is invalid: " + configurationPath.string();
        return false;
    }
    json_object* traceObject = nullptr;
    const boolean traceObjectValid = json_object_object_get_ex(
        root, "trace", &traceObject) &&
        (json_object_get_type(traceObject) == json_type_object);
    if (traceObjectValid == false)
    {
        json_object_put(root);
        traceConfigurationErrorValue = "Trace JSON requires an object named trace";
        return false;
    }

    stringvector selectors;
    json_object* allObject = nullptr;
    if (json_object_object_get_ex(traceObject, "all", &allObject))
    {
        json_object* stateObject = nullptr;
        boolean enabled = false;
        const boolean allValid =
            (json_object_get_type(allObject) == json_type_object) &&
            json_object_object_get_ex(allObject, "state", &stateObject) &&
            jsonTraceState(stateObject, enabled);
        if (allValid == false)
        {
            json_object_put(root);
            traceConfigurationErrorValue =
                "Trace JSON all.state must be enable or disable";
            return false;
        }
        selectors.push_back(enabled ? "all.enable" : "all.disable");
    }

    json_object_object_foreach(traceObject, moduleName, moduleObject)
    {
        const stringview module(moduleName);
        if (module == "all")
        {
            continue;
        }
        const string prefix = string(module) + ".";
        boolean moduleKnown = false;
        for (const auto& trace : traceSourceLocations)
        {
            if (trace.first.rfind(prefix, 0U) == 0U)
            {
                moduleKnown = true;
                break;
            }
        }
        if ((moduleKnown == false) ||
            (json_object_get_type(moduleObject) != json_type_object))
        {
            json_object_put(root);
            traceConfigurationErrorValue = "Unknown trace module in JSON: " +
                string(module);
            return false;
        }
        json_object* stateObject = nullptr;
        if (json_object_object_get_ex(moduleObject, "state", &stateObject))
        {
            boolean enabled = false;
            if (jsonTraceState(stateObject, enabled) == false)
            {
                json_object_put(root);
                traceConfigurationErrorValue = "Trace JSON module state must be "
                    "enable or disable: " + string(module);
                return false;
            }
            selectors.push_back(string(module) +
                (enabled ? ".enable" : ".disable"));
        }
    }

    json_object_object_foreach(traceObject, tagModuleName, tagModuleObject)
    {
        if (stringview(tagModuleName) == "all")
        {
            continue;
        }
        json_object* tagsObject = nullptr;
        if (json_object_object_get_ex(tagModuleObject, "tags", &tagsObject))
        {
            if (json_object_get_type(tagsObject) != json_type_object)
            {
                json_object_put(root);
                traceConfigurationErrorValue = "Trace JSON tags must be an object";
                return false;
            }
            json_object_object_foreach(tagsObject, numericId, stateObject)
            {
                const string uid = string(tagModuleName) + "." + numericId;
                boolean enabled = false;
                if ((isValidUid(uid) == false) ||
                    (traceSourceLocations.find(uid) == traceSourceLocations.end()))
                {
                    json_object_put(root);
                    traceConfigurationErrorValue =
                        "Unknown trace ID in JSON: " + uid;
                    return false;
                }
                if (jsonTraceState(stateObject, enabled) == false)
                {
                    json_object_put(root);
                    traceConfigurationErrorValue = "Trace JSON tag state must be "
                        "enable or disable: " + uid;
                    return false;
                }
                selectors.push_back(uid + (enabled ? ".enable" : ".disable"));
            }
        }
    }
    json_object_put(root);

    for (const string& selector : selectors)
    {
        const size stateSeparator = selector.rfind('.');
        const string target = selector.substr(0U, stateSeparator);
        const boolean enabled = selector.substr(stateSeparator + 1U) == "enable";
        const size uidSeparator = target.find('.');
        const boolean applied = target == "all" ? setAllTracesEnabled(enabled) :
            (uidSeparator == string::npos ?
                setModuleTracesEnabled(target, enabled) :
                setTraceEnabled(target, enabled));
        if (applied == false)
        {
            return false;
        }
    }
    return true;
}

/** @brief Enables one known trace UID. */
boolean XWalkTrace::enableGlobalTrace(stringview uid)
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    return instance.setTraceEnabled(uid, true);
}

/** @brief Disables one known trace UID. */
boolean XWalkTrace::disableGlobalTrace(stringview uid)
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    return instance.setTraceEnabled(uid, false);
}

/** @brief Enables all registered and future normal traces. */
boolean XWalkTrace::enableAllGlobalTraces()
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    return instance.setAllTracesEnabled(true);
}

/** @brief Disables all registered and future normal traces. */
boolean XWalkTrace::disableAllGlobalTraces()
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    return instance.setAllTracesEnabled(false);
}

/** @brief Restores the mandatory all-disabled startup configuration. */
boolean XWalkTrace::resetGlobalTraceConfiguration()
{
    return disableAllGlobalTraces();
}

/** @brief Applies one exact selector or JSON path from left to right. */
boolean XWalkTrace::applyGlobalTraceArgument(stringview argument)
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    const boolean jsonSelected = (argument.size() > 5U) &&
        (argument.substr(argument.size() - 5U) == ".json");
    if (jsonSelected)
    {
        return instance.loadJsonConfiguration(filesystempath(argument));
    }
    const size stateSeparator = argument.rfind('.');
    if (stateSeparator == stringview::npos)
    {
        instance.traceConfigurationErrorValue =
            "Invalid trace selector: " + string(argument);
        return false;
    }
    const stringview target = argument.substr(0U, stateSeparator);
    const stringview state = argument.substr(stateSeparator + 1U);
    const boolean stateValid = (state == "enable") || (state == "disable");
    if (stateValid == false)
    {
        instance.traceConfigurationErrorValue =
            "Trace state must be enable or disable: " + string(argument);
        return false;
    }
    const boolean enabled = state == "enable";
    if (target == "all")
    {
        return instance.setAllTracesEnabled(enabled);
    }
    if (target.find('.') == stringview::npos)
    {
        return instance.setModuleTracesEnabled(target, enabled);
    }
    return instance.setTraceEnabled(target, enabled);
}

/** @brief Returns the most recent runtime configuration error. */
string XWalkTrace::globalTraceConfigurationError()
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    return instance.traceConfigurationErrorValue;
}

} /* namespace xwalk::hal */
