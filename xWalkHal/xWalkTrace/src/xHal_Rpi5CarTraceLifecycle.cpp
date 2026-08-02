/******************************************************************************
 * @file        xHal_Rpi5CarTraceLifecycle.cpp
 * @brief       Implements xWalk trace validation and lifecycle behavior.
 *
 * @details
 * Validates typed, numeric, and textual severity selections and binds a
 * caller-provided synchronous output callback without taking ownership.
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
 * @brief Converts and validates one numeric Python-compatible level.
 *
 * @param[in] level
 * Severity number in the inclusive range zero through four.
 *
 * @return
 * Corresponding typed trace level.
 *
 * @throws std::out_of_range
 * If `level` is greater than four.
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
 * @brief Converts and validates one lowercase Python-compatible level name.
 *
 * @param[in] levelName
 * One of `critical`, `error`, `warning`, `info`, or `debug`.
 *
 * @return
 * Corresponding typed trace level.
 *
 * @throws std::invalid_argument
 * If `levelName` is not supported.
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
 * @brief Validates a typed trace level.
 *
 * @param[in] level
 * Trace severity to validate.
 *
 * @return
 * The validated level.
 *
 * @throws std::out_of_range
 * If `level` does not identify a supported severity.
 */
XWalkTraceLevel XWalkTrace::validateLevel(XWalkTraceLevel level)
{
    return parseLevel(static_cast<uint8>(level));
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a trace interface with a typed severity threshold.
 *
 * @param[in,out] outputContext
 * Non-owning callback context; null is permitted for stateless callbacks.
 *
 * @param[in] output
 * Non-null synchronous record-output callback.
 *
 * @param[in] level
 * Initial severity threshold; warning is used by default.
 *
 * @post
 * Records at or above the configured urgency are forwarded synchronously.
 *
 * @throws std::invalid_argument
 * If `output` is null.
 *
 * @throws std::out_of_range
 * If `level` does not identify a supported severity.
 */
XWalkTrace::XWalkTrace(contextpointer outputContext, traceoutputcallback output,
    XWalkTraceLevel level):
    outputContextPointer(outputContext), outputCallback(output), levelValue(validateLevel(level))
{
    if (outputCallback == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Trace output callback must not be null");
    }
    reportLevelChange();
}

/**
 * @brief Constructs a trace interface from a numeric severity threshold.
 *
 * @param[in,out] outputContext
 * Non-owning callback context; null is permitted for stateless callbacks.
 *
 * @param[in] output
 * Non-null synchronous record-output callback.
 *
 * @param[in] level
 * Initial Python-compatible severity number from zero through four.
 *
 * @throws std::invalid_argument
 * If `output` is null.
 *
 * @throws std::out_of_range
 * If `level` is greater than four.
 */
XWalkTrace::XWalkTrace(contextpointer outputContext, traceoutputcallback output, uint8 level):
    XWalkTrace(outputContext, output, parseLevel(level))
{
}

/**
 * @brief Constructs a trace interface from a textual severity threshold.
 *
 * @param[in,out] outputContext
 * Non-owning callback context; null is permitted for stateless callbacks.
 *
 * @param[in] output
 * Non-null synchronous record-output callback.
 *
 * @param[in] levelName
 * Initial lowercase Python-compatible severity name.
 *
 * @throws std::invalid_argument
 * If `output` is null or `levelName` is unsupported.
 */
XWalkTrace::XWalkTrace(contextpointer outputContext, traceoutputcallback output, stringview levelName):
    XWalkTrace(outputContext, output, parseLevel(levelName))
{
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the trace interface without releasing its non-owning context.
 */
XWalkTrace::~XWalkTrace() = default;

} /* namespace xwalk::hal */
