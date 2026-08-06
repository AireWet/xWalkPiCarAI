/******************************************************************************
 * @file        xHal_Rpi5CarTrace.cpp
 * @brief       Implements trace filtering and synchronous record delivery.
 *
 * @details
 * Applies the configured severity threshold, reports level changes, and
 * forwards accepted records to the caller-provided output callback.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Reports whether a severity passes the configured threshold.
 *
 * @param[in] level
 * Valid severity being considered.
 *
 * @return
 * `true` when the record must be forwarded; otherwise `false`.
 */
boolean XWalkTrace::accepts(XWalkTraceLevel level) const noexcept
{
    const uint8 requestedLevel = static_cast<uint8>(level);
    const uint8 configuredLevel = static_cast<uint8>(levelValue);
    return requestedLevel <= configuredLevel;
}

/**
 * @brief Forwards one record when its severity passes the threshold.
 *
 * @param[in] level
 * Valid severity assigned to the record.
 *
 * @param[in] message
 * Message view that remains valid throughout the synchronous callback.
 *
 * @note
 * Any exception raised by the injected callback is propagated.
 */
void XWalkTrace::write(XWalkTraceLevel level, stringview message) const
{
    const hal::boolean levelAccepted =
        static_cast<hal::boolean>(
            accepts(level));
    if (levelAccepted)
    {
        outputCallback(outputContextPointer, level, message);
    }
}

/**
 * @brief Reports a level change through the debug channel when enabled.
 *
 * @post
 * A debug record is delivered only when the new threshold accepts debug.
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

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Selects a typed severity threshold.
 *
 * @param[in] level
 * Highest numeric severity accepted by the filter.
 *
 * @post
 * `level()` equals `level`.
 *
 * @throws std::out_of_range
 * If `level` does not identify a supported severity.
 */
void XWalkTrace::setLevel(XWalkTraceLevel level)
{
    levelValue = validateLevel(level);
    reportLevelChange();
}

/**
 * @brief Selects a numeric Python-compatible severity threshold.
 *
 * @param[in] level
 * Severity number in the inclusive range zero through four.
 *
 * @post
 * `level()` equals the corresponding typed level.
 *
 * @throws std::out_of_range
 * If `level` is greater than four.
 */
void XWalkTrace::setLevel(uint8 level)
{
    setLevel(parseLevel(level));
}

/**
 * @brief Selects a textual Python-compatible severity threshold.
 *
 * @param[in] levelName
 * One of `critical`, `error`, `warning`, `info`, or `debug`.
 *
 * @post
 * `level()` equals the corresponding typed level.
 *
 * @throws std::invalid_argument
 * If `levelName` is unsupported.
 */
void XWalkTrace::setLevel(stringview levelName)
{
    setLevel(parseLevel(levelName));
}

/**
 * @brief Returns the configured typed severity threshold.
 *
 * @return
 * Highest numeric severity currently accepted by the filter.
 */
XWalkTraceLevel XWalkTrace::level() const noexcept
{
    return levelValue;
}

/**
 * @brief Returns the lowercase name of the configured threshold.
 *
 * @return
 * Static non-owning view containing the Python-compatible level name.
 */
stringview XWalkTrace::levelName() const noexcept
{
    switch (levelValue)
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
 * @brief Emits a critical record when enabled.
 *
 * @param[in] message
 * Non-owning record text consumed synchronously by the callback.
 */
void XWalkTrace::critical(stringview message) const
{
    write(XWalkTraceLevel::Critical, message);
}

/**
 * @brief Emits an error record when enabled.
 *
 * @param[in] message
 * Non-owning record text consumed synchronously by the callback.
 */
void XWalkTrace::error(stringview message) const
{
    write(XWalkTraceLevel::Error, message);
}

/**
 * @brief Emits a warning record when enabled.
 *
 * @param[in] message
 * Non-owning record text consumed synchronously by the callback.
 */
void XWalkTrace::warning(stringview message) const
{
    write(XWalkTraceLevel::Warning, message);
}

/**
 * @brief Emits an informational record when enabled.
 *
 * @param[in] message
 * Non-owning record text consumed synchronously by the callback.
 */
void XWalkTrace::info(stringview message) const
{
    write(XWalkTraceLevel::Info, message);
}

/**
 * @brief Emits a debug record when enabled.
 *
 * @param[in] message
 * Non-owning record text consumed synchronously by the callback.
 */
void XWalkTrace::debug(stringview message) const
{
    write(XWalkTraceLevel::Debug, message);
}

} /* namespace xwalk::hal */
