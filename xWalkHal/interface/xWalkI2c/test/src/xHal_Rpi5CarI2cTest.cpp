/******************************************************************************
 * @file        xHal_Rpi5CarI2cTest.cpp
 * @brief       Tests individual I2C operations through the host mirror.
 *
 * @details
 * Registers one Google Test case for each public I2C operation while executing
 * the real Linux backend against the injected device-interface mirror.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Test
 *
 * @author      Joxy John
 * @date        2026-08-09
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

#include "xHal_Rpi5CarI2cTest.h"
#include "xHal_Rpi5CarI2cSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Test fixture definitions
 ******************************************************************************/

TEST_SUITE_XWALK_I2C::TEST_SUITE_XWALK_I2C()
    : linuxBackend(hostStub, "host-i2c-mirror", 1U),
      i2c(&linuxBackend, XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
          XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
          XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux),
          XHAL_I2C_READ_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
          XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux)) {}

TEST_SUITE_XWALK_I2C::~TEST_SUITE_XWALK_I2C() = default;

void TEST_SUITE_XWALK_I2C::SetUpTestSuite() {
  const XWalkHal::filesystempath traceConfigurationPath(
      XWALK_I2C_SIMULATION_TRACE_CONFIG_PATH);
  const XWalkHal::filesystempath traceLogPath(
      XWALK_I2C_SIMULATION_TRACE_LOG_PATH);
  xwalk::hal::XWalkTrace::configureGlobal(traceConfigurationPath, traceLogPath);
  XWALK_HAL_TRACE_UID0(RPI .029, "xWalkI2c operation tests started");
}

void TEST_SUITE_XWALK_I2C::TearDownTestSuite() {
  XWALK_HAL_TRACE_UID0(RPI .030, "xWalkI2c operation tests completed");
}

/******************************************************************************
 * Individual operation tests
 ******************************************************************************/

TEST_F(TEST_SUITE_XWALK_I2C, Probe) {
  EXPECT_TRUE(i2c.probe(0x14U));
  EXPECT_EQ(hostStub.lastAddress(), 0x14U);
}

TEST_F(TEST_SUITE_XWALK_I2C, ProbeValidation) {
  EXPECT_THROW(static_cast<void>(i2c.probe(0x80U)), std::out_of_range);
}

TEST_F(TEST_SUITE_XWALK_I2C, WriteRegister) {
  const XWalkHal::bytevector bytes{0x12U, 0x34U};
  i2c.writeRegister(0x14U, 0x20U, bytes);

  EXPECT_EQ(hostStub.lastAddress(), 0x14U);
  EXPECT_EQ(hostStub.lastRegister(), 0x20U);
  EXPECT_EQ(hostStub.lastData(), bytes);
}

TEST_F(TEST_SUITE_XWALK_I2C, TryWriteRegister) {
  const XWalkHal::bytevector bytes{0x12U, 0x34U};
  const XWalkHal::bytevector emptyBytes;

  EXPECT_TRUE(i2c.tryWriteRegister(0x14U, 0x20U, bytes));
  EXPECT_FALSE(i2c.tryWriteRegister(0x14U, 0x20U, emptyBytes));
}

TEST_F(TEST_SUITE_XWALK_I2C, Read) {
  const XWalkHal::bytevector bytes = i2c.read(0x15U, 2U);

  EXPECT_EQ(hostStub.lastAddress(), 0x15U);
  EXPECT_EQ(hostStub.lastReadLength(), 2U);
  EXPECT_EQ(bytes, XWalkHal::bytevector({0xABU, 0xCDU}));
}

TEST_F(TEST_SUITE_XWALK_I2C, ReadRegister) {
  const XWalkHal::bytevector bytes = i2c.readRegister(0x53U, 0x32U, 2U);

  EXPECT_EQ(hostStub.lastAddress(), 0x53U);
  EXPECT_EQ(hostStub.lastRegister(), 0x32U);
  EXPECT_EQ(hostStub.lastReadLength(), 2U);
  EXPECT_EQ(bytes, XWalkHal::bytevector({0x34U, 0x12U}));
}

TEST_F(TEST_SUITE_XWALK_I2C, WriteRegisterThenRead) {
  const XWalkHal::bytevector command{0x00U, 0x00U};
  const XWalkHal::bytevector bytes =
      i2c.writeRegisterThenRead(0x14U, 0x17U, command, 2U);

  EXPECT_EQ(hostStub.lastAddress(), 0x14U);
  EXPECT_EQ(hostStub.lastRegister(), 0x17U);
  EXPECT_EQ(hostStub.lastData(), command);
  EXPECT_EQ(hostStub.lastReadLength(), 2U);
  EXPECT_EQ(bytes, XWalkHal::bytevector({0xABU, 0xCDU}));
}

