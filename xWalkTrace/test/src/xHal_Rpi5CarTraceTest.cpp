/******************************************************************************
 * @file        xHal_Rpi5CarTraceTest.cpp
 * @brief       Verifies trace validation, filtering, and callback delivery.
 *
 * @details
 * Exercises default filtering, every severity, file and callback output,
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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

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

/** @brief Canonical scanner line used to verify compiler-independent metadata. */
#define XHAL_RPI5CAR_TRACE_TEST_METADATA_LINE 1234U

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

/** @brief Writes one complete immutable catalogue fixture below the test directory. */
void writeRuntimeConfiguration(const filesystempath& configurationPath)
{
    outputfilestream configuration(configurationPath, FILE_OPEN_WRITE_TRUNCATE);
    assert(configuration.is_open());
    configuration << R"xml(<?xml version="1.0" encoding="UTF-8"?>
<xwalkTraceCatalogue version="1.0">
  <module name="CTRL" defaultState="disable">
    <trace id="92001" fullId="CTRL.92001" defaultState="disable"
      sourceFile="xWalkTrace/test/src/xHal_Rpi5CarTraceTest.cpp" sourceLine="1234"/>
    <trace id="92002" fullId="CTRL.92002" defaultState="disable"
      sourceFile="trace-test.cpp" sourceLine="2"/>
    <trace id="92003" fullId="CTRL.92003" defaultState="disable"
      sourceFile="trace-test.cpp" sourceLine="3"/>
    <trace id="92004" fullId="CTRL.92004" defaultState="disable"
      sourceFile="trace-test.cpp" sourceLine="4"/>
  </module>
  <module name="RPI" defaultState="disable">
    <trace id="001" fullId="RPI.001" defaultState="disable"
      sourceFile="trace-test.cpp" sourceLine="5"/>
    <trace id="91001" fullId="RPI.91001" defaultState="disable"
      sourceFile="trace-test.cpp" sourceLine="6"/>
    <trace id="91002" fullId="RPI.91002" defaultState="disable"
      sourceFile="trace-test.cpp" sourceLine="7"/>
    <trace id="91003" fullId="RPI.91003" defaultState="disable"
      sourceFile="trace-test.cpp" sourceLine="8"/>
    <trace id="91004" fullId="RPI.91004" defaultState="disable"
      sourceFile="trace-test.cpp" sourceLine="9"/>
  </module>
</xwalkTraceCatalogue>
)xml";
    configuration.flush();
    assert(configuration.good());
}

/** @brief Returns one incremented value for disabled-argument evaluation tests. */
int32 expensiveDiagnostic(int32& invocationCount)
{
    ++invocationCount;
    return invocationCount;
}

/** @brief Verifies explicit first-use paths avoid creating the fallback log directory. */
void testConfiguredGlobalAvoidsDefaultLog()
{
    const filesystempath originalDirectory = std::filesystem::current_path();
    const filesystempath isolatedDirectory =
        originalDirectory / "trace-global-initialization";
    static_cast<void>(std::filesystem::remove_all(isolatedDirectory));
    static_cast<void>(std::filesystem::create_directories(isolatedDirectory));
    const filesystempath configurationPath =
        isolatedDirectory / "trace-catalogue.xml";
    const filesystempath configuredLogPath =
        isolatedDirectory / "configured" / "trace.log";
    writeRuntimeConfiguration(configurationPath);

    std::filesystem::current_path(isolatedDirectory);
    XWalkTrace::configureGlobal(configurationPath, configuredLogPath);
    std::filesystem::current_path(originalDirectory);

    assert(std::filesystem::exists(configuredLogPath));
    assert(std::filesystem::exists(isolatedDirectory / "log") == false);
}

