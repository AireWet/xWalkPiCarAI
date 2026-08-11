/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345Test.cpp
 * @brief       Verifies the ADXL345 port using an in-memory I2C backend.
 *
 * @details
 * Checks configuration writes, discarded samples, signed conversion, axis
 * ordering, address forwarding, and validation without physical hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Host Test
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarAdxl345.h"
#include "xHal_Rpi5CarAdxl345SimulationArguments.h"
#include "xHal_Rpi5CarAdxl345SimulationConfig.h"
#include "xHal_Rpi5CarAdxl345TestSupport.h"
#include "xHal_Rpi5CarTrace.h"

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test state and callbacks private to this translation unit.
 */
namespace {

using TestBus = xwalk::hal::test::adxl345::TestBus;
using xwalk::hal::test::adxl345::probe;
using xwalk::hal::test::adxl345::read;
using xwalk::hal::test::adxl345::readRegister;
using xwalk::hal::test::adxl345::writeRegister;

/**
 * @brief Verifies single-axis configuration, discarded reads, and signed
 * conversion.
 */
void testSingleAxisRead() {
  TestBus bus;
  bus.responses = {{0x00U, 0x00U}, {0x00U, 0x01U}};
  XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, &readRegister);
  XWalkHal::XWalkAdxl345 accelerometer(i2c);

  const XWalkHal::float64 positiveValue =
      accelerometer.read(XWalkHal::XWalkAdxl345Axis::X);
  assert(positiveValue == 1.0);
  assert(accelerometer.address() == XHAL_RPI5CAR_ADXL345_ADDRESS);
  assert(bus.formatWriteCount == 1U);
  assert(bus.powerWriteCount == 1U);
  assert(bus.registerReadCount == 2U);
  assert(bus.lastRegister == XHAL_RPI5CAR_ADXL345_DATA_X_REGISTER);
  assert(bus.lastLength == XHAL_RPI5CAR_ADXL345_SAMPLE_LENGTH);

  bus.responses = {{0x00U, 0x00U}, {0x00U, 0xFFU}};
  bus.responseIndex = 0U;
  const XWalkHal::float64 negativeValue =
      accelerometer.read(XWalkHal::XWalkAdxl345Axis::Y);
  assert(negativeValue == -1.0);
  assert(bus.lastRegister == XHAL_RPI5CAR_ADXL345_DATA_Y_REGISTER);
}

/**
 * @brief Verifies X-, Y-, and Z-axis result ordering and scaling.
 */
void testAllAxesRead() {
  TestBus bus;
  bus.responses = {{0U, 0U},    {0U, 0xFFU}, {0U, 0U},
                   {0x80U, 0U}, {0U, 0U},    {0U, 0x02U}};
  XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, &readRegister);
  XWalkHal::XWalkAdxl345 accelerometer(i2c);

  const XWalkHal::adxl345values values = accelerometer.read();
  assert(values == XWalkHal::adxl345values({-1.0, 0.5, 2.0}));
  assert(bus.formatWriteCount == 3U);
  assert(bus.powerWriteCount == 3U);
  assert(bus.registerReadCount == 6U);
  assert(bus.lastRegister == XHAL_RPI5CAR_ADXL345_DATA_Z_REGISTER);
}

/**
 * @brief Verifies axis, address, backend-capability, and response validation.
 */
void testValidation() {
  TestBus bus;
  bus.responses = {{0U}, {0U, 0U}};
  XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, &readRegister);
  XWalkHal::XWalkAdxl345 accelerometer(i2c);

  xwalk::hal::test::expectFailure([&]() {
    const XWalkHal::XWalkAdxl345Axis invalidAxis =
        static_cast<XWalkHal::XWalkAdxl345Axis>(
            XHAL_RPI5CAR_ADXL345_AXIS_COUNT);
    static_cast<void>(accelerometer.read(invalidAxis));
  });

  xwalk::hal::test::expectFailure([&]() {
    static_cast<void>(accelerometer.read(XWalkHal::XWalkAdxl345Axis::Z));
  });

  XWalkHal::XWalkI2c sequentialOnlyI2c(&bus, &probe, &writeRegister, &read);
  XWalkHal::XWalkAdxl345 sequentialOnlyAccelerometer(sequentialOnlyI2c);
  xwalk::hal::test::expectFailure([&]() {
    static_cast<void>(
        sequentialOnlyAccelerometer.read(XWalkHal::XWalkAdxl345Axis::X));
  });

  xwalk::hal::test::expectFailure([&]() {
    XWalkHal::XWalkAdxl345 invalidAddressAccelerometer(i2c, 0x80U);
  });
}

/** @brief Verifies persistent ADXL345 trace selector parsing and application.
 */
void testTraceSelection() {
  char executable[] = "xWalkAdxl345Test";
  char option[] = "--trace";
  char enableSelector[] = "RPI.197.enable";
  char disableSelector[] = "RPI.197.disable";
  char malformedSelector[] = "RPI.invalid.enable";
  XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
  xwalk::hal::sim::XWalkAdxl345SimulationArguments enable(3, enableArguments);
  assert(enable.valid());
  assert(enable.applyTraceUpdate());
  XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
  xwalk::hal::sim::XWalkAdxl345SimulationArguments disable(3, disableArguments);
  assert(disable.valid());
  assert(disable.applyTraceUpdate());
  XWalkHal::charpointer malformedArguments[]{executable, option,
                                             malformedSelector};
  const xwalk::hal::sim::XWalkAdxl345SimulationArguments malformed(
      3, malformedArguments);
  assert(malformed.valid() == false);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every xWalk ADXL345 host-test scenario.
 *
 * @return
 * Zero when every assertion passes.
 */
XWalkHal::int32 main() {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_ADXL345_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_ADXL345_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .200, "xWalkAdxl345 host tests started");
  testSingleAxisRead();
  testAllAxesRead();
  testValidation();
  testTraceSelection();
  XWALK_HAL_TRACE_UID0(RPI .201, "xWalkAdxl345 host tests completed");
  return 0;
}
