/******************************************************************************
 * @file        xHal_Rpi5CarConfigTest.cpp
 * @brief       Verifies section-aware configuration behavior on the host.
 *
 * @details
 * Exercises initial creation, parsing, default insertion, section replacement,
 * comment preservation, explicit persistence, reload, and input validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig
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

#include "xHal_Rpi5CarConfig.h"
#include "xHal_Rpi5CarConfigSimulationArguments.h"
#include "xHal_Rpi5CarConfigSimulationConfig.h"
#include "xHal_Rpi5CarConfigTestSupport.h"

#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains declarations and definitions private to this translation
 * unit.
 */
namespace {

using namespace xwalk::hal;
using xwalk::hal::test::config::expect;
using xwalk::hal::test::config::writeFixture;

/**
 * @brief Exercises parsing, in-memory mutation, persistence, and reload.
 *
 * @param[in] path
 * Writable test configuration path.
 *
 * @return
 * `true` when every configuration expectation succeeds; otherwise `false`.
 */
boolean testConfiguration(const filesystempath &path) {
  writeFixture(path);
  XWalkConfig configuration(path.string());

  boolean result = true;
  result = expect(configuration.get({}, "root") == "first",
                  "Default section parse failed") &&
           result;
  result = expect(configuration.get("motor", "speed") == "40",
                  "Named section parse failed") &&
           result;
  result =
      expect(configuration.get("motor", "missing", "fallback") == "fallback",
             "Default insertion failed") &&
      result;

  configuration.set("motor", "speed", "75");
  configuration.setSection("sensor", {{"limit", "12"}, {"mode", "safe"}});
  configuration.write();

  configuration.set("motor", "speed", "99");
  const configsections reloaded = configuration.read();
  result = expect(configuration.get("motor", "speed") == "75",
                  "Explicit reload failed") &&
           result;
  result = expect(reloaded.at("sensor").at("limit") == "12",
                  "New section write failed") &&
           result;

  const string contents = readFileContents(path);
  result = expect(contents.find("# retained comment") != string::npos,
                  "Comment preservation failed") &&
           result;
  result = expect(contents.find("missing = fallback") != string::npos,
                  "Inserted default was not persisted") &&
           result;
  result = expect(contents.find("direction = forward") != string::npos,
                  "Unrelated option preservation failed") &&
           result;
  return result;
}

/**
 * @brief Verifies missing-file creation and validation failures.
 *
 * @param[in] path
 * Nonexistent configuration path used by the test.
 *
 * @return
 * `true` when creation and validation behave as documented; otherwise `false`.
 */
boolean testCreationAndValidation(const filesystempath &path) {
  errorcode removeError;
  static_cast<void>(removeFilesystemEntry(path, removeError));
  XWalkConfig configuration(path.string(),
                            "Robot configuration\nGenerated for testing");

  boolean result =
      expect(filesystemEntryExists(path), "Missing file was not created");
  const string contents = readFileContents(path);
  result = expect(contents.find("# Robot configuration") != string::npos,
                  "Description comment was not created") &&
           result;

  xwalk::hal::test::expectFailure(
      [&]() { configuration.set({}, "bad=name", "value"); });

  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(configuration.get("absent", "name")); });
  return result;
}

/** @brief Verifies default, help, valid, and malformed simulation arguments. */
boolean testSimulationArguments() {
  char binaryName[] = "xWalkConfigSimulation";
  charpointer defaults[]{binaryName};
  const sim::XWalkConfigSimulationArguments defaultArguments(1, defaults);

  char helpOption[] = "--help";
  charpointer helpValues[]{binaryName, helpOption};
  const sim::XWalkConfigSimulationArguments helpArguments(2, helpValues);

  char traceOption[] = "--trace";
  char enableSelector[] = "RPI.109.enable";
  charpointer enableValues[]{binaryName, traceOption, enableSelector};
  const sim::XWalkConfigSimulationArguments enableArguments(3, enableValues);
  const boolean updateEnabled =
      enableArguments.valid() && enableArguments.applyTraceUpdate();

  char disableSelector[] = "RPI.109.disable";
  charpointer disableValues[]{binaryName, traceOption, disableSelector};
  const sim::XWalkConfigSimulationArguments disableArguments(3, disableValues);
  const boolean updateDisabled =
      disableArguments.valid() && disableArguments.applyTraceUpdate();

  char malformedSelector[] = "RPI.Config.enable";
  charpointer malformedValues[]{binaryName, traceOption, malformedSelector};
  const sim::XWalkConfigSimulationArguments malformedArguments(3,
                                                               malformedValues);
  return defaultArguments.valid() && helpArguments.valid() &&
         helpArguments.helpRequested() && updateEnabled && updateDisabled &&
         (malformedArguments.valid() == false);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side xWalk configuration tests.
 *
 * @param[in] argumentCount
 * Number of command-line arguments; exactly two are required.
 *
 * @param[in] argumentValues
 * Non-owning argument array whose second entry is a writable test path.
 *
 * @return
 * `EXIT_SUCCESS` when every expectation succeeds; otherwise `EXIT_FAILURE`.
 */
int main(int argumentCount, charpointer argumentValues[]) {
  XWalkTrace::configureGlobal(XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH,
                              XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .113, "xWalkConfig section-aware tests started");
  if (argumentCount != 2) {
    return EXIT_FAILURE;
  }

  const filesystempath testPath(argumentValues[1]);
  static_cast<void>(createDirectories(testPath.parent_path()));
  const boolean result =
      testConfiguration(testPath) &&
      testCreationAndValidation(testPath.parent_path() / "created.ini") &&
      testSimulationArguments();
  XWALK_HAL_TRACE_UID0(RPI .114, "xWalkConfig section-aware tests completed");
  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
