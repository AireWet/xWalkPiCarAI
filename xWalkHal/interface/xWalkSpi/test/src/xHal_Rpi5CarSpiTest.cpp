/******************************************************************************
 * @file        xHal_Rpi5CarSpiTest.cpp
 * @brief       Tests individual SPI operations through the host mirror.
 *
 * @details
 * Executes the real Linux backend against the injected device-interface mirror
 * and validates simulation trace arguments without physical hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Test
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarSpiTest.h"
#include "xHal_Rpi5CarSpiSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"

/** @brief Contains callback state private to the SPI host tests. */
namespace {

/** @brief Supplies a deliberately incomplete SPI response. */
XWalkHal::bytevector
incompleteTransfer(XWalkHal::contextpointer context,
                   const XWalkHal::bytevector &transmitData) {
  static_cast<void>(context);
  static_cast<void>(transmitData);
  return {0x00U};
}

} /* namespace */

/** @brief Constructs the Linux backend and public SPI test object. */
TEST_SUITE_XWALK_SPI::TEST_SUITE_XWALK_SPI()
    : linuxBackend(hostStub, "host-spi-mirror"),
      spi(&linuxBackend,
          XHAL_SPI_TRANSFER_CALLBACK(xwalk::hal::XWalkSpiLinux)) {}

/** @brief Destroys the isolated test fixture. */
TEST_SUITE_XWALK_SPI::~TEST_SUITE_XWALK_SPI() = default;

/** @brief Configures tracing before the SPI operation suite starts. */
void TEST_SUITE_XWALK_SPI::SetUpTestSuite() {
  const XWalkHal::filesystempath traceConfigurationPath(
      XWALK_SPI_SIMULATION_TRACE_CONFIG_PATH);
  const XWalkHal::filesystempath traceLogPath(
      XWALK_SPI_SIMULATION_TRACE_LOG_PATH);
  xwalk::hal::XWalkTrace::configureGlobal(traceConfigurationPath, traceLogPath);
  XWALK_HAL_TRACE_UID0(RPI .054, "xWalkSpi operation tests started");
}

/** @brief Records completion after the SPI operation suite finishes. */
void TEST_SUITE_XWALK_SPI::TearDownTestSuite() {
  XWALK_HAL_TRACE_UID0(RPI .055, "xWalkSpi operation tests completed");
}

TEST_F(TEST_SUITE_XWALK_SPI, Transfer) {
  const XWalkHal::bytevector request{0x9FU, 0x00U, 0x00U, 0x00U};
  const XWalkHal::bytevector response = spi.transfer(request);

  EXPECT_EQ(hostStub.lastTransmitData(), request);
  EXPECT_EQ(response, XWalkHal::bytevector({0x00U, 0xEFU, 0x40U, 0x18U}));
  EXPECT_EQ(hostStub.speedHz(), XHAL_RPI5CAR_SPI_DEFAULT_SPEED_HZ);
  EXPECT_EQ(hostStub.mode(), XHAL_RPI5CAR_SPI_DEFAULT_MODE);
  EXPECT_EQ(hostStub.bitsPerWord(), XHAL_RPI5CAR_SPI_DEFAULT_BITS_PER_WORD);
}

TEST_F(TEST_SUITE_XWALK_SPI, TransferValidation) {
  EXPECT_THROW(static_cast<void>(spi.transfer({})), std::invalid_argument);
  const XWalkHal::bytevector oversized(
      XHAL_RPI5CAR_SPI_MAXIMUM_TRANSFER_BYTES + 1U, 0U);
  EXPECT_THROW(static_cast<void>(spi.transfer(oversized)), std::out_of_range);
}

TEST_F(TEST_SUITE_XWALK_SPI, CallbackValidation) {
  EXPECT_THROW(XWalkHal::XWalkSpi(nullptr, nullptr), std::invalid_argument);
}

TEST_F(TEST_SUITE_XWALK_SPI, ResponseLengthValidation) {
  XWalkHal::XWalkSpi incompleteSpi(nullptr, &incompleteTransfer);
  EXPECT_THROW(static_cast<void>(incompleteSpi.transfer({0x9FU, 0x00U})),
               std::runtime_error);
}

TEST_F(TEST_SUITE_XWALK_SPI, SimulationTraceArgumentsDefault) {
  char binaryName[] = "xWalkSpiSimulation";
  XWalkHal::charpointer arguments[]{binaryName};
  const xwalk::hal::sim::XWalkSpiSimulationArguments parsed(1, arguments);
  EXPECT_TRUE(parsed.valid());
  EXPECT_FALSE(parsed.helpRequested());
}

TEST_F(TEST_SUITE_XWALK_SPI, SimulationTraceArgumentsHelp) {
  char binaryName[] = "xWalkSpiSimulation";
  char helpOption[] = "--help";
  XWalkHal::charpointer arguments[]{binaryName, helpOption};
  const xwalk::hal::sim::XWalkSpiSimulationArguments parsed(2, arguments);
  EXPECT_TRUE(parsed.valid());
  EXPECT_TRUE(parsed.helpRequested());
}

TEST_F(TEST_SUITE_XWALK_SPI, SimulationTraceArgumentsUid) {
  char binaryName[] = "xWalkSpiSimulation";
  char option[] = "--trace";
  char selector[] = "RPI.059.enable";
  XWalkHal::charpointer arguments[]{binaryName, option, selector};
  const xwalk::hal::sim::XWalkSpiSimulationArguments parsed(3, arguments);
  EXPECT_TRUE(parsed.valid());
  EXPECT_TRUE(parsed.applyTraceUpdate());
}

TEST_F(TEST_SUITE_XWALK_SPI, SimulationTraceArgumentsAll) {
  char binaryName[] = "xWalkSpiSimulation";
  char option[] = "--trace";
  char selector[] = "all.disable";
  XWalkHal::charpointer arguments[]{binaryName, option, selector};
  const xwalk::hal::sim::XWalkSpiSimulationArguments parsed(3, arguments);
  EXPECT_TRUE(parsed.valid());
  EXPECT_TRUE(parsed.applyTraceUpdate());
}

TEST_F(TEST_SUITE_XWALK_SPI, SimulationTraceArgumentsValidation) {
  char binaryName[] = "xWalkSpiSimulation";
  char option[] = "--trace";
  char malformedSelector[] = "RPI.Camera.enable";
  XWalkHal::charpointer malformedArguments[]{binaryName, option,
                                             malformedSelector};
  const xwalk::hal::sim::XWalkSpiSimulationArguments malformed(
      3, malformedArguments);
  EXPECT_FALSE(malformed.valid());

  char unknownOption[] = "--verbose";
  char validSelector[] = "all.disable";
  XWalkHal::charpointer unknownArguments[]{binaryName, unknownOption,
                                           validSelector};
  const xwalk::hal::sim::XWalkSpiSimulationArguments unknown(3,
                                                             unknownArguments);
  EXPECT_FALSE(unknown.valid());
}
