/******************************************************************************
 * @file        xHal_Rpi5CarTraceControl.cpp
 * @brief       Implements persistent boot-time trace enable and disable control.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 * @author      Joxy John
 * @date        2026-08-09
 * @version     2.1.0
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

/**
 * @brief Persists one existing trace flag through a same-directory temporary file.
 * @param[in] uid Complete trace identifier to update.
 * @param[in] enabled New individual trace state.
 * @return `true` after persistence and lookup update; otherwise `false`.
 */
boolean XWalkTrace::setTraceEnabled(stringview uid, boolean enabled)
{
    if (isValidUid(uid) == false)
    {
        return false;
    }

    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError loadResult =
        document.LoadFile(configurationPathValue.string().c_str());
    if (loadResult != tinyxml2::XML_SUCCESS)
    {
        return false;
    }

    tinyxml2::XMLElement* root = document.FirstChildElement("xwalkTrace");
    tinyxml2::XMLElement* traces =
        root == nullptr ? nullptr : root->FirstChildElement("traces");
    if (traces == nullptr)
    {
        return false;
    }

    tinyxml2::XMLElement* matchedTrace = nullptr;
    tinyxml2::XMLElement* trace = traces->FirstChildElement("trace");
    while (trace != nullptr)
    {
        const char* uidAttribute = trace->Attribute("uid");
        const boolean uidMatched = (uidAttribute != nullptr) && (uid == uidAttribute);
        if (uidMatched)
        {
            matchedTrace = trace;
            break;
        }
        trace = trace->NextSiblingElement("trace");
    }
    if (matchedTrace == nullptr)
    {
        return false;
    }

    matchedTrace->SetAttribute("enabled", enabled);
    filesystempath temporaryPath = configurationPathValue;
    temporaryPath += ".tmp";
    const tinyxml2::XMLError saveResult =
        document.SaveFile(temporaryPath.string().c_str());
    if (saveResult != tinyxml2::XML_SUCCESS)
    {
        return false;
    }

    errorcode renameError;
    std::filesystem::rename(temporaryPath, configurationPathValue, renameError);
    if (renameError)
    {
        errorcode removeError;
        static_cast<void>(std::filesystem::remove(temporaryPath, removeError));
        return false;
    }
    traceEnabledValues[string(uid)] = enabled;
    return true;
}

/**
 * @brief Enables one known trace UID in the process-wide XML configuration.
 * @param[in] uid Complete UID already present in generated XML.
 * @return `true` after persistence succeeds; otherwise `false`.
 */
boolean XWalkTrace::enableGlobalTrace(stringview uid)
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    return instance.setTraceEnabled(uid, true);
}

/**
 * @brief Disables one known trace UID in the process-wide XML configuration.
 * @param[in] uid Complete UID already present in generated XML.
 * @return `true` after persistence succeeds; otherwise `false`.
 */
boolean XWalkTrace::disableGlobalTrace(stringview uid)
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    return instance.setTraceEnabled(uid, false);
}

} /* namespace xwalk::hal */
