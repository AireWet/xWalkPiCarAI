/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345HostStub.cpp
 * @brief       Implements the device-free ADXL345 I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarAdxl345HostStub.h"
#include "xHal_Rpi5CarAdxl345.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
boolean XWalkAdxl345HostStub::probe(contextpointer context, uint8 address) {
  static_cast<void>(context);
  return address == XHAL_RPI5CAR_ADXL345_ADDRESS;
}

void XWalkAdxl345HostStub::writeRegister(contextpointer context, uint8 address,
                                         uint8 reg, const bytevector &data) {
  static_cast<void>(address);
  static_cast<void>(data);
  XWalkAdxl345HostStub &self = *static_cast<XWalkAdxl345HostStub *>(context);
  ++self.writeCountValue;
  XWALK_HAL_TRACE_UID1(RPI .196, "ADXL345 host stub recorded register 0x%02X",
                       reg);
}

bytevector XWalkAdxl345HostStub::read(contextpointer context, uint8 address,
                                      size length) {
  static_cast<void>(context);
  static_cast<void>(address);
  return bytevector(length, 0U);
}

bytevector XWalkAdxl345HostStub::readRegister(contextpointer context,
                                              uint8 address, uint8 reg,
                                              size length) {
  static_cast<void>(address);
  XWalkAdxl345HostStub &self = *static_cast<XWalkAdxl345HostStub *>(context);
  ++self.registerReadCountValue;
  if (length != XHAL_RPI5CAR_ADXL345_SAMPLE_LENGTH) {
    return {};
  }
  const boolean discardedRead = (self.registerReadCountValue % 2U) != 0U;
  if (discardedRead) {
    return {0U, 0U};
  }
  if (reg == XHAL_RPI5CAR_ADXL345_DATA_X_REGISTER) {
    return {0U, 1U};
  }
  if (reg == XHAL_RPI5CAR_ADXL345_DATA_Y_REGISTER) {
    return {0U, 0U};
  }
  return {0U, 0xFFU};
}

size XWalkAdxl345HostStub::writeCount() const noexcept {
  return writeCountValue;
}
size XWalkAdxl345HostStub::registerReadCount() const noexcept {
  return registerReadCountValue;
}
} /* namespace xwalk::hal::sim */
