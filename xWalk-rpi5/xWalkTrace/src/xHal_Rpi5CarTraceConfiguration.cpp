/******************************************************************************
 * @file        xHal_Rpi5CarTraceConfiguration.cpp
 * @brief       Loads persistent xWalk trace catalogue state once.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 * @author      Joxy John
 * @date        2026-08-09
 * @version     3.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarTrace.h"

#include <filesystem>
#include <tinyxml2.h>

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /** @brief Validates one complete tagged-trace identifier. */
    boolean XWalkTrace::isValidUid(stringview uid) noexcept
    {
        const size separatorPosition = uid.find('.');
        if ((separatorPosition == stringview::npos) || (separatorPosition == 0U))
        {
            return false;
        }
        const stringview module = uid.substr(0U, separatorPosition);
        if ((module != "RPI") && (module != "CTRL") && (module != "RPIAGENT") && (module != "LIB"))
        {
            return false;
        }
        const stringview number = uid.substr(separatorPosition + 1U);
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

    /** @brief Reopens the append-only log and loads one generated catalogue. */
    void XWalkTrace::initialize(const filesystempath& configurationPath, const filesystempath& logPath)
    {
        configurationPathValue = configurationPath;
        globalTraceEnabledValue = false;
        moduleEnabledValues.clear();
        traceEnabledValues.clear();
        traceSourceLocations.clear();
        traceConfigurationErrorValue.clear();
        startTime = steadyclock::now();

        const filesystempath logDirectory = logPath.parent_path();
        if (logDirectory.empty() == false)
        {
            static_cast<void>(createDirectories(logDirectory));
        }
        if (logFile.is_open())
        {
            logFile.close();
        }
        logFile.clear();
        logFile.open(logPath, FILE_OPEN_WRITE_APPEND);
        if (logFile.is_open() == false)
        {
            throw runtimeerror("Trace log file could not be opened for append");
        }
        logPathValue = logPath;
        loadConfiguration(configurationPath);
    }

    /**
     * @brief Loads known IDs, locations, and persistent states from the XML catalogue.
     *
     * @param[in] configurationPath
     * Existing scanner-generated catalogue whose `defaultState` flags are loaded.
     *
     * @post
     * A valid catalogue replaces the complete in-memory registry. Invalid input
     * leaves all normal traces disabled and writes one unfiltered diagnostic.
     */
    void XWalkTrace::loadConfiguration(const filesystempath& configurationPath)
    {
        tinyxml2::XMLDocument document;
        const tinyxml2::XMLError loadResult = document.LoadFile(configurationPath.string().c_str());
        if (loadResult == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
        {
            writeConfigurationDiagnostic("WARNING", "Trace catalogue is missing; all normal traces are disabled");
            return;
        }
        if (loadResult != tinyxml2::XML_SUCCESS)
        {
            writeConfigurationDiagnostic("ERROR", "Trace catalogue is malformed; all normal traces are disabled");
            return;
        }
        const tinyxml2::XMLElement* root = document.FirstChildElement("xwalkTraceCatalogue");
        const boolean rootPresent = root != nullptr;
        const char* versionAttribute = rootPresent ? root->Attribute("version") : nullptr;
        const boolean versionPresent = versionAttribute != nullptr;
        const boolean versionValid = versionPresent && (stringview(versionAttribute) == "1.0");
        if ((rootPresent == false) || (versionValid == false))
        {
            writeConfigurationDiagnostic("ERROR", "Trace catalogue root is invalid; all normal traces are disabled");
            return;
        }

        const char* globalDefault = root->Attribute("defaultState");
        const stringview globalState = globalDefault == nullptr ? "disable" : globalDefault;
        const boolean globalStateValid = (globalState == "enable") || (globalState == "disable");
        if (globalStateValid == false)
        {
            writeConfigurationDiagnostic("ERROR",
                                         "Trace catalogue global state is invalid; all normal traces are disabled");
            return;
        }

        orderedmap<string, XWalkTraceSourceLocation> loadedLocations;
        orderedmap<string, boolean> loadedModules;
        orderedmap<string, boolean> loadedTraces;
        orderedmap<string, boolean> loadedScopedNumericIds;
        boolean catalogueValid = true;
        for (const tinyxml2::XMLElement* module = root->FirstChildElement("module");
             (module != nullptr) && catalogueValid;
             module = module->NextSiblingElement("module"))
        {
            const char* moduleName = module->Attribute("name");
            const char* moduleDefault = module->Attribute("defaultState");
            const boolean moduleAttributesPresent = (moduleName != nullptr) && (moduleDefault != nullptr);
            const stringview moduleState = moduleDefault == nullptr ? "" : moduleDefault;
            const boolean moduleStateValid = (moduleState == "enable") || (moduleState == "disable");
            catalogueValid = moduleAttributesPresent && moduleStateValid;
            const boolean moduleEnabled = moduleState == "enable";
            if (catalogueValid)
            {
                loadedModules[string(moduleName)] = moduleEnabled;
            }
            for (const tinyxml2::XMLElement* trace = module->FirstChildElement("trace");
                 (trace != nullptr) && catalogueValid;
                 trace = trace->NextSiblingElement("trace"))
            {
                const char* uid = trace->Attribute("fullId");
                const char* numericId = trace->Attribute("id");
                const char* defaultState = trace->Attribute("defaultState");
                const char* sourceFile = trace->Attribute("sourceFile");
                unsigned int sourceLine = 0U;
                unsigned int priority = 0U;
                const stringview traceState = defaultState == nullptr ? "" : defaultState;
                const boolean traceStateValid = (traceState == "enable") || (traceState == "disable");
                const string numericIdText = numericId == nullptr ? "" : numericId;
                const size firstNonZero = numericIdText.find_first_not_of('0');
                const boolean numericIdIsZero = firstNonZero == string::npos;
                const string normalizedNumericId = numericIdIsZero ? "0" : numericIdText.substr(firstNonZero);
                const string scopedNumericId = string(moduleName) + "." + normalizedNumericId;
                const boolean numericIdUnique =
                    (numericId != nullptr) &&
                    (loadedScopedNumericIds.find(scopedNumericId) == loadedScopedNumericIds.end());
                const boolean attributesValid =
                    (uid != nullptr) && (numericId != nullptr) && (defaultState != nullptr) &&
                    (sourceFile != nullptr) && isValidUid(uid) && traceStateValid && numericIdUnique &&
                    (string(uid) == string(moduleName) + "." + numericId) &&
                    (trace->QueryUnsignedAttribute("sourceLine", &sourceLine) == tinyxml2::XML_SUCCESS) &&
                    (sourceLine > 0U) &&
                    (trace->QueryUnsignedAttribute("priority", &priority) == tinyxml2::XML_SUCCESS) &&
                    (priority < XHAL_RPI5CAR_TRACE_PRIORITY_COUNT) &&
                    (loadedLocations.find(uid) == loadedLocations.end());
                if (attributesValid == false)
                {
                    catalogueValid = false;
                    break;
                }
                loadedLocations[string(uid)] = {
                    string(sourceFile), static_cast<uint32>(sourceLine), static_cast<uint8>(priority)};
                loadedScopedNumericIds[scopedNumericId] = true;
                const boolean traceEnabled = traceState == "enable";
                if (traceEnabled != moduleEnabled)
                {
                    loadedTraces[string(uid)] = traceEnabled;
                }
            }
        }
        if (catalogueValid == false)
        {
            writeConfigurationDiagnostic("ERROR", "Trace catalogue values are invalid; all normal traces are disabled");
            return;
        }
        globalTraceEnabledValue = globalState == "enable";
        traceSourceLocations = loadedLocations;
        moduleEnabledValues = loadedModules;
        traceEnabledValues = loadedTraces;
    }

    /** @brief Writes one unfiltered trace-system configuration diagnostic. */
    void XWalkTrace::writeConfigurationDiagnostic(stringview category, stringview message)
    {
        writeRecordLocked("TRACE", category, "", "xWalkTrace", 0U, message);
    }

} /* namespace xwalk::hal */