TEST_F(TEST_SUITE_XWALK_I2C, ReadRegisterCallbackValidation) {
  xwalk::hal::XWalkI2c sequentialOnlyI2c{
      &linuxBackend, XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
      XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
      XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux)};

  EXPECT_THROW(
      static_cast<void>(sequentialOnlyI2c.readRegister(0x53U, 0x32U, 2U)),
      std::runtime_error);
}

TEST_F(TEST_SUITE_XWALK_I2C, SimulationTraceArgumentsDefault) {
  char binaryName[] = "xWalkI2cSimulation";
  XWalkHal::charpointer arguments[]{binaryName};
  const xwalk::hal::sim::XWalkI2cSimulationArguments parsed(1, arguments);
  EXPECT_TRUE(parsed.valid());
  EXPECT_FALSE(parsed.helpRequested());
}

TEST_F(TEST_SUITE_XWALK_I2C, SimulationTraceArgumentsHelp) {
  char binaryName[] = "xWalkI2cSimulation";
  char helpOption[] = "--help";
  XWalkHal::charpointer helpArguments[]{binaryName, helpOption};
  const xwalk::hal::sim::XWalkI2cSimulationArguments helpParsed(2,
                                                                helpArguments);
  EXPECT_TRUE(helpParsed.valid());
  EXPECT_TRUE(helpParsed.helpRequested());

  char shortHelpOption[] = "-h";
  XWalkHal::charpointer shortHelpArguments[]{binaryName, shortHelpOption};
  const xwalk::hal::sim::XWalkI2cSimulationArguments shortHelpParsed(
      2, shortHelpArguments);
  EXPECT_TRUE(shortHelpParsed.valid());
  EXPECT_TRUE(shortHelpParsed.helpRequested());
}

TEST_F(TEST_SUITE_XWALK_I2C, SimulationTraceArgumentsUid) {
  char binaryName[] = "xWalkI2cSimulation";
  char option[] = "--trace";
  char disableSelector[] = "RPI.031.disable";
  XWalkHal::charpointer disableArguments[]{binaryName, option, disableSelector};
  const xwalk::hal::sim::XWalkI2cSimulationArguments disableParsed(
      3, disableArguments);
  EXPECT_TRUE(disableParsed.valid());
  EXPECT_TRUE(disableParsed.applyTraceUpdate());

  char enableSelector[] = "RPI.031.enable";
  XWalkHal::charpointer enableArguments[]{binaryName, option, enableSelector};
  const xwalk::hal::sim::XWalkI2cSimulationArguments enableParsed(
      3, enableArguments);
  EXPECT_TRUE(enableParsed.valid());
  EXPECT_TRUE(enableParsed.applyTraceUpdate());
}

TEST_F(TEST_SUITE_XWALK_I2C, SimulationTraceArgumentsAll) {
  char binaryName[] = "xWalkI2cSimulation";
  char option[] = "--trace";
  char disableSelector[] = "all.disable";
  XWalkHal::charpointer disableArguments[]{binaryName, option, disableSelector};
  const xwalk::hal::sim::XWalkI2cSimulationArguments disableParsed(
      3, disableArguments);
  EXPECT_TRUE(disableParsed.valid());
  EXPECT_TRUE(disableParsed.applyTraceUpdate());

  char enableSelector[] = "all.enable";
  XWalkHal::charpointer enableArguments[]{binaryName, option, enableSelector};
  const xwalk::hal::sim::XWalkI2cSimulationArguments enableParsed(
      3, enableArguments);
  EXPECT_TRUE(enableParsed.valid());
  EXPECT_TRUE(enableParsed.applyTraceUpdate());
}

TEST_F(TEST_SUITE_XWALK_I2C, SimulationTraceArgumentsValidation) {
  char binaryName[] = "xWalkI2cSimulation";
  char option[] = "--trace";
  char malformedSelector[] = "RPI.Camera.enable";
  XWalkHal::charpointer malformedArguments[]{binaryName, option,
                                             malformedSelector};
  const xwalk::hal::sim::XWalkI2cSimulationArguments malformed(
      3, malformedArguments);
  EXPECT_FALSE(malformed.valid());

  char unknownOption[] = "--verbose";
  char validSelector[] = "all.disable";
  XWalkHal::charpointer unknownArguments[]{binaryName, unknownOption,
                                           validSelector};
  const xwalk::hal::sim::XWalkI2cSimulationArguments unknown(3,
                                                             unknownArguments);
  EXPECT_FALSE(unknown.valid());
}
