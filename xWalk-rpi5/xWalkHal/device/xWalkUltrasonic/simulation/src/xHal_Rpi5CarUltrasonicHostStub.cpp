/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicHostStub.cpp
 * @brief       Implements the device-free ultrasonic GPIO host stub.
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarUltrasonicHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
void XWalkUltrasonicHostStub::configure(contextpointer context, uint8 pin,
                                        XWalkGpioMode mode, XWalkGpioPull pull,
                                        boolean initialValue) {
  static_cast<void>(context);
  static_cast<void>(pin);
  static_cast<void>(mode);
  static_cast<void>(pull);
  static_cast<void>(initialValue);
}

boolean XWalkUltrasonicHostStub::read(contextpointer context, uint8 pin) {
  static_cast<void>(pin);
  XWalkUltrasonicHostStub &self =
      *static_cast<XWalkUltrasonicHostStub *>(context);
  ++self.echoReadCountValue;
  if (self.echoReadCountValue == 3U) {
    common::sleepMicroseconds(1'000U);
  }
  return (self.echoReadCountValue == 2U) || (self.echoReadCountValue == 3U);
}

void XWalkUltrasonicHostStub::write(contextpointer context, uint8 pin,
                                    boolean value) {
  XWalkUltrasonicHostStub &self =
      *static_cast<XWalkUltrasonicHostStub *>(context);
  if (pin == 27U) {
    ++self.triggerWriteCountValue;
    if (value) {
      ++self.triggerCountValue;
      self.echoReadCountValue = 0U;
      XWALK_HAL_TRACE_UID0(RPI .206,
                           "Ultrasonic host stub received a trigger pulse");
    }
  }
}

void XWalkUltrasonicHostStub::interrupt(contextpointer context, uint8 pin,
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

void XWalkUltrasonicHostStub::cancelInterrupt(contextpointer context,
                                              uint8 pin) {
  static_cast<void>(context);
  static_cast<void>(pin);
}

XWalkGpioCallbacks XWalkUltrasonicHostStub::callbacks() {
  return {&configure, &read, &write, &interrupt, &cancelInterrupt};
}

uint32 XWalkUltrasonicHostStub::triggerCount() const noexcept {
  return triggerCountValue;
}
uint32 XWalkUltrasonicHostStub::triggerWriteCount() const noexcept {
  return triggerWriteCountValue;
}
} /* namespace xwalk::hal::sim */
