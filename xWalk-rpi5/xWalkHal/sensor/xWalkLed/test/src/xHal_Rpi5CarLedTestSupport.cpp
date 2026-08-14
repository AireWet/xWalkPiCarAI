/******************************************************************************
 * @file        xHal_Rpi5CarLedTestSupport.cpp
 * @brief       Implements reusable xWalkLed host-test support.
 * @project     xWalk Firmware
 * @module      xWalkLed Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarLedTestSupport.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::test::led {
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
                   XWalkGpioPull pull, boolean initialValue) {
  GpioBackend &backend = *static_cast<GpioBackend *>(context);
  static_cast<void>(pin);
  static_cast<void>(mode);
  static_cast<void>(pull);
  ++backend.configureCount;
  backend.value = initialValue;
}
boolean readGpio(contextpointer context, uint8 pin) {
  static_cast<void>(pin);
  return static_cast<GpioBackend *>(context)->value;
}
void writeGpio(contextpointer context, uint8 pin, boolean value) {
  GpioBackend &backend = *static_cast<GpioBackend *>(context);
  static_cast<void>(pin);
  const boolean simulatedWriteFailure = backend.failWrites.load();
  if (simulatedWriteFailure) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Simulated LED GPIO failure");
  }
  ++backend.writeCount;
  backend.value = value;
}
void registerGpioInterrupt(contextpointer context, uint8 pin,
                           XWalkGpioEdge edge, uint32 debounceMs,
                           contextpointer handlerContext,
                           gpiointerrupthandler handler) {
  static_cast<void>(context);
  static_cast<void>(pin);
  static_cast<void>(edge);
  static_cast<void>(debounceMs);
  static_cast<void>(handlerContext);
  static_cast<void>(handler);
}
void cancelGpioInterrupt(contextpointer context, uint8 pin) {
  static_cast<void>(context);
  static_cast<void>(pin);
}
XWalkGpioCallbacks gpioCallbacks() {
  return {&configureGpio, &readGpio, &writeGpio, &registerGpioInterrupt,
          &cancelGpioInterrupt};
}
boolean probeI2c(contextpointer context, uint8 address) {
  static_cast<void>(context);
  static_cast<void>(address);
  return true;
}
void writeI2cRegister(contextpointer context, uint8 address, uint8 reg,
                      const bytevector &data) {
  static_cast<void>(address);
  static_cast<void>(reg);
  static_cast<void>(data);
  ++static_cast<I2cBackend *>(context)->writeCount;
}
bytevector readI2c(contextpointer context, uint8 address, size length) {
  static_cast<void>(context);
  static_cast<void>(address);
  return bytevector(length, 0U);
}
RgbFixture::RgbFixture()
    : i2c(&bus, &probeI2c, &writeI2cRegister, &readI2c),
      red(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState),
      green(i2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState),
      blue(i2c, 2U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState) {}
boolean nearlyEqual(float64 left, float64 right) {
  constexpr float64 PERCENT_TOLERANCE{0.000'001};
  return XHAL_ABSOLUTE_VALUE(left - right) <= PERCENT_TOLERANCE;
}
} /* namespace xwalk::hal::test::led */
