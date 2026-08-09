/******************************************************************************
 * @file        xHal_Rpi5CarTrace.cpp
 * @brief       Implements xWalk trace filtering and record dispatch.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 * @author      Joxy John
 * @date        2026-08-09
 * @version     2.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarTrace.h"

#include <mutex>
#include <sstream>
#include <string>

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/**
 * @brief Returns the stable lowercase name of one compatibility severity.
 * @param[in] level Valid critical-through-debug severity.
 * @return Static non-owning severity name.
 */
stringview XWalkTrace::nameForLevel(XWalkTraceLevel level) noexcept
{
    switch (level)
    {
        case XWalkTraceLevel::Critical:
            return XHAL_RPI5CAR_TRACE_LEVEL_CRITICAL_NAME;
        case XWalkTraceLevel::Error:
            return XHAL_RPI5CAR_TRACE_LEVEL_ERROR_NAME;
        case XWalkTraceLevel::Warning:
            return XHAL_RPI5CAR_TRACE_LEVEL_WARNING_NAME;
        case XWalkTraceLevel::Info:
            return XHAL_RPI5CAR_TRACE_LEVEL_INFO_NAME;
        case XWalkTraceLevel::Debug:
            return XHAL_RPI5CAR_TRACE_LEVEL_DEBUG_NAME;
        default:
            return XHAL_RPI5CAR_TRACE_LEVEL_WARNING_NAME;
    }
}

/**
 * @brief Tests one compatibility severity against the configured threshold.
 * @param[in] level Severity being considered.
 * @return `true` when its numeric value is no greater than the threshold.
 */
boolean XWalkTrace::accepts(XWalkTraceLevel level) const noexcept
{
    return static_cast<uint8>(level) <= static_cast<uint8>(levelValue);
}

/**
 * @brief Tests process-wide priority and UID configuration without formatting.
 * @param[in] priority Tagged priority from zero through three.
 * @param[in] uid Complete tagged-trace identifier.
 * @return `true` only when the priority and known UID are both enabled.
 */
boolean XWalkTrace::globalTraceIsEnabled(uint8 priority, stringview uid)
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    const boolean priorityValid = priority < XHAL_RPI5CAR_TRACE_PRIORITY_COUNT;
    if (priorityValid == false)
    {
        return false;
    }

    const boolean priorityEnabled = instance.priorityEnabledValues[priority];
    if (priorityEnabled == false)
    {
        return false;
    }

    const auto trace = instance.traceEnabledValues.find(string(uid));
    const boolean traceKnown = trace != instance.traceEnabledValues.end();
    return traceKnown && trace->second;
}

/**
 * @brief Rechecks and writes one tagged record through this instance.
 * @param[in] priority Tagged priority from zero through three.
 * @param[in] component `HAL` or `CTRL` component label.
 * @param[in] uid Complete scanner-validated identifier.
 * @param[in] sourceFile Compiler-provided caller source path.
 * @param[in] sourceLine One-based public macro invocation line.
 * @param[in] message Fully formatted message.
 */
void XWalkTrace::writeTagged(uint8 priority, stringview component, stringview uid,
    stringview sourceFile, uint32 sourceLine, stringview message)
{
    {
        mutexlock lock(traceMutex);
        const boolean priorityValid = priority < XHAL_RPI5CAR_TRACE_PRIORITY_COUNT;
        if (priorityValid == false)
        {
            return;
        }

        const auto trace = traceEnabledValues.find(string(uid));
        const boolean traceKnown = trace != traceEnabledValues.end();
        const boolean traceEnabled = traceKnown && trace->second;
        const boolean recordEnabled = priorityEnabledValues[priority] && traceEnabled;
        if (recordEnabled == false)
        {
            return;
        }

        const string priorityCategory = string("P") + std::to_string(priority);
        writeRecordLocked(component, priorityCategory, uid, sourceFile, sourceLine, message);
    }
    const XWalkTraceLevel callbackLevel = static_cast<XWalkTraceLevel>(priority);
    outputCallback(outputContextPointer, callbackLevel, message);
}

/**
 * @brief Writes one unfiltered warning, error, assertion, or verbose record.
 * @param[in] component `HAL`, `CTRL`, or `TRACE` component label.
 * @param[in] category `WARNING`, `ERROR`, `ASSERT`, or `VERBOSE`.
 * @param[in] sourceFile Compiler-provided caller source path.
 * @param[in] sourceLine One-based public macro invocation line.
 * @param[in] message Fully formatted message or assertion signal text.
 */
void XWalkTrace::writeCategory(stringview component, stringview category,
    stringview sourceFile, uint32 sourceLine, stringview message)
{
    {
        mutexlock lock(traceMutex);
        writeRecordLocked(component, category, "", sourceFile, sourceLine, message);
    }
    const boolean warningCategory = category == "WARNING";
    const XWalkTraceLevel callbackLevel = warningCategory ?
        XWalkTraceLevel::Warning : XWalkTraceLevel::Error;
    outputCallback(outputContextPointer, callbackLevel, message);
}

