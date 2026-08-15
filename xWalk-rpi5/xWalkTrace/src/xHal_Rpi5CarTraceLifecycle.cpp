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
#include "xHal_Rpi5CarTraceBuildConfig.h"

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
            throw outofrange("Trace level must be in range 0..4");
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
        throw invalidargument("Trace level name is unsupported");
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
    void XWalkTrace::discardOutput(contextpointer context, XWalkTraceLevel level, stringview message) noexcept
    {
        static_cast<void>(context);
        static_cast<void>(level);
        static_cast<void>(message);
    }

    /**
     * @brief Returns the lazily initialized process-wide macro trace instance.
     * @param[in] configurationPath Optional first-use catalogue path.
     * @param[in] logPath Optional first-use log path.
     * @return Stable instance shared by every public trace macro.
     */
    XWalkTrace& XWalkTrace::globalInstance(const filesystempath* configurationPath, const filesystempath* logPath)
    {
        const filesystempath defaultConfigurationPath(XWALK_TRACE_CONFIG_PATH);
        const filesystempath defaultLogPath =
            filesystempath(XHAL_RPI5CAR_TRACE_LOG_DIRECTORY) / XHAL_RPI5CAR_TRACE_LOG_FILENAME;
        const filesystempath& initialConfigurationPath =
            configurationPath == nullptr ? defaultConfigurationPath : *configurationPath;
        const filesystempath& initialLogPath = logPath == nullptr ? defaultLogPath : *logPath;
        static XWalkTrace instance(
            nullptr, &XWalkTrace::discardOutput, XWalkTraceLevel::Warning, initialConfigurationPath, initialLogPath);
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
    XWalkTrace::XWalkTrace(contextpointer outputContext, traceoutputcallback output, XWalkTraceLevel level)
        : XWalkTrace(outputContext,
                     output,
                     level,
                     filesystempath(XWALK_TRACE_CONFIG_PATH),
                     filesystempath(XHAL_RPI5CAR_TRACE_LOG_DIRECTORY) / XHAL_RPI5CAR_TRACE_LOG_FILENAME)
    {
    }

    /**
     * @brief Constructs a trace with explicit initial catalogue and log paths.
     * @param[in,out] outputContext Optional non-owning callback context.
     * @param[in] output Non-null synchronous callback.
     * @param[in] level Initial validated compatibility threshold.
     * @param[in] configurationPath XML catalogue loaded during construction.
     * @param[in] logPath Append-only log opened during construction.
     */
    XWalkTrace::XWalkTrace(contextpointer outputContext,
                           traceoutputcallback output,
                           XWalkTraceLevel level,
                           const filesystempath& configurationPath,
                           const filesystempath& logPath)
        : outputContextPointer(outputContext), outputCallback(output), logFile(), globalTraceEnabledValue(false),
          moduleEnabledValues{}, traceEnabledValues{}, traceSourceLocations{}, configurationPathValue(), logPathValue(),
          traceConfigurationErrorValue(), startTime(steadyclock::now()), levelValue(validateLevel(level)), traceMutex()
    {
        if (outputCallback == nullptr)
        {
            throw invalidargument("Trace output callback must not be null");
        }

        initialize(configurationPath, logPath);
        reportLevelChange();
    }

    /**
     * @brief Constructs a compatibility trace from a numeric severity.
     * @param[in,out] outputContext Optional non-owning callback context.
     * @param[in] output Non-null synchronous callback.
     * @param[in] level Severity number in the inclusive range zero through four.
     */
    XWalkTrace::XWalkTrace(contextpointer outputContext, traceoutputcallback output, uint8 level)
        : XWalkTrace(outputContext, output, parseLevel(level))
    {
    }

    /**
     * @brief Constructs a compatibility trace from a lowercase severity name.
     * @param[in,out] outputContext Optional non-owning callback context.
     * @param[in] output Non-null synchronous callback.
     * @param[in] levelName Supported critical-through-debug severity name.
     */
    XWalkTrace::XWalkTrace(contextpointer outputContext, traceoutputcallback output, stringview levelName)
        : XWalkTrace(outputContext, output, parseLevel(levelName))
    {
    }

    /** @brief Closes the owned file stream through scope-bound destruction. */
    XWalkTrace::~XWalkTrace() = default;

    /**
     * @brief Reloads and redirects the process-wide macro trace instance.
     * @param[in] configurationPath XML configuration loaded under synchronization.
     * @param[in] logPath Append-only log destination opened under synchronization.
     */
    void XWalkTrace::configureGlobal(const filesystempath& configurationPath, const filesystempath& logPath)
    {
        XWalkTrace& instance = globalInstance(&configurationPath, &logPath);
        mutexlock lock(instance.traceMutex);
        const boolean pathsChanged =
            (instance.configurationPathValue != configurationPath) || (instance.logPathValue != logPath);
        if (pathsChanged)
        {
            instance.initialize(configurationPath, logPath);
        }
    }

} /* namespace xwalk::hal */
