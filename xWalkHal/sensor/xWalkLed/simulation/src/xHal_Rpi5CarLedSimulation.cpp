/******************************************************************************
 * @file        xHal_Rpi5CarLedSimulation.cpp
 * @brief       Implements the device-free xWalkLed simulation.
 * @project     xWalk Firmware
 * @module      xWalkLed Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarLedSimulation.h"
#include "xHal_Rpi5CarLedHostStub.h"
#include "xHal_Rpi5CarRgbLed.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
int32 runLedSimulation() {
  XWalkLedHostStub backend;
  const XWalkGpioCallbacks callbackSet = XWalkLedHostStub::gpioCallbacks();
  XWalkGpio gpio(&backend, callbackSet, "LED");
  XWalkLed led(gpio);
  led.on();
  const boolean onValid = led.isOn() && backend.gpioState();
  led.toggle();
  const boolean toggleValid =
      (led.isOn() == false) && (backend.gpioState() == false);
  XWalkI2c i2c(&backend, &XWalkLedHostStub::probe,
               &XWalkLedHostStub::writeRegister, &XWalkLedHostStub::read);
  XWalkPwmTimerState timerState;
  XWalkPwm red(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
  XWalkPwm green(i2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
  XWalkPwm blue(i2c, 2U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
  XWalkRgbLed rgbLed(red, green, blue, XWalkRgbLedCommon::Cathode);
  const rgbcolor requestedColor{12U, 34U, 56U};
  rgbLed.setColor(requestedColor);
  const boolean valid = onValid && toggleValid &&
                        (rgbLed.color() == requestedColor) &&
                        (backend.i2cWriteCount() > 0U);
  XWALK_HAL_TRACE_UID0(RPI .268, "xWalkLed host simulation completed");
  return valid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
