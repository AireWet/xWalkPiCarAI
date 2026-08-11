/******************************************************************************
 * @file        xHal_Rpi5CarAdcHostStub.cpp
 * @brief       Implements the device-free ADC I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkAdc Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarAdcHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
boolean XWalkAdcHostStub::probe(contextpointer context, uint8 address) {
  XWalkAdcHostStub &self = *static_cast<XWalkAdcHostStub *>(context);
  ++self.probeCountValue;
  return address == 0x15U;
}

void XWalkAdcHostStub::writeRegister(contextpointer context, uint8 address,
                                     uint8 reg, const bytevector &data) {
  static_cast<void>(address);
  static_cast<void>(data);
  XWalkAdcHostStub &self = *static_cast<XWalkAdcHostStub *>(context);
  ++self.writeCountValue;
  self.commandValue = reg;
  XWALK_HAL_TRACE_UID1(RPI .177, "ADC host stub recorded command 0x%02X", reg);
}

bytevector XWalkAdcHostStub::read(contextpointer context, uint8 address,
                                  size length) {
  static_cast<void>(context);
  static_cast<void>(address);
  return length == 2U ? bytevector{0x08U, 0x00U} : bytevector{};
}

size XWalkAdcHostStub::probeCount() const noexcept { return probeCountValue; }
size XWalkAdcHostStub::writeCount() const noexcept { return writeCountValue; }
uint8 XWalkAdcHostStub::command() const noexcept { return commandValue; }
} /* namespace xwalk::hal::sim */
