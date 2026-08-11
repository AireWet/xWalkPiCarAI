/******************************************************************************
 * @file        xHal_Rpi5CarConfigStoreTest.cpp
 * @brief       Verifies file-backed xWalk configuration persistence.
 *
 * @details
 * Checks creation, defaults, updates, Robot HAT whitespace compatibility,
 * duplicate handling, input validation, and recovery after external deletion.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig Host Test
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

#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarConfigSimulationConfig.h"

#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"

#include <filesystem>
#include <fstream>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test scenarios private to this translation unit.
 */
namespace {

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Verifies directory creation, defaults, appends, and updates.
 *
 * @param[in] filePath
 * Test-owned configuration path located below the CMake binary directory.
 */
void testPersistence(const XWalkHal::filesystempath &filePath) {
  xwalk::hal::XWalkConfigStore store(filePath.string());
  assert(xwalk::hal::isRegularFile(filePath));
  assert(store.filePath() == filePath.string());
  assert(store.get("missing", "fallback") == "fallback");

  const XWalkHal::filesystempermissions permissionsBefore =
      xwalk::hal::filesystemStatus(filePath).permissions();
  store.set("motor_direction", "1");
  store.set("motor_speed", "42");
  assert(store.get("motor_direction") == "1");
  assert(store.get("motor_speed") == "42");
  assert(xwalk::hal::filesystemStatus(filePath).permissions() ==
         permissionsBefore);

  store.set("motor_speed", "64");
  assert(store.get("motor_speed") == "64");
}

/**
 * @brief Verifies compatibility parsing and last-matching-entry behavior.
 *
 * @param[in] filePath
 * Existing test configuration path.
 */
void testCompatibility(const XWalkHal::filesystempath &filePath) {
  XWalkHal::outputfilestream file(filePath,
                                  xwalk::hal::FILE_OPEN_WRITE_TRUNCATE);
  file << "# preserved comment\n";
  file << "trimmed = 1 2 3\n";
  file << "malformed line\n";
  file << "duplicate = first\n";
  file << "duplicate = second\n";
  file.close();
  assert(!file.fail());

  xwalk::hal::XWalkConfigStore store(filePath.string());
  assert(store.get("trimmed") == "123");
  assert(store.get("duplicate") == "second");
  store.set("duplicate", "updated");
  assert(store.get("duplicate") == "updated");
}

/**
 * @brief Verifies recursive relative includes, ordering, and root-only
 * persistence.
 *
 * @param[in] filePath
 * Test-owned primary configuration path.
 */
void testIncludes(const XWalkHal::filesystempath &filePath) {
  const XWalkHal::filesystempath includeDirectory =
      filePath.parent_path() / "picar-x.d";
  const XWalkHal::filesystempath providerDirectory = includeDirectory / "ai";
  xwalk::hal::createDirectories(providerDirectory);

  XWalkHal::outputfilestream providerFile(providerDirectory / "ollama.conf",
                                          xwalk::hal::FILE_OPEN_WRITE_TRUNCATE);
  providerFile << "provider = ollama\n";
  providerFile.close();
  assert(!providerFile.fail());

  XWalkHal::outputfilestream voiceFile(includeDirectory / "voice.conf",
                                       xwalk::hal::FILE_OPEN_WRITE_TRUNCATE);
  voiceFile << "include = ai/ollama.conf\n";
  voiceFile << "shared = included\n";
  voiceFile.close();
  assert(!voiceFile.fail());

  XWalkHal::outputfilestream primaryFile(filePath,
                                         xwalk::hal::FILE_OPEN_WRITE_TRUNCATE);
  primaryFile << "include = picar-x.d/voice.conf\n";
  primaryFile << "shared = primary\n";
  primaryFile.close();
  assert(!primaryFile.fail());

  xwalk::hal::XWalkConfigStore store(filePath.string());
  assert(store.get("provider") == "ollama");
  assert(store.get("shared") == "primary");
  store.set("calibration", "verified");
  assert(store.get("calibration") == "verified");
  const XWalkHal::string primaryContents =
      xwalk::hal::readFileContents(filePath);
  assert(primaryContents.find("include = picar-x.d/voice.conf") !=
         XWalkHal::string::npos);
  assert(primaryContents.find("provider = ollama") == XWalkHal::string::npos);

  primaryFile.open(filePath, xwalk::hal::FILE_OPEN_WRITE_TRUNCATE);
  primaryFile << "include = picar-x.conf\n";
  primaryFile.close();
  assert(!primaryFile.fail());
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(store.get("provider")); });

  primaryFile.open(filePath, xwalk::hal::FILE_OPEN_WRITE_TRUNCATE);
  primaryFile << "include = ../outside.conf\n";
  primaryFile.close();
  assert(!primaryFile.fail());
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(store.get("provider")); });

  static_cast<void>(
      xwalk::hal::removeFilesystemEntry(providerDirectory / "ollama.conf"));
  static_cast<void>(
      xwalk::hal::removeFilesystemEntry(includeDirectory / "voice.conf"));
  static_cast<void>(xwalk::hal::removeFilesystemEntry(providerDirectory));
  static_cast<void>(xwalk::hal::removeFilesystemEntry(includeDirectory));
}

/**
 * @brief Verifies rejection of paths, keys, and values that violate the text
 * format.
 *
 * @param[in] filePath
 * Test-owned configuration path used for validation calls.
 */
void testValidation(const XWalkHal::filesystempath &filePath) {
  xwalk::hal::test::expectFailure(
      [&]() { xwalk::hal::XWalkConfigStore store(""); });

  xwalk::hal::XWalkConfigStore store(filePath.string());
  xwalk::hal::test::expectFailure(
      [&]() { store.set("invalid=name", "value"); });

  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(store.get(" name ")); });

  xwalk::hal::test::expectFailure(
      [&]() { store.set("name", "first\nsecond"); });
}

/**
 * @brief Verifies recreation when another component removes the backing file.
 *
 * @param[in] filePath
 * Existing test-owned configuration path that this scenario removes.
 */
void testRecreation(const XWalkHal::filesystempath &filePath) {
  xwalk::hal::XWalkConfigStore store(filePath.string());
  assert(xwalk::hal::removeFilesystemEntry(filePath));
  assert(store.get("missing", "recreated") == "recreated");
  assert(xwalk::hal::isRegularFile(filePath));
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all xWalk configuration-store host-test scenarios.
 *
 * @param[in] argumentCount
 * Must equal two so one test path is available.
 *
 * @param[in] arguments
 * Non-owning process argument array whose second entry is the test
 * configuration path.
 *
 * @return
 * Zero when every assertion passes; one when the required path is absent.
 */
xwalk::hal::int32 main(xwalk::hal::int32 argumentCount,
                       xwalk::hal::charpointer arguments[]) {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .115, "xWalkConfig store tests started");
  if (argumentCount != 2) {
    return 1;
  }

  const XWalkHal::filesystempath filePath(arguments[1]);
  const XWalkHal::filesystempath testDirectory = filePath.parent_path();
  XWalkHal::filesystempath replacementPath = filePath;
  replacementPath += ".tmp";
  static_cast<void>(xwalk::hal::removeFilesystemEntry(filePath));
  static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
  testPersistence(filePath);
  testCompatibility(filePath);
  testIncludes(filePath);
  testValidation(filePath);
  testRecreation(filePath);
  static_cast<void>(xwalk::hal::removeFilesystemEntry(filePath));
  static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
  static_cast<void>(xwalk::hal::removeFilesystemEntry(testDirectory));
  XWALK_HAL_TRACE_UID0(RPI .116, "xWalkConfig store tests completed");
  return 0;
}
