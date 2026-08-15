/******************************************************************************
 * @file        xHal_Rpi5CarTraceControl.cpp
 * @brief       Implements ordered persistent trace configuration control.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 * @author      Joxy John
 * @date        2026-08-09
 * @version     3.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarTrace.h"

#include <json-c/json.h>
#include <tinyxml2.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

    /** @brief Maximum accepted trace JSON size in bytes. */
    constexpr std::uintmax_t XWALK_TRACE_MAXIMUM_JSON_BYTES{1'048'576U};

    /** @brief Decodes one exact JSON trace state into a Boolean value. */
    xwalk::hal::boolean jsonTraceState(json_object* value, xwalk::hal::boolean& enabled)
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

    /**
     * @brief Resolves one known UID through its trace, module, and global states.
     *
     * @param[in] uid
     * Complete scanner-known trace identifier.
     *
     * @return
     * `true` only when the UID exists and its effective persistent state is enabled.
     */
    boolean XWalkTrace::traceIsEnabled(stringview uid) const
    {
        const string uidValue(uid);
        const auto source = traceSourceLocations.find(uidValue);
        const boolean sourceKnown = source != traceSourceLocations.end();
        if (sourceKnown == false)
        {
            return false;
        }
        const auto trace = traceEnabledValues.find(uidValue);
        const boolean traceOverridePresent = trace != traceEnabledValues.end();
        if (traceOverridePresent)
        {
            return trace->second;
        }
        const size separatorPosition = uid.find('.');
        const string moduleName(uid.substr(0U, separatorPosition));
        const auto module = moduleEnabledValues.find(moduleName);
        const boolean moduleOverridePresent = module != moduleEnabledValues.end();
        return moduleOverridePresent ? module->second : globalTraceEnabledValue;
    }

    /**
     * @brief Atomically persists effective runtime states in the configured XML catalogue.
     *
     * @details
     * Writes a same-directory replacement, preserves existing permission bits,
     * and publishes it with one rename only after every known UID is updated.
     *
     * @return
     * `true` after the complete XML replacement succeeds; otherwise `false` and
     * `traceConfigurationErrorValue` describes the failure.
     */
    boolean XWalkTrace::persistConfiguration()
    {
        tinyxml2::XMLDocument document;
        const string configurationPath = configurationPathValue.string();
        const tinyxml2::XMLError loadResult = document.LoadFile(configurationPath.c_str());
        if (loadResult != tinyxml2::XML_SUCCESS)
        {
            traceConfigurationErrorValue = "Trace XML could not be loaded for update: " + configurationPath;
            return false;
        }
        tinyxml2::XMLElement* root = document.FirstChildElement("xwalkTraceCatalogue");
        const boolean rootPresent = root != nullptr;
        if (rootPresent == false)
        {
            traceConfigurationErrorValue = "Trace XML root is missing: " + configurationPath;
            return false;
        }
        root->SetAttribute("defaultState", globalTraceEnabledValue ? "enable" : "disable");

        orderedmap<string, boolean> updatedTraceIds;
        boolean catalogueValid = true;
        for (tinyxml2::XMLElement* module = root->FirstChildElement("module"); (module != nullptr) && catalogueValid;
             module = module->NextSiblingElement("module"))
        {
            const char* moduleName = module->Attribute("name");
            const boolean moduleNamePresent = moduleName != nullptr;
            if (moduleNamePresent == false)
            {
                catalogueValid = false;
                break;
            }
            const auto configuredModule = moduleEnabledValues.find(moduleName);
            const boolean moduleOverridePresent = configuredModule != moduleEnabledValues.end();
            const boolean moduleEnabled = moduleOverridePresent ? configuredModule->second : globalTraceEnabledValue;
            module->SetAttribute("defaultState", moduleEnabled ? "enable" : "disable");

            for (tinyxml2::XMLElement* trace = module->FirstChildElement("trace"); (trace != nullptr) && catalogueValid;
                 trace = trace->NextSiblingElement("trace"))
            {
                const char* uid = trace->Attribute("fullId");
                const boolean uidPresent = uid != nullptr;
                const boolean uidKnown = uidPresent && (traceSourceLocations.find(uid) != traceSourceLocations.end());
                const boolean uidUnique = uidPresent && (updatedTraceIds.find(uid) == updatedTraceIds.end());
                if ((uidKnown == false) || (uidUnique == false))
                {
                    catalogueValid = false;
                    break;
                }
                const boolean enabled = traceIsEnabled(uid);
                trace->SetAttribute("defaultState", enabled ? "enable" : "disable");
                updatedTraceIds[string(uid)] = true;
            }
        }
        const boolean everyTraceUpdated = updatedTraceIds.size() == traceSourceLocations.size();
        if ((catalogueValid == false) || (everyTraceUpdated == false))
        {
            traceConfigurationErrorValue = "Trace XML inventory changed while applying persistent state";
            return false;
        }

        const filesystempath temporaryPath(configurationPath + XHAL_RPI5CAR_CONFIG_REPLACEMENT_SUFFIX);
        errorcode operationError;
        static_cast<void>(removeFilesystemEntry(temporaryPath, operationError));
        const tinyxml2::XMLError saveResult = document.SaveFile(temporaryPath.string().c_str());
        if (saveResult != tinyxml2::XML_SUCCESS)
        {
            traceConfigurationErrorValue = "Trace XML replacement could not be written: " + temporaryPath.string();
            return false;
        }

        operationError.clear();
        const filesystemstatus originalStatus = filesystemStatus(configurationPathValue, operationError);
        const boolean statusRead = operationError.value() == 0;
        if (statusRead)
        {
            operationError.clear();
            replaceFilesystemPermissions(temporaryPath, originalStatus.permissions(), operationError);
        }
        const boolean permissionsPreserved = operationError.value() == 0;
        if (permissionsPreserved == false)
        {
            operationError.clear();
            static_cast<void>(removeFilesystemEntry(temporaryPath, operationError));
            traceConfigurationErrorValue = "Trace XML replacement permissions could not be preserved";
            return false;
        }

        operationError.clear();
        renameFilesystemEntry(temporaryPath, configurationPathValue, operationError);
        const boolean replacementPublished = operationError.value() == 0;
        if (replacementPublished == false)
        {
            operationError.clear();
            static_cast<void>(removeFilesystemEntry(temporaryPath, operationError));
            traceConfigurationErrorValue = "Trace XML replacement could not be published";
            return false;
        }
        traceConfigurationErrorValue.clear();
        return true;
    }

    /** @brief Loads, validates, and applies one ordered JSON configuration. */
    boolean XWalkTrace::loadJsonConfiguration(const filesystempath& configurationPath)
    {
        errorcode fileError;
        const std::uintmax_t fileBytes = std::filesystem::file_size(configurationPath, fileError);
        if (fileError || (fileBytes > XWALK_TRACE_MAXIMUM_JSON_BYTES))
        {
            traceConfigurationErrorValue =
                "Trace JSON file is missing, unreadable, or too large: " + configurationPath.string();
            return false;
        }
        json_object* root = json_object_from_file(configurationPath.string().c_str());
        if ((root == nullptr) || (json_object_get_type(root) != json_type_object))
        {
            if (root != nullptr)
            {
                json_object_put(root);
            }
            traceConfigurationErrorValue = "Trace JSON is invalid: " + configurationPath.string();
            return false;
        }
        json_object* traceObject = nullptr;
        const boolean traceObjectValid = json_object_object_get_ex(root, "trace", &traceObject) &&
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
            const boolean allValid = (json_object_get_type(allObject) == json_type_object) &&
                                     json_object_object_get_ex(allObject, "state", &stateObject) &&
                                     jsonTraceState(stateObject, enabled);
            if (allValid == false)
            {
                json_object_put(root);
                traceConfigurationErrorValue = "Trace JSON all.state must be enable or disable";
                return false;
            }
            selectors.push_back(enabled ? "all.enable" : "all.disable");
        }

        json_object_object_foreach(traceObject, moduleName, moduleObject)
        {
            // json-c owns moduleName through root. Keep an owned copy because
            // validation failures release root before composing the diagnostic.
            const string module(moduleName);
            if (module == "all")
            {
                continue;
            }
            const string prefix = module + ".";
            boolean moduleKnown = false;
            for (const auto& trace : traceSourceLocations)
            {
                if (trace.first.rfind(prefix, 0U) == 0U)
                {
                    moduleKnown = true;
                    break;
                }
            }
            if ((moduleKnown == false) || (json_object_get_type(moduleObject) != json_type_object))
            {
                json_object_put(root);
                traceConfigurationErrorValue = "Unknown trace module in JSON: " + module;
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
                                                   "enable or disable: " +
                                                   module;
                    return false;
                }
                selectors.push_back(module + (enabled ? ".enable" : ".disable"));
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
                    if ((isValidUid(uid) == false) || (traceSourceLocations.find(uid) == traceSourceLocations.end()))
                    {
                        json_object_put(root);
                        traceConfigurationErrorValue = "Unknown trace ID in JSON: " + uid;
                        return false;
                    }
                    if (jsonTraceState(stateObject, enabled) == false)
                    {
                        json_object_put(root);
                        traceConfigurationErrorValue = "Trace JSON tag state must be "
                                                       "enable or disable: " +
                                                       uid;
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
            const boolean applied = target == "all"
                                        ? setAllTracesEnabled(enabled)
                                        : (uidSeparator == string::npos ? setModuleTracesEnabled(target, enabled)
                                                                        : setTraceEnabled(target, enabled));
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
        const orderedmap<string, boolean> previousTraces = instance.traceEnabledValues;
        const boolean stateApplied = instance.setTraceEnabled(uid, true);
        if (stateApplied == false)
        {
            return false;
        }
        const boolean statePersisted = instance.persistConfiguration();
        if (statePersisted == false)
        {
            instance.traceEnabledValues = previousTraces;
        }
        return statePersisted;
    }

    /** @brief Disables one known trace UID. */
    boolean XWalkTrace::disableGlobalTrace(stringview uid)
    {
        XWalkTrace& instance = globalInstance();
        mutexlock lock(instance.traceMutex);
        const orderedmap<string, boolean> previousTraces = instance.traceEnabledValues;
        const boolean stateApplied = instance.setTraceEnabled(uid, false);
        if (stateApplied == false)
        {
            return false;
        }
        const boolean statePersisted = instance.persistConfiguration();
        if (statePersisted == false)
        {
            instance.traceEnabledValues = previousTraces;
        }
        return statePersisted;
    }

    /** @brief Enables all registered and future normal traces. */
    boolean XWalkTrace::enableAllGlobalTraces()
    {
        XWalkTrace& instance = globalInstance();
        mutexlock lock(instance.traceMutex);
        const boolean previousGlobal = instance.globalTraceEnabledValue;
        const orderedmap<string, boolean> previousModules = instance.moduleEnabledValues;
        const orderedmap<string, boolean> previousTraces = instance.traceEnabledValues;
        static_cast<void>(instance.setAllTracesEnabled(true));
        const boolean statePersisted = instance.persistConfiguration();
        if (statePersisted == false)
        {
            instance.globalTraceEnabledValue = previousGlobal;
            instance.moduleEnabledValues = previousModules;
            instance.traceEnabledValues = previousTraces;
        }
        return statePersisted;
    }

    /** @brief Disables all registered and future normal traces. */
    boolean XWalkTrace::disableAllGlobalTraces()
    {
        XWalkTrace& instance = globalInstance();
        mutexlock lock(instance.traceMutex);
        const boolean previousGlobal = instance.globalTraceEnabledValue;
        const orderedmap<string, boolean> previousModules = instance.moduleEnabledValues;
        const orderedmap<string, boolean> previousTraces = instance.traceEnabledValues;
        static_cast<void>(instance.setAllTracesEnabled(false));
        const boolean statePersisted = instance.persistConfiguration();
        if (statePersisted == false)
        {
            instance.globalTraceEnabledValue = previousGlobal;
            instance.moduleEnabledValues = previousModules;
            instance.traceEnabledValues = previousTraces;
        }
        return statePersisted;
    }

    /** @brief Persists an all-disabled normal-trace configuration. */
    boolean XWalkTrace::resetGlobalTraceConfiguration()
    {
        return disableAllGlobalTraces();
    }

    /** @brief Applies one exact selector or JSON path from left to right. */
    boolean XWalkTrace::applyGlobalTraceArgument(stringview argument)
    {
        XWalkTrace& instance = globalInstance();
        mutexlock lock(instance.traceMutex);
        const boolean previousGlobal = instance.globalTraceEnabledValue;
        const orderedmap<string, boolean> previousModules = instance.moduleEnabledValues;
        const orderedmap<string, boolean> previousTraces = instance.traceEnabledValues;
        const boolean jsonSelected = (argument.size() > 5U) && (argument.substr(argument.size() - 5U) == ".json");
        boolean stateApplied = false;
        if (jsonSelected)
        {
            stateApplied = instance.loadJsonConfiguration(filesystempath(argument));
        }
        else
        {
            const size stateSeparator = argument.rfind('.');
            if (stateSeparator == stringview::npos)
            {
                instance.traceConfigurationErrorValue = "Invalid trace selector: " + string(argument);
                return false;
            }
            const stringview target = argument.substr(0U, stateSeparator);
            const stringview state = argument.substr(stateSeparator + 1U);
            const boolean stateValid = (state == "enable") || (state == "disable");
            if (stateValid == false)
            {
                instance.traceConfigurationErrorValue = "Trace state must be enable or disable: " + string(argument);
                return false;
            }
            const boolean enabled = state == "enable";
            const size targetSeparator = target.find('.');
            const boolean allSelected = target == "all";
            const boolean moduleSelected = targetSeparator == stringview::npos;
            stateApplied = allSelected ? instance.setAllTracesEnabled(enabled)
                                       : (moduleSelected ? instance.setModuleTracesEnabled(target, enabled)
                                                         : instance.setTraceEnabled(target, enabled));
        }
        if (stateApplied == false)
        {
            return false;
        }
        const boolean statePersisted = instance.persistConfiguration();
        if (statePersisted == false)
        {
            instance.globalTraceEnabledValue = previousGlobal;
            instance.moduleEnabledValues = previousModules;
            instance.traceEnabledValues = previousTraces;
        }
        return statePersisted;
    }

    /** @brief Returns the most recent runtime configuration error. */
    string XWalkTrace::globalTraceConfigurationError()
    {
        XWalkTrace& instance = globalInstance();
        mutexlock lock(instance.traceMutex);
        return instance.traceConfigurationErrorValue;
    }

} /* namespace xwalk::hal */
