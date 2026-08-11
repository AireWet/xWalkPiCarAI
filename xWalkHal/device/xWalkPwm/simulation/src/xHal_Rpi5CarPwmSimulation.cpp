/******************************************************************************
 * @file        xHal_Rpi5CarPwmSimulation.cpp
 * @brief       Implements the device-free xWalkPwm simulation.
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarPwmSimulation.h"
#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarPwmHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
int32 runPwmSimulation() {
  XWalkPwmHostStub backend;
  XWalkI2c i2c(&backend, &XWalkPwmHostStub::probe,
               &XWalkPwmHostStub::writeRegister, &XWalkPwmHostStub::read,
               nullptr, &XWalkPwmHostStub::tryWriteRegister);
  XWalkPwmTimerState timerState;
  XWalkPwm pwm(i2c, 0U, {}, timerState);
  pwm.setPeriod(4095.0);
  pwm.setPulseWidthPercent(25.0);
  const boolean safeWriteSucceeded = pwm.trySetPulseWidthPercent(0.0);
  const boolean payloadValid = (backend.lastPayloadSize() == 2U) &&
                               (backend.lastHighByte() == 0U) &&
                               (backend.lastLowByte() == 0U);
  const boolean simulationValid =
      safeWriteSucceeded && payloadValid && (backend.writeCount() >= 5U) &&
      (backend.lastRegister() == XHAL_RPI5CAR_PWM_CHANNEL_REG);
  XWALK_HAL_TRACE_UID0(RPI .169, "xWalkPwm host simulation completed");
  return simulationValid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
