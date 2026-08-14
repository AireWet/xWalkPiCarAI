/******************************************************************************
 * @file        xHal_Rpi5CarMotorSimulation.cpp
 * @brief       Implements the device-free xWalkMotor simulation.
 * @project     xWalk Firmware
 * @module      xWalkMotor Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarMotorSimulation.h"
#include "xHal_Rpi5CarMotorHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
int32 runMotorSimulation() {
  XWalkMotorHostStub backend;
  XWalkI2c i2c(&backend, &XWalkMotorHostStub::probe,
               &XWalkMotorHostStub::writeRegister, &XWalkMotorHostStub::read,
               nullptr, &XWalkMotorHostStub::tryWriteRegister);
  XWalkPwmTimerState timerState;
  XWalkPwm pwm(i2c, 13U, 0x14U, timerState);
  const XWalkGpioCallbacks callbackSet = XWalkMotorHostStub::gpioCallbacks();
  XWalkGpio direction(&backend, callbackSet, "D4");
  XWalkMotor motor(pwm, direction);
  static_cast<void>(motor.initialize());
  motor.setSpeed(35.0);
  const boolean forwardValid = (motor.speed() == 35.0) && backend.direction();
  motor.setSpeed(-20.0);
  const boolean reverseValid =
      (motor.speed() == -20.0) && (backend.direction() == false);
  motor.stop();
  const boolean valid = forwardValid && reverseValid &&
                        (motor.speed() == 0.0) &&
                        (backend.i2cWriteCount() > 0U);
  XWALK_HAL_TRACE_UID0(RPI .253, "xWalkMotor host simulation completed");
  return valid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
