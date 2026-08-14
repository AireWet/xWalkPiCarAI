/******************************************************************************
 * @file        xHal_Rpi5CarLedHostStub.cpp
 * @brief       Implements the device-free LED host stub.
 * @project     xWalk Firmware
 * @module      xWalkLed Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarLedHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
boolean XWalkLedHostStub::probe(contextpointer context, uint8 address) {
  static_cast<void>(context);
  static_cast<void>(address);
  return true;
}
void XWalkLedHostStub::writeRegister(contextpointer context, uint8 address,
                                     uint8 reg, const bytevector &data) {
  static_cast<void>(address);
  static_cast<void>(reg);
  static_cast<void>(data);
  ++static_cast<XWalkLedHostStub *>(context)->i2cWriteCountValue;
  XWALK_HAL_TRACE_UID0(RPI .267,
                       "LED host stub accepted an RGB PWM register write");
}
bytevector XWalkLedHostStub::read(contextpointer context, uint8 address,
                                  size length) {
  static_cast<void>(context);
  static_cast<void>(address);
  return bytevector(length, 0U);
}
void XWalkLedHostStub::configureGpio(contextpointer context, uint8 pin,
                                     XWalkGpioMode mode, XWalkGpioPull pull,
                                     boolean initialValue) {
  static_cast<void>(pin);
  static_cast<void>(mode);
  static_cast<void>(pull);
  static_cast<XWalkLedHostStub *>(context)->gpioValue = initialValue;
}
boolean XWalkLedHostStub::readGpio(contextpointer context, uint8 pin) {
  static_cast<void>(pin);
  return static_cast<XWalkLedHostStub *>(context)->gpioValue;
}
void XWalkLedHostStub::writeGpio(contextpointer context, uint8 pin,
                                 boolean value) {
  static_cast<void>(pin);
  static_cast<XWalkLedHostStub *>(context)->gpioValue = value;
}
void XWalkLedHostStub::interruptGpio(contextpointer context, uint8 pin,
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
void XWalkLedHostStub::cancelGpioInterrupt(contextpointer context, uint8 pin) {
  static_cast<void>(context);
  static_cast<void>(pin);
}
XWalkGpioCallbacks XWalkLedHostStub::gpioCallbacks() {
  return {&configureGpio, &readGpio, &writeGpio, &interruptGpio,
          &cancelGpioInterrupt};
}
uint32 XWalkLedHostStub::i2cWriteCount() const noexcept {
  return i2cWriteCountValue;
}
boolean XWalkLedHostStub::gpioState() const noexcept { return gpioValue; }
} /* namespace xwalk::hal::sim */
