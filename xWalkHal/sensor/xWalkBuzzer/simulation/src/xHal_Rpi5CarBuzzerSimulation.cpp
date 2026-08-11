/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerSimulation.cpp
 * @brief       Implements the device-free xWalkBuzzer simulation.
 * @project     xWalk Firmware
 * @module      xWalkBuzzer Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBuzzerSimulation.h"
#include "xHal_Rpi5CarBuzzerHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
int32 runBuzzerSimulation() {
  XWalkBuzzerHostStub backend;
  const XWalkGpioCallbacks callbackSet = XWalkBuzzerHostStub::gpioCallbacks();
  XWalkGpio gpio(&backend, callbackSet, "D4");
  XWalkBuzzer activeBuzzer(gpio);
  activeBuzzer.on();
  const boolean activeOnValid = backend.gpioState();
  activeBuzzer.off();
  const boolean activeOffValid = backend.gpioState() == false;
  XWalkI2c i2c(&backend, &XWalkBuzzerHostStub::probe,
               &XWalkBuzzerHostStub::writeRegister, &XWalkBuzzerHostStub::read);
  XWalkPwmTimerState timerState;
  XWalkPwm pwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
  XWalkBuzzer passiveBuzzer(pwm);
  passiveBuzzer.play(440.0, 0.0);
  const boolean passiveValid = passiveBuzzer.isPassive() &&
                               (passiveBuzzer.isOn() == false) &&
                               (backend.i2cWriteCount() > 0U);
  const boolean valid = activeOnValid && activeOffValid && passiveValid;
  XWALK_HAL_TRACE_UID0(RPI .283, "xWalkBuzzer host simulation completed");
  return valid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
