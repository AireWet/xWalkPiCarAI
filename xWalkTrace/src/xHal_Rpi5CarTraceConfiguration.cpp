/******************************************************************************
 * @file        xHal_Rpi5CarTraceConfiguration.cpp
 * @brief       Loads xWalk trace priority and UID configuration once.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 * @author      Joxy John
 * @date        2026-08-09
 * @version     2.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarTrace.h"

#include <filesystem>
#include <fstream>
#include <tinyxml2.h>

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/**
 * @brief Validates one complete tagged-trace identifier.
 *
 * @param[in] uid
 * Candidate identifier to validate without allocation.
 *
 * @return
 * `true` for `RPI.<digits>` or `CTRL.<digits>`, including identifiers whose
 * numeric part has leading zeros; otherwise `false`.
 */
boolean XWalkTrace::isValidUid(stringview uid) noexcept
{
    const size separatorPosition = uid.find('.');
    const boolean separatorValid =
        (separatorPosition != stringview::npos) && (separatorPosition > 0U);
    if (separatorValid == false)
    {
        return false;
    }

    const stringview tag = uid.substr(0U, separatorPosition);
    const boolean tagValid = (tag == "RPI") || (tag == "CTRL");
    if (tagValid == false)
    {
        return false;
    }

    const stringview number = uid.substr(separatorPosition + 1U);
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
 * @brief Reopens the append-only log and loads one XML configuration.
 *
 * @param[in] configurationPath
 * XML file read once for priority and UID enable flags.
 *
 * @param[in] logPath
 * Log file whose parent directories are created when needed.
 *
 * @throws filesystemerror
 * If a required log directory cannot be created.
 *
 * @throws std::runtime_error
 * If the log file cannot be opened for append.
 */
void XWalkTrace::initialize(const filesystempath& configurationPath,
    const filesystempath& logPath)
{
    configurationPathValue = configurationPath;
    priorityEnabledValues.fill(false);
    traceEnabledValues.clear();
    startTime = steadyclock::now();

    const filesystempath logDirectory = logPath.parent_path();
    const boolean logDirectoryProvided = logDirectory.empty() == false;
    if (logDirectoryProvided)
    {
        static_cast<void>(createDirectories(logDirectory));
    }

    const boolean previousLogOpen = logFile.is_open();
    if (previousLogOpen)
    {
        logFile.close();
    }
    logFile.clear();
    logFile.open(logPath, FILE_OPEN_WRITE_APPEND);
    const boolean logOpenFailed = logFile.is_open() == false;
    if (logOpenFailed)
    {
        XHAL_THROW_RUNTIME_ERROR("Trace log file could not be opened for append");
    }

    loadConfiguration(configurationPath);
}

/**
 * @brief Loads configuration into temporary containers and commits it atomically.
 *
 * @param[in] configurationPath
 * XML file containing `priorities` and `traces` elements.
 *
 * @post
 * Missing or invalid input leaves every priority and UID disabled and writes a
 * configuration diagnostic.
 */
void XWalkTrace::loadConfiguration(const filesystempath& configurationPath)
{
    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError loadResult = document.LoadFile(configurationPath.string().c_str());
    const boolean configurationMissing = loadResult == tinyxml2::XML_ERROR_FILE_NOT_FOUND;
    if (configurationMissing)
    {
        writeConfigurationDiagnostic("WARNING",
            "Trace XML is missing; all tagged traces are disabled");
        return;
    }

    const boolean configurationLoadFailed = loadResult != tinyxml2::XML_SUCCESS;
    if (configurationLoadFailed)
    {
        writeConfigurationDiagnostic("ERROR",
            "Trace XML is malformed; all tagged traces are disabled");
        return;
    }

    const tinyxml2::XMLElement* root = document.FirstChildElement("xwalkTrace");
    const boolean rootMissing = root == nullptr;
    if (rootMissing)
    {
        writeConfigurationDiagnostic("ERROR",
            "Trace XML root is invalid; all tagged traces are disabled");
        return;
    }

    fixedarray<boolean, XHAL_RPI5CAR_TRACE_PRIORITY_COUNT> loadedPriorities{};
    orderedmap<string, boolean> loadedTraces{};
    boolean configurationValid = true;

    const tinyxml2::XMLElement* priorities = root->FirstChildElement("priorities");
    const boolean prioritiesMissing = priorities == nullptr;
    if (prioritiesMissing)
    {
        configurationValid = false;
    }
    else
    {
        const tinyxml2::XMLElement* priority = priorities->FirstChildElement("priority");
        while (priority != nullptr)
        {
            unsigned int level = XHAL_RPI5CAR_TRACE_PRIORITY_COUNT;
            boolean enabled = false;
            const tinyxml2::XMLError levelResult = priority->QueryUnsignedAttribute(
                "level", &level);
            const tinyxml2::XMLError enabledResult = priority->QueryBoolAttribute(
                "enabled", &enabled);
            const boolean priorityInvalid =
                (levelResult != tinyxml2::XML_SUCCESS) ||
                (enabledResult != tinyxml2::XML_SUCCESS) ||
                (level >= XHAL_RPI5CAR_TRACE_PRIORITY_COUNT);
            if (priorityInvalid)
            {
                configurationValid = false;
                break;
            }
            loadedPriorities[static_cast<size>(level)] = enabled;
            priority = priority->NextSiblingElement("priority");
        }
    }

    const tinyxml2::XMLElement* traces = root->FirstChildElement("traces");
    const boolean tracesMissing = traces == nullptr;
    if (tracesMissing)
    {
        configurationValid = false;
    }
    else
    {
        const tinyxml2::XMLElement* trace = traces->FirstChildElement("trace");
        while (trace != nullptr)
        {
            const char* uidAttribute = trace->Attribute("uid");
            boolean enabled = false;
            const tinyxml2::XMLError enabledResult = trace->QueryBoolAttribute(
                "enabled", &enabled);
            const boolean uidMissing = uidAttribute == nullptr;
            const boolean uidInvalid = uidMissing ||
                (uidMissing == false && isValidUid(uidAttribute) == false);
            const boolean traceInvalid = uidInvalid ||
                (enabledResult != tinyxml2::XML_SUCCESS);
            if (traceInvalid)
            {
                configurationValid = false;
                break;
            }
            loadedTraces[string(uidAttribute)] = enabled;
            trace = trace->NextSiblingElement("trace");
        }
    }

    if (configurationValid == false)
    {
        writeConfigurationDiagnostic("ERROR",
            "Trace XML values are invalid; all tagged traces are disabled");
        return;
    }

    priorityEnabledValues = loadedPriorities;
    traceEnabledValues = loadedTraces;
}

/**
 * @brief Writes one unfiltered trace-system configuration diagnostic.
 *
 * @param[in] category
 * `WARNING` or `ERROR` category selected by the detected failure.
 *
 * @param[in] message
 * Human-readable safe-default explanation.
 */
void XWalkTrace::writeConfigurationDiagnostic(stringview category, stringview message)
{
    writeRecordLocked("TRACE", category, "", "xWalkTrace", 0U, message);
}

} /* namespace xwalk::hal */
