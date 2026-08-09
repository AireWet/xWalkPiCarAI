/******************************************************************************
 * @file        xHal_Rpi5CarTraceLifecycle.cpp
 * @brief       Implements xWalk trace validation and lifecycle behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 *
 * @author      Joxy John
 * @date        2026-08-09
 * @version     2.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 ******************************************************************************/

#include "xHal_Rpi5CarTrace.h"

#include <filesystem>
#include <fstream>
#include <mutex>

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/**
 * @brief Converts one numeric compatibility severity.
 * @param[in] level Severity number in the inclusive range zero through four.
 * @return Corresponding validated trace level.
 * @throws std::out_of_range If `level` exceeds four.
 */
XWalkTraceLevel XWalkTrace::parseLevel(uint8 level)
{
    if (level >= XHAL_RPI5CAR_TRACE_LEVEL_COUNT)
    {
        XHAL_THROW_OUT_OF_RANGE("Trace level must be in range 0..4");
    }
    return static_cast<XWalkTraceLevel>(level);
}

/**
 * @brief Converts one lowercase compatibility severity name.
 * @param[in] levelName Supported critical-through-debug severity name.
 * @return Corresponding validated trace level.
 * @throws std::invalid_argument If `levelName` is unsupported.
 */
XWalkTraceLevel XWalkTrace::parseLevel(stringview levelName)
{
    if (levelName == XHAL_RPI5CAR_TRACE_LEVEL_CRITICAL_NAME)
    {
        return XWalkTraceLevel::Critical;
    }
    if (levelName == XHAL_RPI5CAR_TRACE_LEVEL_ERROR_NAME)
    {
        return XWalkTraceLevel::Error;
    }
    if (levelName == XHAL_RPI5CAR_TRACE_LEVEL_WARNING_NAME)
    {
        return XWalkTraceLevel::Warning;
    }
    if (levelName == XHAL_RPI5CAR_TRACE_LEVEL_INFO_NAME)
    {
        return XWalkTraceLevel::Info;
    }
    if (levelName == XHAL_RPI5CAR_TRACE_LEVEL_DEBUG_NAME)
    {
        return XWalkTraceLevel::Debug;
    }
    XHAL_THROW_INVALID_ARGUMENT("Trace level name is unsupported");
}

/**
 * @brief Validates one typed compatibility severity.
 * @param[in] level Typed severity to validate.
 * @return The validated input severity.
 * @throws std::out_of_range If the underlying numeric value exceeds four.
 */
XWalkTraceLevel XWalkTrace::validateLevel(XWalkTraceLevel level)
{
    return parseLevel(static_cast<uint8>(level));
}

/**
 * @brief Consumes a global-instance callback record without output.
 * @param[in,out] context Unused null callback context.
 * @param[in] level Unused emitted compatibility level.
 * @param[in] message Unused non-owning record text.
 */
void XWalkTrace::discardOutput(contextpointer context, XWalkTraceLevel level,
    stringview message) noexcept
{
    static_cast<void>(context);
    static_cast<void>(level);
    static_cast<void>(message);
}

/**
 * @brief Returns the lazily initialized process-wide macro trace instance.
 * @return Stable instance shared by every public trace macro.
 */
XWalkTrace& XWalkTrace::globalInstance()
{
    static XWalkTrace instance(nullptr, &XWalkTrace::discardOutput);
    return instance;
}

/**
 * @brief Constructs a compatibility trace and loads the generated XML once.
 * @param[in,out] outputContext Optional non-owning callback context.
 * @param[in] output Non-null synchronous callback.
 * @param[in] level Initial validated compatibility threshold.
 * @throws std::invalid_argument If `output` is null.
 * @throws filesystemerror If the default log directory cannot be created.
 * @throws std::runtime_error If the default log cannot be opened.
 */
XWalkTrace::XWalkTrace(contextpointer outputContext, traceoutputcallback output,
    XWalkTraceLevel level):
    outputContextPointer(outputContext),
    outputCallback(output),
    logFile(),
    priorityEnabledValues{},
    traceEnabledValues{},
    configurationPathValue(),
    startTime(steadyclock::now()),
    levelValue(validateLevel(level)),
    traceMutex()
{
    if (outputCallback == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Trace output callback must not be null");
    }

    const filesystempath defaultLogPath =
        filesystempath(XHAL_RPI5CAR_TRACE_LOG_DIRECTORY) /
        XHAL_RPI5CAR_TRACE_LOG_FILENAME;
    initialize(filesystempath(XWALK_TRACE_CONFIG_PATH), defaultLogPath);
    reportLevelChange();
}

/**
 * @brief Constructs a compatibility trace from a numeric severity.
 * @param[in,out] outputContext Optional non-owning callback context.
 * @param[in] output Non-null synchronous callback.
 * @param[in] level Severity number in the inclusive range zero through four.
 */
XWalkTrace::XWalkTrace(contextpointer outputContext, traceoutputcallback output,
    uint8 level):
    XWalkTrace(outputContext, output, parseLevel(level))
{
}

/**
 * @brief Constructs a compatibility trace from a lowercase severity name.
 * @param[in,out] outputContext Optional non-owning callback context.
 * @param[in] output Non-null synchronous callback.
 * @param[in] levelName Supported critical-through-debug severity name.
 */
XWalkTrace::XWalkTrace(contextpointer outputContext, traceoutputcallback output,
    stringview levelName):
    XWalkTrace(outputContext, output, parseLevel(levelName))
{
}

/** @brief Closes the owned file stream through scope-bound destruction. */
XWalkTrace::~XWalkTrace() = default;

/**
 * @brief Reloads and redirects the process-wide macro trace instance.
 * @param[in] configurationPath XML configuration loaded under synchronization.
 * @param[in] logPath Append-only log destination opened under synchronization.
 */
void XWalkTrace::configureGlobal(const filesystempath& configurationPath,
    const filesystempath& logPath)
{
    XWalkTrace& instance = globalInstance();
    mutexlock lock(instance.traceMutex);
    instance.initialize(configurationPath, logPath);
}

} /* namespace xwalk::hal */
