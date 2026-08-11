/******************************************************************************
 * @file        xHal_Rpi5CarServoExampleLinux.cpp
 * @brief       Implements Linux I2C composition for the servo example.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServoExampleLinux.h"

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarI2cLinux.h"

#include "xHal_Rpi5CarTrace.h"
#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains Linux adapters for ported example programs.
 */
namespace xwalk::hal::example {

/**
 * @brief Runs bounded physical sweeps on servo channel one.
 * @param[in] i2cDevice Linux I2C character-device path.
 * @param[in] cycleCount Complete cycles from one through 100.
 * @warning Physically moves the servo through its full supported range.
 */
void XWalkServoExampleLinux::run(cstring i2cDevice, uint32 cycleCount) {
  XWalkI2cLinux i2cBackend(i2cDevice);
  XWalkI2c i2c(&i2cBackend, XHAL_I2C_PROBE_CALLBACK(XWalkI2cLinux),
               XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkI2cLinux),
               XHAL_I2C_READ_CALLBACK(XWalkI2cLinux));
  XWalkPwmTimerState timerState;
  XWalkPwm pwm(i2c, 1U, {}, timerState);
  XWalkServo servo(pwm);
  static_cast<void>(servo.initialize());
  servoObject = &servo;
  const XWalkServoExampleCallbacks exampleCallbacks{&setAngle, &wait, &report};
  XWalkServoExample example(this, exampleCallbacks);
  try {
    example.run(cycleCount);
    servoObject = nullptr;
  } catch (...) {
    servoObject = nullptr;
    throw;
  }
}

/**
 * @brief Resolves one callback context with a bound servo.
 * @param[in,out] context Non-null Linux adapter context.
 * @return Referenced adapter.
 * @throws std::invalid_argument If the adapter or servo binding is invalid.
 */
XWalkServoExampleLinux &
XWalkServoExampleLinux::adapter(contextpointer context) {
  if (context == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Servo Linux context must not be null");
  }
  XWalkServoExampleLinux &self =
      *static_cast<XWalkServoExampleLinux *>(context);
  if (self.servoObject == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Servo Linux adapter has no bound servo");
  }
  return self;
}

/** @brief Commands one angle on the bound servo. */
void XWalkServoExampleLinux::setAngle(contextpointer context,
                                      float64 angleDegrees) {
  adapter(context).servoObject->setAngle(angleDegrees);
}

/** @brief Waits for one source-compatible duration. */
void XWalkServoExampleLinux::wait(contextpointer context,
                                  uint32 durationMilliseconds) {
  static_cast<void>(adapter(context));
  common::sleepMilliseconds(durationMilliseconds);
}

/** @brief Prints one source-compatible angle with a carriage return. */
void XWalkServoExampleLinux::report(contextpointer context,
                                    int32 angleDegrees) {
  static_cast<void>(adapter(context));
  std::cout << angleDegrees << "  \r" << std::flush;
}

} /* namespace xwalk::hal::example */
