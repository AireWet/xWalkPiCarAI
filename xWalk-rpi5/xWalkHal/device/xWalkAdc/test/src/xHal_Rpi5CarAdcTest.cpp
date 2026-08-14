/******************************************************************************
 * @file        xHal_Rpi5CarAdcTest.cpp
 * @brief       Verifies the xWalk ADC port with an in-memory I2C backend.
 *
 * @details
 * Checks address selection, channel mapping, bus traffic, sample assembly,
 *voltage scaling, and validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdc Host Test
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

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarAdcSimulationArguments.h"
#include "xHal_Rpi5CarAdcSimulationConfig.h"
#include "xHal_Rpi5CarAdcTestSupport.h"
#include "xHal_Rpi5CarTrace.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <thread>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test state and callbacks private to this translation unit.
 */
namespace {

/**
 * @brief Verifies numeric channel mapping and raw sample acquisition.
 */
void testRead() {
  xwalk::hal::test::adc::TestBus bus;
  xwalk::hal::XWalkI2c i2c(&bus, &xwalk::hal::test::adc::probe,
                           &xwalk::hal::test::adc::writeRegister,
                           &xwalk::hal::test::adc::read);
  xwalk::hal::XWalkAdc adc(i2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);

  const XWalkHal::uint16 sample = adc.read();
  assert(sample == 0x0ABCU);
  assert(adc.channel() == 0U);
  assert(adc.command() == 0x17U);
  assert(bus.writeAddress == XHAL_RPI5CAR_ADC_ADDRESS_1);
  assert(bus.writeRegister == 0x17U);
  assert(bus.writeData == XWalkHal::bytevector({0U, 0U}));
  assert(bus.readAddress == XHAL_RPI5CAR_ADC_ADDRESS_1);
  assert(bus.readLength == 2U);
}

/**
 * @brief Verifies named channels, automatic address selection, and full-scale
 * voltage conversion.
 */
void testSelectionAndVoltage() {
  xwalk::hal::test::adc::TestBus bus;
  bus.presentAddresses.insert(XHAL_RPI5CAR_ADC_ADDRESS_2);
  bus.readBytes = {0x0FU, 0xFFU};
  xwalk::hal::XWalkI2c i2c(&bus, &xwalk::hal::test::adc::probe,
                           &xwalk::hal::test::adc::writeRegister,
                           &xwalk::hal::test::adc::read);
  xwalk::hal::XWalkAdc adc(i2c, "A7");

  assert(adc.address() == XHAL_RPI5CAR_ADC_ADDRESS_2);
  assert(adc.command() == 0x10U);
  assert(bus.probes == XWalkHal::bytevector({0x14U, 0x15U}));
  const XWalkHal::float64 voltageDifference =
      XHAL_ABSOLUTE_VALUE(adc.readVoltage() - 3.3);
  assert(voltageDifference < 0.000001);
}

/**
 * @brief Verifies rejection of invalid channels and incomplete reads.
 */
void testValidation() {
  xwalk::hal::test::adc::TestBus bus;
  xwalk::hal::XWalkI2c i2c(&bus, &xwalk::hal::test::adc::probe,
                           &xwalk::hal::test::adc::writeRegister,
                           &xwalk::hal::test::adc::read);
  xwalk::hal::test::expectFailure([&]() { xwalk::hal::XWalkAdc adc(i2c, 8U); });

  xwalk::hal::test::expectFailure(
      [&]() { xwalk::hal::XWalkAdc adc(i2c, "A8"); });

  bus.readBytes = {0x01U};
  xwalk::hal::XWalkAdc adc(i2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
  xwalk::hal::test::expectFailure([&]() { static_cast<void>(adc.read()); });
}

/**
 * @brief Proves concurrent ADC conversions retain their selected channel
 * responses.
 *
 * @details
 * The first conversion is paused inside its write callback while a second
 * caller becomes runnable. The shared I2C transaction lock preserves write/read
 * pairs without real-time sleeping.
 */
void testConcurrentConversionsAreAtomic() {
  xwalk::hal::test::adc::TestBus bus;
  bus.returnSelectedChannelValue = true;
  bus.pauseFirstWrite = true;
  bus.channelValues[0U] = 0x0111U;
  bus.channelValues[1U] = 0x0222U;
  xwalk::hal::XWalkI2c i2c(&bus, &xwalk::hal::test::adc::probe,
                           &xwalk::hal::test::adc::writeRegister,
                           &xwalk::hal::test::adc::read);
  xwalk::hal::XWalkAdc adcA0(i2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
  xwalk::hal::XWalkAdc adcA1(i2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkHal::uint16 a0Value{};
  XWalkHal::uint16 a1Value{};

  std::thread first([&]() { a0Value = adcA0.read(); });
  {
    XWalkHal::uniquemutexlock lock(bus.mutexValue);
    bus.conditionValue.wait(lock, [&bus]() { return bus.firstWriteObserved; });
  }
  std::thread second([&]() {
    {
      const XWalkHal::mutexlock lock(bus.mutexValue);
      bus.contenderReady = true;
    }
    bus.conditionValue.notify_all();
    a1Value = adcA1.read();
  });
  {
    XWalkHal::uniquemutexlock lock(bus.mutexValue);
    bus.conditionValue.wait(lock, [&bus]() { return bus.contenderReady; });
    bus.releaseFirstWrite = true;
  }
  bus.conditionValue.notify_all();
  first.join();
  second.join();

  assert(a0Value == 0x0111U);
  assert(a1Value == 0x0222U);
  assert(bus.operationOrder == XWalkHal::bytevector({0x17U, 0U, 0x16U, 0U}));
}

/** @brief Verifies persistent ADC trace selector parsing and application. */
void testTraceSelection() {
  char executable[] = "xWalkAdcTest";
  char option[] = "--trace";
  char enableSelector[] = "RPI.178.enable";
  char disableSelector[] = "RPI.178.disable";
  char malformedSelector[] = "RPI.invalid.enable";
  XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
  xwalk::hal::sim::XWalkAdcSimulationArguments enable(3, enableArguments);
  assert(enable.valid());
  assert(enable.applyTraceUpdate());
  XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
  xwalk::hal::sim::XWalkAdcSimulationArguments disable(3, disableArguments);
  assert(disable.valid());
  assert(disable.applyTraceUpdate());
  XWalkHal::charpointer malformedArguments[]{executable, option,
                                             malformedSelector};
  const xwalk::hal::sim::XWalkAdcSimulationArguments malformed(
      3, malformedArguments);
  assert(malformed.valid() == false);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all xWalk ADC host-test scenarios.
 *
 * @return
 * Zero when every assertion passes. A failed assertion terminates the process.
 */
XWalkHal::int32 main() {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_ADC_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_ADC_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .181, "xWalkAdc host tests started");
  testRead();
  testSelectionAndVoltage();
  testValidation();
  testConcurrentConversionsAreAtomic();
  testTraceSelection();
  XWALK_HAL_TRACE_UID0(RPI .182, "xWalkAdc host tests completed");
  return 0;
}
