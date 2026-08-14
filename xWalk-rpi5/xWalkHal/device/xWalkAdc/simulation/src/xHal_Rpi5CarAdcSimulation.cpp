/******************************************************************************
 * @file        xHal_Rpi5CarAdcSimulation.cpp
 * @brief       Implements the device-free xWalkAdc simulation.
 * @project     xWalk Firmware
 * @module      xWalkAdc Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarAdcSimulation.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarAdcHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
int32 runAdcSimulation() {
  XWalkAdcHostStub backend;
  XWalkI2c i2c(&backend, &XWalkAdcHostStub::probe,
               &XWalkAdcHostStub::writeRegister, &XWalkAdcHostStub::read);
  XWalkAdc adc(i2c, "A3");
  const uint16 sample = adc.read();
  const float64 voltage = adc.readVoltage();
  const boolean valid =
      (adc.address() == 0x15U) && (backend.probeCount() == 2U) &&
      (backend.writeCount() == 2U) && (backend.command() == adc.command()) &&
      (sample == 0x0800U) && (voltage > 1.64) && (voltage < 1.66);
  XWALK_HAL_TRACE_UID0(RPI .178, "xWalkAdc host simulation completed");
  return valid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