/** @brief Verifies JSON ordering, strict states, unknown IDs, and precedence. */
void testRuntimeConfigurationArguments()
{
    const filesystempath configurationPath("trace-argument-catalogue.xml");
    writeRuntimeConfiguration(configurationPath);
    XWalkTrace::configureGlobal(configurationPath, "trace-argument.log");

    assert(XWalkTrace::applyGlobalTraceArgument("RPI.91001.enable"));
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91001"));
    assert(XWalkTrace::applyGlobalTraceArgument("RPI.disable"));
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91001") == false);
    assert(XWalkTrace::applyGlobalTraceArgument("all.enable"));
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91001"));
    assert(XWalkTrace::globalTraceIsEnabled(0U, "CTRL.92001"));
    assert(XWalkTrace::applyGlobalTraceArgument("CTRL.92001.disable"));
    assert(XWalkTrace::globalTraceIsEnabled(0U, "CTRL.92001") == false);

    const filesystempath jsonPath("trace-argument.json");
    outputfilestream jsonFile(jsonPath, FILE_OPEN_WRITE_TRUNCATE);
    jsonFile << R"json({
  "trace": {
    "all": {"state": "disable"},
    "RPI": {"state": "enable", "tags": {"91001": "disable"}},
    "CTRL": {"state": "disable", "tags": {"92001": "enable"}}
  }
})json";
    jsonFile.close();
    assert(XWalkTrace::applyGlobalTraceArgument(jsonPath.string()));
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91001") == false);
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91004"));
    assert(XWalkTrace::globalTraceIsEnabled(0U, "CTRL.92001"));
    assert(XWalkTrace::globalTraceIsEnabled(0U, "CTRL.92004") == false);

    outputfilestream invalidJson("trace-invalid-state.json", FILE_OPEN_WRITE_TRUNCATE);
    invalidJson << R"json({"trace":{"all":{"state":true}}})json";
    invalidJson.close();
    assert(XWalkTrace::applyGlobalTraceArgument("trace-invalid-state.json") == false);
    assert(XWalkTrace::globalTraceConfigurationError().find("enable or disable") !=
        string::npos);

    outputfilestream unknownJson("trace-unknown-id.json", FILE_OPEN_WRITE_TRUNCATE);
    unknownJson << R"json({"trace":{"RPI":{"tags":{"99999":"enable"}}}})json";
    unknownJson.close();
    assert(XWalkTrace::applyGlobalTraceArgument("trace-unknown-id.json") == false);
    assert(XWalkTrace::globalTraceConfigurationError().find("RPI.99999") !=
        string::npos);
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

/** @brief Verifies accepted records are emitted to the terminal and log file. */
void testFileAndTerminalOutput()
{
    TraceCapture capture;
    XWalkTrace trace(&capture, &captureOutput);
    std::ostringstream terminalOutput;
    std::streambuf* const previousTerminalOutput =
        std::clog.rdbuf(terminalOutput.rdbuf());
    trace.warning("host file output record");
    std::clog.rdbuf(previousTerminalOutput);

    const filesystempath logPath = filesystempath(XHAL_RPI5CAR_TRACE_LOG_DIRECTORY) /
        XHAL_RPI5CAR_TRACE_LOG_FILENAME;
    const string logContents = readFileContents(logPath);
    const string terminalContents = terminalOutput.str();
    assert(logContents.find("[LEGACY] [WARNING]") != string::npos);
    assert(logContents.find("host file output record") != string::npos);
    assert(terminalContents.find("[LEGACY] [WARNING]") != string::npos);
    assert(terminalContents.find("host file output record") != string::npos);
}

/** @brief Verifies missing and malformed XML use safe disabled defaults. */
void testConfigurationFailures()
{
    const filesystempath missingLog("trace-missing-config.log");
    errorcode operationError;
    static_cast<void>(removeFilesystemEntry(missingLog, operationError));
    XWalkTrace::configureGlobal("trace-config-does-not-exist.xml", missingLog);
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91001") == false);
    XWALK_VERBOSE("Always visible value: %d", 9);
    const string missingContents = readFileContents(missingLog);
    assert(missingContents.find("[TRACE] [WARNING]") != string::npos);
    assert(missingContents.find("all normal traces are disabled") != string::npos);
    assert(missingContents.find("[TRACE] [VERBOSE]") == string::npos);
    assert(missingContents.find("Always visible value: 9") == string::npos);

    const filesystempath malformedConfiguration("trace-malformed-config.xml");
    outputfilestream malformed(malformedConfiguration, FILE_OPEN_WRITE_TRUNCATE);
    malformed << "<xwalkTraceCatalogue>";
    malformed.close();
    const filesystempath malformedLog("trace-malformed-config.log");
    operationError.clear();
    static_cast<void>(removeFilesystemEntry(malformedLog, operationError));
    XWalkTrace::configureGlobal(malformedConfiguration, malformedLog);
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91001") == false);
    const string malformedContents = readFileContents(malformedLog);
    assert(malformedContents.find("[TRACE] [ERROR]") != string::npos);
}