/**
 * @brief Converts and writes one numeric process-wide assertion signal.
 * @param[in] component `HAL` or `CTRL` component label.
 * @param[in] signalNumber Application-defined numeric assertion signal.
 * @param[in] sourceFile Compiler-provided caller source path.
 * @param[in] sourceLine One-based public macro invocation line.
 */
void XWalkTrace::globalWriteAssertion(stringview component, int32 signalNumber,
    stringview sourceFile, uint32 sourceLine)
{
    std::ostringstream message;
    message << "signal=" << signalNumber;
    globalInstance().writeCategory(component, "ASSERT", sourceFile, sourceLine,
        message.str());
}

/**
 * @brief Writes one accepted compatibility record and invokes its callback.
 * @param[in] level Compatibility severity assigned to the record.
 * @param[in] message Non-owning message valid through the callback invocation.
 */
void XWalkTrace::write(XWalkTraceLevel level, stringview message) const
{
    {
        mutexlock lock(traceMutex);
        const boolean levelAccepted = accepts(level);
        if (levelAccepted == false)
        {
            return;
        }
        string category(nameForLevel(level));
        for (char& character : category)
        {
            const boolean lowercaseLetter = (character >= 'a') && (character <= 'z');
            if (lowercaseLetter)
            {
                character = static_cast<char>(character - ('a' - 'A'));
            }
        }
        writeRecordLocked("LEGACY", category, "", "legacy", 0U, message);
    }
    outputCallback(outputContextPointer, level, message);
}

/**
 * @brief Emits the compatibility threshold-change message at debug severity.
 * @post The message remains filtered unless debug is accepted.
 */
void XWalkTrace::reportLevelChange() const
{
    switch (levelValue)
    {
        case XWalkTraceLevel::Critical:
            debug(XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_PREFIX
                XHAL_RPI5CAR_TRACE_LEVEL_CRITICAL_NAME XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_SUFFIX);
            break;
        case XWalkTraceLevel::Error:
            debug(XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_PREFIX
                XHAL_RPI5CAR_TRACE_LEVEL_ERROR_NAME XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_SUFFIX);
            break;
        case XWalkTraceLevel::Warning:
            debug(XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_PREFIX
                XHAL_RPI5CAR_TRACE_LEVEL_WARNING_NAME XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_SUFFIX);
            break;
        case XWalkTraceLevel::Info:
            debug(XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_PREFIX
                XHAL_RPI5CAR_TRACE_LEVEL_INFO_NAME XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_SUFFIX);
            break;
        case XWalkTraceLevel::Debug:
            debug(XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_PREFIX
                XHAL_RPI5CAR_TRACE_LEVEL_DEBUG_NAME XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_SUFFIX);
            break;
        default:
            break;
    }
}

/**
 * @brief Selects one typed compatibility threshold.
 * @param[in] level Valid critical-through-debug severity.
 * @throws std::out_of_range If `level` has an unsupported underlying value.
 */
void XWalkTrace::setLevel(XWalkTraceLevel level)
{
    {
        mutexlock lock(traceMutex);
        levelValue = validateLevel(level);
    }
    reportLevelChange();
}

/**
 * @brief Selects one numeric compatibility threshold.
 * @param[in] level Severity number in the inclusive range zero through four.
 * @throws std::out_of_range If `level` exceeds four.
 */
void XWalkTrace::setLevel(uint8 level)
{
    setLevel(parseLevel(level));
}

/**
 * @brief Selects one lowercase compatibility threshold name.
 * @param[in] levelName Supported critical-through-debug name.
 * @throws std::invalid_argument If `levelName` is unsupported.
 */
void XWalkTrace::setLevel(stringview levelName)
{
    setLevel(parseLevel(levelName));
}

/**
 * @brief Returns the configured typed compatibility threshold.
 * @return Current critical-through-debug threshold.
 */
XWalkTraceLevel XWalkTrace::level() const noexcept
{
    return levelValue;
}

/**
 * @brief Returns the configured lowercase compatibility threshold name.
 * @return Static non-owning current threshold name.
 */
stringview XWalkTrace::levelName() const noexcept
{
    return nameForLevel(levelValue);
}

/**
 * @brief Emits one compatibility critical record when accepted.
 * @param[in] message Non-owning text consumed synchronously.
 */
void XWalkTrace::critical(stringview message) const
{
    write(XWalkTraceLevel::Critical, message);
}

/**
 * @brief Emits one compatibility error record when accepted.
 * @param[in] message Non-owning text consumed synchronously.
 */
void XWalkTrace::error(stringview message) const
{
    write(XWalkTraceLevel::Error, message);
}

/**
 * @brief Emits one compatibility warning record when accepted.
 * @param[in] message Non-owning text consumed synchronously.
 */
void XWalkTrace::warning(stringview message) const
{
    write(XWalkTraceLevel::Warning, message);
}

/**
 * @brief Emits one compatibility informational record when accepted.
 * @param[in] message Non-owning text consumed synchronously.
 */
void XWalkTrace::info(stringview message) const
{
    write(XWalkTraceLevel::Info, message);
}

/**
 * @brief Emits one compatibility debug record when accepted.
 * @param[in] message Non-owning text consumed synchronously.
 */
void XWalkTrace::debug(stringview message) const
{
    write(XWalkTraceLevel::Debug, message);
}

} /* namespace xwalk::hal */
