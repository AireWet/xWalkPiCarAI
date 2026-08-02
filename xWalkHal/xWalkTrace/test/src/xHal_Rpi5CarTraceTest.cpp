/******************************************************************************
 * @file        xHal_Rpi5CarTraceTest.cpp
 * @brief       Verifies trace validation, filtering, and callback delivery.
 *
 * @details
 * Exercises default filtering, every severity, numeric and textual selection,
 * level-change reporting, and invalid construction or configuration requests.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace Host Test
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

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains host-test state and scenarios private to this translation unit.
 */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Constants
 ******************************************************************************/

/** @brief Maximum number of trace records retained by one test capture. */
#define XHAL_RPI5CAR_TRACE_TEST_RECORD_CAPACITY 8U

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Retains bounded synchronous callback output for host assertions. */
struct TraceCapture
{
    /** @brief Ordered accepted severities retained by the callback. */
    fixedarray<XWalkTraceLevel, XHAL_RPI5CAR_TRACE_TEST_RECORD_CAPACITY> levels{};
    /** @brief Ordered owned message copies retained by the callback. */
    fixedarray<string, XHAL_RPI5CAR_TRACE_TEST_RECORD_CAPACITY> messages{};
    /** @brief Number of valid records currently stored in the arrays. */
    size count{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Copies one synchronous trace record into bounded test state.
 *
 * @param[in,out] context
 * Non-null pointer to a `TraceCapture` with remaining capacity.
 *
 * @param[in] level
 * Severity forwarded by the trace object.
 *
 * @param[in] message
 * Record text copied before the callback returns.
 */
void captureOutput(contextpointer context, XWalkTraceLevel level, stringview message)
{
    TraceCapture& capture = *static_cast<TraceCapture*>(context);
    assert(capture.count < XHAL_RPI5CAR_TRACE_TEST_RECORD_CAPACITY);
    capture.levels[capture.count] = level;
    capture.messages[capture.count] = string(message);
    ++capture.count;
}

/** @brief Verifies warning-default filtering and all severity entry points. */
void testDefaultFiltering()
{
    TraceCapture capture;
    XWalkTrace trace(&capture, &captureOutput);

    assert(trace.level() == XWalkTraceLevel::Warning);
    assert(trace.levelName() == XHAL_RPI5CAR_TRACE_LEVEL_WARNING_NAME);
    trace.debug("debug");
    trace.info("info");
    trace.warning("warning");
    trace.error("error");
    trace.critical("critical");

    assert(capture.count == 3U);
    assert(capture.levels[0U] == XWalkTraceLevel::Warning);
    assert(capture.messages[0U] == "warning");
    assert(capture.levels[1U] == XWalkTraceLevel::Error);
    assert(capture.messages[1U] == "error");
    assert(capture.levels[2U] == XWalkTraceLevel::Critical);
    assert(capture.messages[2U] == "critical");
}

/** @brief Verifies typed, numeric, and textual threshold selection. */
void testLevelSelection()
{
    TraceCapture capture;
    XWalkTrace trace(&capture, &captureOutput, XWalkTraceLevel::Info);

    trace.setLevel(static_cast<uint8>(XHAL_RPI5CAR_TRACE_LEVEL_DEBUG));
    assert(trace.level() == XWalkTraceLevel::Debug);
    assert(trace.levelName() == XHAL_RPI5CAR_TRACE_LEVEL_DEBUG_NAME);
    assert(capture.count == 1U);
    assert(capture.levels[0U] == XWalkTraceLevel::Debug);
    assert(capture.messages[0U] == "Set trace level to [debug]");

    trace.info("selected info");
    trace.setLevel(XHAL_RPI5CAR_TRACE_LEVEL_ERROR_NAME);
    trace.warning("filtered warning");
    trace.error("selected error");
    assert(trace.level() == XWalkTraceLevel::Error);
    assert(trace.levelName() == XHAL_RPI5CAR_TRACE_LEVEL_ERROR_NAME);
    assert(capture.count == 3U);
    assert(capture.messages[1U] == "selected info");
    assert(capture.messages[2U] == "selected error");

    trace.setLevel(XWalkTraceLevel::Critical);
    assert(trace.level() == XWalkTraceLevel::Critical);
}

/** @brief Verifies numeric and textual constructors preserve Python inputs. */
void testConstructorInputs()
{
    TraceCapture numericCapture;
    XWalkTrace numericTrace(&numericCapture, &captureOutput,
        static_cast<uint8>(XHAL_RPI5CAR_TRACE_LEVEL_CRITICAL));
    assert(numericTrace.level() == XWalkTraceLevel::Critical);
    assert(numericCapture.count == 0U);

    TraceCapture textCapture;
    XWalkTrace textTrace(&textCapture, &captureOutput, XHAL_RPI5CAR_TRACE_LEVEL_INFO_NAME);
    assert(textTrace.level() == XWalkTraceLevel::Info);
    assert(textTrace.levelName() == XHAL_RPI5CAR_TRACE_LEVEL_INFO_NAME);
    assert(textCapture.count == 0U);
}

/** @brief Verifies invalid callbacks and severity selections are rejected. */
void testValidation()
{
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkTrace trace(nullptr, nullptr);
        static_cast<void>(trace);
    });

    TraceCapture capture;
    XWalkTrace trace(&capture, &captureOutput);
    xwalk::hal::test::expectFailure([&]()
    {
        trace.setLevel(static_cast<uint8>(XHAL_RPI5CAR_TRACE_LEVEL_COUNT));
    });

    xwalk::hal::test::expectFailure([&]()
    {
        trace.setLevel("Warning");
    });

    xwalk::hal::test::expectFailure([&]()
    {
        trace.setLevel(static_cast<XWalkTraceLevel>(255U));
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every host-side trace test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int32 main()
{
    testDefaultFiltering();
    testLevelSelection();
    testConstructorInputs();
    testValidation();
    return 0;
}