/** @brief Verifies macro filtering, categories, call sites, timing, and concurrency. */
void testMacroRuntime()
{
    const filesystempath configurationPath("trace-runtime-config.xml");
    const filesystempath logPath("trace-runtime.log");
    writeRuntimeConfiguration(configurationPath);
    errorcode operationError;
    static_cast<void>(removeFilesystemEntry(logPath, operationError));
    XWalkTrace::configureGlobal(configurationPath, logPath);

    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91001") == false);
    assert(XWalkTrace::globalTraceIsEnabled(1U, "CTRL.92001") == false);

    const boolean leadingZeroTraceEnabled = XWalkTrace::enableGlobalTrace("RPI.001");
    assert(leadingZeroTraceEnabled);
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.001"));
    const boolean leadingZeroTraceDisabled = XWalkTrace::disableGlobalTrace("RPI.001");
    assert(leadingZeroTraceDisabled);
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.001") == false);
    assert(XWalkTrace::enableGlobalTrace("RPI.Camera") == false);
    assert(XWalkTrace::enableGlobalTrace("RPI.99999") == false);

    const boolean allTracesDisabled = XWalkTrace::disableAllGlobalTraces();
    assert(allTracesDisabled);
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91001") == false);
    assert(XWalkTrace::globalTraceIsEnabled(1U, "CTRL.92001") == false);
    assert(XWalkTrace::globalTraceIsEnabled(3U, "RPI.91004") == false);
    const boolean allTracesEnabled = XWalkTrace::enableAllGlobalTraces();
    assert(allTracesEnabled);
    assert(XWalkTrace::globalTraceIsEnabled(0U, "RPI.91001"));
    assert(XWalkTrace::globalTraceIsEnabled(1U, "CTRL.92001"));
    assert(XWalkTrace::globalTraceIsEnabled(2U, "RPI.91003"));
    assert(XWalkTrace::applyGlobalTraceArgument("all.disable"));
    assert(XWalkTrace::applyGlobalTraceArgument("RPI.enable"));
    assert(XWalkTrace::applyGlobalTraceArgument("RPI.91002.disable"));
    assert(XWalkTrace::applyGlobalTraceArgument("RPI.91003.disable"));
    assert(XWalkTrace::applyGlobalTraceArgument("CTRL.enable"));
    assert(XWalkTrace::applyGlobalTraceArgument("CTRL.92002.disable"));
    assert(XWalkTrace::applyGlobalTraceArgument("CTRL.92003.disable"));

    std::ostringstream terminalOutput;
    std::streambuf* const previousTerminalOutput =
        std::clog.rdbuf(terminalOutput.rdbuf());
    int32 diagnosticInvocations = 0;
    XWALK_HAL_TRACE_UID0(RPI.91001, "Enabled HAL value: %u", 7U);
    XWALK_HAL_TRACE_UID1(
        RPI.91002,
        "Disabled HAL value: %d",
        expensiveDiagnostic(diagnosticInvocations));
    XWALK_HAL_TRACE_UID2(
        RPI.91003,
        "Priority-disabled HAL trace: %d",
        expensiveDiagnostic(diagnosticInvocations));
    XWALK_HAL_TRACE_UID3(RPI.91004, "Enabled lowest-priority HAL trace");

    XWALK_CTRL_TRACE_UID1(
        CTRL.92001,
        "Enabled CTRL trace");
    XWALK_CTRL_TRACE_UID0(CTRL.92002, "UID-disabled CTRL trace");
    XWALK_CTRL_TRACE_UID2(CTRL.92003, "Priority-disabled CTRL trace");
    XWALK_CTRL_TRACE_UID3(CTRL.92004, "Enabled CTRL diagnostic: %d", 4);

    const uint32 halWarningLine = __LINE__ + 1U;
    XWALK_HAL_WARNINGS("HAL warning: %d", 11);
    const uint32 halErrorLine = __LINE__ + 1U;
    XWALK_HAL_ERROR("HAL error: %d", -5);
    const uint32 halAssertLine = __LINE__ + 1U;
    XWALK_HAL_ASSERT(100);
    const uint32 ctrlWarningLine = __LINE__ + 1U;
    XWALK_CTRL_WARNINGS("CTRL warning: %d", 12);
    const uint32 ctrlErrorLine = __LINE__ + 1U;
    XWALK_CTRL_ERROR("CTRL error: %d", -2);
    const uint32 ctrlAssertLine = __LINE__ + 1U;
    XWALK_CTRL_ASSERT(200);

    fixedarray<threadhandle, 4U> writers{};
    for (threadhandle& writer : writers)
    {
        writer = threadhandle([]()
        {
            for (uint32 index = 0U; index < 8U; ++index)
            {
                XWALK_CTRL_WARNINGS("Concurrent warning: %u", index);
            }
        });
    }
    for (threadhandle& writer : writers)
    {
        writer.join();
    }
    std::clog.rdbuf(previousTerminalOutput);

    assert(diagnosticInvocations == 0);
    const string contents = readFileContents(logPath);
    const string terminalContents = terminalOutput.str();
    assert(terminalContents == contents);
    assert(contents.find("[HAL] [P0] [RPI.91001]") != string::npos);
    assert(contents.find("RPI.91002") == string::npos);
    assert(contents.find("RPI.91003") == string::npos);
    assert(contents.find("[HAL] [P3] [RPI.91004]") != string::npos);
    assert(contents.find("[CTRL] [P1] [CTRL.92001]") != string::npos);
    assert(contents.find("CTRL.92002") == string::npos);
    assert(contents.find("CTRL.92003") == string::npos);
    assert(contents.find("[CTRL] [P3] [CTRL.92004]") != string::npos);
    assert(contents.find("[HAL] [WARNING]") != string::npos);
    assert(contents.find("[HAL] [ERROR]") != string::npos);
    assert(contents.find("[HAL] [ASSERT]") != string::npos);
    assert(contents.find("[CTRL] [WARNING]") != string::npos);
    assert(contents.find("[CTRL] [ERROR]") != string::npos);
    assert(contents.find("[CTRL] [ASSERT]") != string::npos);
    assert(contents.find("signal=100") != string::npos);
    assert(contents.find("signal=200") != string::npos);
    assert(terminalContents.find("[HAL] [P0] [RPI.91001]") != string::npos);
    assert(terminalContents.find("RPI.91002") == string::npos);
    assert(terminalContents.find("RPI.91003") == string::npos);
    assert(terminalContents.find("[CTRL] [P1] [CTRL.92001]") != string::npos);
    assert(terminalContents.find("CTRL.92002") == string::npos);
    assert(terminalContents.find("CTRL.92003") == string::npos);

    const string sourceName("xHal_Rpi5CarTraceTest.cpp:");
    assert(contents.find("trace-test.cpp:6") != string::npos);
    assert(contents.find(sourceName +
        std::to_string(XHAL_RPI5CAR_TRACE_TEST_METADATA_LINE)) != string::npos);
    assert(contents.find(sourceName + std::to_string(halWarningLine)) != string::npos);
    assert(contents.find(sourceName + std::to_string(halErrorLine)) != string::npos);
    assert(contents.find(sourceName + std::to_string(halAssertLine)) != string::npos);
    assert(contents.find(sourceName + std::to_string(ctrlWarningLine)) != string::npos);
    assert(contents.find(sourceName + std::to_string(ctrlErrorLine)) != string::npos);
    assert(contents.find(sourceName + std::to_string(ctrlAssertLine)) != string::npos);
    const filesystempath compilerSourcePath(__FILE__);
    const boolean compilerPathAbsolute = compilerSourcePath.is_absolute();
    if (compilerPathAbsolute)
    {
        assert(contents.find(__FILE__) == string::npos);
    }

    const std::regex timestampPattern(
        R"(^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]{3}Z )");
    const std::regex elapsedPattern(R"(\[T\+[0-9]+\.[0-9]{6}s\])");
    size lineStart = 0U;
    const size contentsSize = contents.size();
    float64 previousElapsed = 0.0;
    while (lineStart < contentsSize)
    {
        const size lineEnd = contents.find('\n', lineStart);
        const string line = contents.substr(lineStart, lineEnd - lineStart);
        assert(std::regex_search(line, timestampPattern));
        assert(std::regex_search(line, elapsedPattern));
        const size elapsedStart = line.find("[T+") + 3U;
        const size elapsedEnd = line.find("s]", elapsedStart);
        const float64 elapsed = std::stod(line.substr(elapsedStart, elapsedEnd - elapsedStart));
        assert(elapsed >= previousElapsed);
        previousElapsed = elapsed;
        lineStart = (lineEnd == string::npos) ? contentsSize : lineEnd + 1U;
    }
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
    testConfiguredGlobalAvoidsDefaultLog();
    testDefaultFiltering();
    testFileAndTerminalOutput();
    testLevelSelection();
    testConstructorInputs();
    testValidation();
    testConfigurationFailures();
    testRuntimeConfigurationArguments();
    testMacroRuntime();
    return 0;
}
