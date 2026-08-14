/******************************************************************************
 * @file        xHal_Rpi5CarGpioTest.cpp
 * @brief       Tests individual GPIO operations through the host mirror.
 *
 * @details
 * Executes the real Linux backend against the injected device-interface mirror,
 * preserves callback-level interrupt coverage, and validates trace arguments.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Test
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

#include "xHal_Rpi5CarGpioTest.h"
#include "xHal_Rpi5CarGpioSimulationConfig.h"
#include "xHal_Rpi5CarGpioTestSupport.h"
#include "xHal_Rpi5CarTrace.h"

TEST_SUITE_XWALK_GPIO::TEST_SUITE_XWALK_GPIO()
    : linuxBackend(hostStub, "host-gpio-mirror", "host-gpio", "xwalk-stub",
                   27U),
      gpio(&linuxBackend, XHAL_GPIO_CALLBACKS(xwalk::hal::XWalkGpioLinux),
           "D4") {}

TEST_SUITE_XWALK_GPIO::~TEST_SUITE_XWALK_GPIO() = default;

void TEST_SUITE_XWALK_GPIO::SetUpTestSuite() {
  const XWalkHal::filesystempath traceConfigurationPath(
      XWALK_GPIO_SIMULATION_TRACE_CONFIG_PATH);
  const XWalkHal::filesystempath traceLogPath(
      XWALK_GPIO_SIMULATION_TRACE_LOG_PATH);
  xwalk::hal::XWalkTrace::configureGlobal(traceConfigurationPath, traceLogPath);
  XWALK_HAL_TRACE_UID0(RPI .083, "xWalkGpio operation tests started");
}

void TEST_SUITE_XWALK_GPIO::TearDownTestSuite() {
  XWALK_HAL_TRACE_UID0(RPI .084, "xWalkGpio operation tests completed");
}

TEST_F(TEST_SUITE_XWALK_GPIO, DigitalIo) {
  EXPECT_EQ(hostStub.pin(), 23U);
  EXPECT_EQ(hostStub.mode(), XWalkHal::XWalkGpioMode::Output);
  EXPECT_EQ(hostStub.configureCount(), 1U);

  hostStub.setInputValue(true);
  EXPECT_TRUE(gpio.read());
  EXPECT_EQ(hostStub.mode(), XWalkHal::XWalkGpioMode::Input);
  EXPECT_EQ(hostStub.readCount(), 1U);

  EXPECT_FALSE(gpio.off());
  EXPECT_EQ(hostStub.mode(), XWalkHal::XWalkGpioMode::Output);
  EXPECT_FALSE(hostStub.value());
  EXPECT_EQ(hostStub.writeCount(), 1U);
}

TEST_F(TEST_SUITE_XWALK_GPIO, Polarity) {
  xwalk::hal::sim::XWalkGpioHostStub polarityStub;
  xwalk::hal::XWalkGpioLinux polarityBackend(polarityStub, "host-gpio-mirror");
  xwalk::hal::XWalkGpio activeLow(
      &polarityBackend, XHAL_GPIO_CALLBACKS(xwalk::hal::XWalkGpioLinux), "USER",
      XWalkHal::XWalkGpioMode::Input, XWalkHal::XWalkGpioPull::Up, false);
  polarityStub.setInputValue(true);
  EXPECT_FALSE(activeLow.read());
  EXPECT_TRUE(activeLow.on());
  EXPECT_FALSE(polarityStub.value());
}

TEST_F(TEST_SUITE_XWALK_GPIO, NamedPinMap) {
  const XWalkHal::fixedarray<xwalk::hal::test::gpio::PinMapping, 26U> mappings{
      {{"D0", 17U},         {"D1", 4U},   {"D2", 27U},     {"D3", 22U},
       {"D4", 23U},         {"D5", 24U},  {"D6", 25U},     {"D7", 4U},
       {"D8", 5U},          {"D9", 6U},   {"D10", 12U},    {"D11", 13U},
       {"D12", 19U},        {"D13", 16U}, {"D14", 26U},    {"D15", 20U},
       {"D16", 21U},        {"SW", 25U},  {"USER", 25U},   {"LED", 26U},
       {"BOARD_TYPE", 12U}, {"RST", 16U}, {"BLEINT", 13U}, {"BLERST", 20U},
       {"MCURST", 5U},      {"CE", 8U}}};
  for (const xwalk::hal::test::gpio::PinMapping &mapping : mappings) {
    xwalk::hal::sim::XWalkGpioHostStub pinStub;
    xwalk::hal::XWalkGpioLinux pinBackend(pinStub, "host-gpio-mirror");
    xwalk::hal::XWalkGpio mappedGpio(
        &pinBackend, XHAL_GPIO_CALLBACKS(xwalk::hal::XWalkGpioLinux),
        mapping.name);
    EXPECT_EQ(mappedGpio.pin(), mapping.pin);
  }
}

TEST_F(TEST_SUITE_XWALK_GPIO, Interrupt) {
  xwalk::hal::test::gpio::InterruptBackend backend;
  xwalk::hal::test::gpio::HandlerData handlerData;
  xwalk::hal::XWalkGpio interruptGpio(
      &backend, xwalk::hal::test::gpio::interruptCallbacks(), "LED");
  interruptGpio.irq(&handlerData, &xwalk::hal::test::gpio::handleInterrupt,
                    XWalkHal::XWalkGpioEdge::Rising, 250U);
  ASSERT_NE(backend.handler, nullptr);
  backend.handler(backend.handlerContext);
  EXPECT_EQ(handlerData.count, 1U);
  interruptGpio.deinit();
  EXPECT_EQ(backend.registrationCount, 1U);
  EXPECT_EQ(backend.cancellationCount, 1U);
}

TEST_F(TEST_SUITE_XWALK_GPIO, Validation) {
  const XWalkHal::XWalkGpioCallbacks callbacks =
      xwalk::hal::test::gpio::interruptCallbacks();
  xwalk::hal::test::gpio::InterruptBackend backend;
  EXPECT_THROW(xwalk::hal::XWalkGpio(&backend, callbacks, 7U),
               std::out_of_range);
  EXPECT_THROW(xwalk::hal::XWalkGpio(&backend, callbacks, "UNKNOWN"),
               std::invalid_argument);

  XWalkHal::XWalkGpioCallbacks invalidCallbacks = callbacks;
  invalidCallbacks.read = nullptr;
  EXPECT_THROW(xwalk::hal::XWalkGpio(&backend, invalidCallbacks, "D0"),
               std::invalid_argument);

  xwalk::hal::XWalkGpio validGpio(&backend, callbacks, "D0");
  EXPECT_THROW(validGpio.irq(nullptr, nullptr, XWalkHal::XWalkGpioEdge::Both),
               std::invalid_argument);
}

TEST_F(TEST_SUITE_XWALK_GPIO, SimulationTraceArgumentsDefault) {
  char binaryName[] = "xWalkGpioSimulation";
  XWalkHal::charpointer arguments[]{binaryName};
  const xwalk::hal::sim::XWalkGpioSimulationArguments parsed(1, arguments);
  EXPECT_TRUE(parsed.valid());
  EXPECT_FALSE(parsed.helpRequested());
}

TEST_F(TEST_SUITE_XWALK_GPIO, SimulationTraceArgumentsHelp) {
  char binaryName[] = "xWalkGpioSimulation";
  char helpOption[] = "--help";
  XWalkHal::charpointer arguments[]{binaryName, helpOption};
  const xwalk::hal::sim::XWalkGpioSimulationArguments parsed(2, arguments);
  EXPECT_TRUE(parsed.valid());
  EXPECT_TRUE(parsed.helpRequested());
}

TEST_F(TEST_SUITE_XWALK_GPIO, SimulationTraceArgumentsUid) {
  char binaryName[] = "xWalkGpioSimulation";
  char option[] = "--trace";
  char selector[] = "RPI.080.enable";
  XWalkHal::charpointer arguments[]{binaryName, option, selector};
  const xwalk::hal::sim::XWalkGpioSimulationArguments parsed(3, arguments);
  EXPECT_TRUE(parsed.valid());
  EXPECT_TRUE(parsed.applyTraceUpdate());
}

TEST_F(TEST_SUITE_XWALK_GPIO, SimulationTraceArgumentsAll) {
  char binaryName[] = "xWalkGpioSimulation";
  char option[] = "--trace";
  char selector[] = "all.disable";
  XWalkHal::charpointer arguments[]{binaryName, option, selector};
  const xwalk::hal::sim::XWalkGpioSimulationArguments parsed(3, arguments);
  EXPECT_TRUE(parsed.valid());
  EXPECT_TRUE(parsed.applyTraceUpdate());
}

TEST_F(TEST_SUITE_XWALK_GPIO, SimulationTraceArgumentsValidation) {
  char binaryName[] = "xWalkGpioSimulation";
  char option[] = "--trace";
  char malformedSelector[] = "RPI.Camera.enable";
  XWalkHal::charpointer malformedArguments[]{binaryName, option,
                                             malformedSelector};
  const xwalk::hal::sim::XWalkGpioSimulationArguments malformed(
      3, malformedArguments);
  EXPECT_FALSE(malformed.valid());

  char unknownOption[] = "--verbose";
  char validSelector[] = "all.disable";
  XWalkHal::charpointer unknownArguments[]{binaryName, unknownOption,
                                           validSelector};
  const xwalk::hal::sim::XWalkGpioSimulationArguments unknown(3,
                                                              unknownArguments);
  EXPECT_FALSE(unknown.valid());
}
