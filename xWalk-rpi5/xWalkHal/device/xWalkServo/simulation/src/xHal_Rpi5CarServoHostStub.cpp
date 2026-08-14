/******************************************************************************
 * @file        xHal_Rpi5CarServoHostStub.cpp
 * @brief       Implements the device-free Servo I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkServo Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarServoHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
boolean XWalkServoHostStub::probe(contextpointer context, uint8 address) {
  static_cast<void>(context);
  return address == 0x14U;
}

void XWalkServoHostStub::writeRegister(contextpointer context, uint8 address,
                                       uint8 reg, const bytevector &data) {
  static_cast<void>(address);
  XWalkServoHostStub &self = *static_cast<XWalkServoHostStub *>(context);
  ++self.writeCountValue;
  self.lastRegisterValue = reg;
  self.lastPayloadSizeValue = data.size();
  XWALK_HAL_TRACE_UID1(RPI .186, "Servo host stub recorded register 0x%02X",
                       reg);
}

bytevector XWalkServoHostStub::read(contextpointer context, uint8 address,
                                    size length) {
  static_cast<void>(context);
  static_cast<void>(address);
  return bytevector(length, 0U);
}

size XWalkServoHostStub::writeCount() const noexcept { return writeCountValue; }
uint8 XWalkServoHostStub::lastRegister() const noexcept {
  return lastRegisterValue;
}
size XWalkServoHostStub::lastPayloadSize() const noexcept {
  return lastPayloadSizeValue;
}
} /* namespace xwalk::hal::sim */
