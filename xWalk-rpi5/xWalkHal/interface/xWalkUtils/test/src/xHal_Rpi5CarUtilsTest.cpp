/******************************************************************************
 * @file        xHal_Rpi5CarUtilsTest.cpp
 * @brief       Verifies backend-neutral Robot HAT utility behavior.
 *
 * @details
 * Exercises output routing, volume clamping, command and lookup forwarding,
 * mapping validation, lazy caching, stderr restoration, and callback checks.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Test
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

#include "xHal_Rpi5CarLazyReader.h"
#include "xHal_Rpi5CarUtilsSimulationArguments.h"
#include "xHal_Rpi5CarUtilsSimulationConfig.h"
#include "xHal_Rpi5CarUtilsTestSupport.h"

#include "xHal_Rpi5CarStderrGuard.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarUtils.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains host-test state and callbacks private to this translation
 * unit. */
namespace {

using namespace xwalk::hal;
using namespace xwalk::hal::test::utils;

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Verifies colored-output routing and volume clamping. */
void testOutputAndVolume() {
  TestUtilsBackend backend;
  XWalkUtils utilities(&backend, utilityCallbacks());

  utilities.info("information");
  assert(backend.color == XWalkUtilityColor::White);
  utilities.debug("diagnostic", "", true);
  assert(backend.color == XWalkUtilityColor::Gray);
  assert(backend.message == "diagnostic");
  assert(backend.ending.empty());
  assert(backend.flush);
  utilities.warning("warning");
  assert(backend.color == XWalkUtilityColor::Yellow);
  utilities.error("error");
  assert(backend.color == XWalkUtilityColor::Red);
  assert(backend.outputCount == 4U);

  utilities.setVolume(-10);
  assert(backend.volumePercent == 0U);
  utilities.setVolume(45);
  assert(backend.volumePercent == 45U);
  utilities.setVolume(110);
  assert(backend.volumePercent == 100U);
}

/** @brief Verifies command, executable, network, and username forwarding. */
void testPlatformQueries() {
  TestUtilsBackend backend;
  XWalkUtils utilities(&backend, utilityCallbacks());

  const XWalkCommandResult result =
      utilities.runCommand("test", "user", "group");
  assert(result.status == 7);
  assert(result.output == "combined output");
  assert(backend.command == "test");
  assert(backend.user == "user");
  assert(backend.group == "group");
  assert(utilities.commandExists("available"));
  assert(!utilities.isInstalled("missing"));
  assert(utilities.checkExecutable("available"));
  assert(backend.executable == "available");

  assert(utilities.ipAddress() == "192.0.2.10");
  assert(backend.interfaceName == "eth0");
  assert(utilities.ipAddress("wlan0").empty());
  const stringvector interfaces{"usb0", "eth0", "wlan0"};
  assert(utilities.ipAddress(interfaces) == "192.0.2.10");
  assert(utilities.username() == "robot");
}

/** @brief Verifies numeric mapping and invalid-range rejection. */
void testMapping() {
  assert(XWalkUtils::mapping(5.0, 0.0, 10.0, 0.0, 100.0) == 50.0);
  assert(XWalkUtils::mapping(15.0, 0.0, 10.0, 0.0, 100.0) == 150.0);

  xwalk::hal::test::expectFailure([&]() {
    static_cast<void>(XWalkUtils::mapping(1.0, 2.0, 2.0, 3.0, 4.0));
  });
}

/** @brief Verifies strict-interval lazy caching and refresh behavior. */
void testLazyReader() {
  TestLazyBackend backend;
  XWalkLazyReader<uint32> reader(&backend, &lazyRead, &lazyClock, 10U);

  assert(reader.read() == 10U);
  assert(backend.readCount == 1U);
  backend.currentTimeUs = 10'000U;
  assert(reader.read() == 10U);
  assert(backend.readCount == 1U);
  backend.currentTimeUs = 10'001U;
  assert(reader.read() == 11U);
  assert(backend.readCount == 2U);
}

/** @brief Verifies scope-bound redirect restoration and retained token
 * delivery. */
void testStderrGuard() {
  TestRedirectBackend backend;
  {
    XWalkStderrGuard guard(&backend, &redirectError, &restoreError);
    assert(backend.redirectCount == 1U);
    assert(backend.restoreCount == 0U);
    static_cast<void>(guard);
  }
  assert(backend.restoreCount == 1U);
  assert(backend.token == 42);
}

/** @brief Verifies rejection of incomplete callback bindings and invalid
 * colors. */
void testValidation() {
  TestUtilsBackend backend;
  const fixedarray<XWalkUtilsCallbacks, 6U> incompleteCallbacks{
      XWalkUtilsCallbacks{nullptr, &setVolume, &runCommand, &executableExists,
                          &ipAddress, &username},
      XWalkUtilsCallbacks{&writeOutput, nullptr, &runCommand, &executableExists,
                          &ipAddress, &username},
      XWalkUtilsCallbacks{&writeOutput, &setVolume, nullptr, &executableExists,
                          &ipAddress, &username},
      XWalkUtilsCallbacks{&writeOutput, &setVolume, &runCommand, nullptr,
                          &ipAddress, &username},
      XWalkUtilsCallbacks{&writeOutput, &setVolume, &runCommand,
                          &executableExists, nullptr, &username},
      XWalkUtilsCallbacks{&writeOutput, &setVolume, &runCommand,
                          &executableExists, &ipAddress, nullptr}};
  for (const XWalkUtilsCallbacks &callbacks : incompleteCallbacks) {
    xwalk::hal::test::expectFailure([&]() {
      XWalkUtils utilities(&backend, callbacks);
      static_cast<void>(utilities);
    });
  }

  XWalkUtils utilities(&backend, utilityCallbacks());
  xwalk::hal::test::expectFailure([&]() {
    utilities.printColor("invalid", static_cast<XWalkUtilityColor>(255U));
  });

  TestRedirectBackend redirectBackend;
  xwalk::hal::test::expectFailure([&]() {
    XWalkStderrGuard guard(&redirectBackend, nullptr, &restoreError);
    static_cast<void>(guard);
  });

  TestLazyBackend lazyBackend;
  xwalk::hal::test::expectFailure([&]() {
    XWalkLazyReader<uint32> reader(&lazyBackend, nullptr, &lazyClock);
    static_cast<void>(reader);
  });
}

/** @brief Verifies default, help, valid, and malformed simulation selectors. */
void testSimulationArguments() {
  char binaryName[] = "xWalkUtilsSimulation";
  charpointer defaults[]{binaryName};
  const sim::XWalkUtilsSimulationArguments defaultArguments(1, defaults);
  assert(defaultArguments.valid());

  char helpOption[] = "--help";
  charpointer helpValues[]{binaryName, helpOption};
  const sim::XWalkUtilsSimulationArguments helpArguments(2, helpValues);
  assert(helpArguments.valid());
  assert(helpArguments.helpRequested());

  char traceOption[] = "--trace";
  char enableSelector[] = "RPI.136.enable";
  charpointer enableValues[]{binaryName, traceOption, enableSelector};
  const sim::XWalkUtilsSimulationArguments enableArguments(3, enableValues);
  assert(enableArguments.valid());
  assert(enableArguments.applyTraceUpdate());

  char disableSelector[] = "RPI.136.disable";
  charpointer disableValues[]{binaryName, traceOption, disableSelector};
  const sim::XWalkUtilsSimulationArguments disableArguments(3, disableValues);
  assert(disableArguments.valid());
  assert(disableArguments.applyTraceUpdate());

  char malformedSelector[] = "RPI.Utils.enable";
  charpointer malformedValues[]{binaryName, traceOption, malformedSelector};
  const sim::XWalkUtilsSimulationArguments malformedArguments(3,
                                                              malformedValues);
  assert(malformedArguments.valid() == false);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every host-side utility test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int main() {
  XWalkTrace::configureGlobal(XWALK_UTILS_SIMULATION_TRACE_CONFIG_PATH,
                              XWALK_UTILS_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .139, "xWalkUtils host tests started");
  testOutputAndVolume();
  testPlatformQueries();
  testMapping();
  testLazyReader();
  testStderrGuard();
  testValidation();
  testSimulationArguments();
  XWALK_HAL_TRACE_UID0(RPI .140, "xWalkUtils host tests completed");
  return 0;
}
