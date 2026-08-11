/******************************************************************************
 * @file        xHal_Rpi5CarServoLifecycle.cpp
 * @brief       Implements lifecycle operations for the servo abstraction.
 *
 * @details
 * Stores the caller-created PWM dependency and configures its shared timer for
 * the period and prescaler used by the Python Robot HAT servo implementation.
 *
 * @project     xWalk Firmware
 * @module      xWalkServo
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarServo.h"

#include "xHal_Rpi5CarMath.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a servo controller around a PWM channel.
 *
 * @param[in] pwm
 * Caller-created PWM channel passed by reference.
 *
 * @pre
 * `pwm` outlives this servo object.
 *
 * @post
 * The PWM timer period is 4095 counts and its rounded prescaler is configured
 * for an approximately 50 Hertz servo frame.
 */
XWalkServo::XWalkServo(XWalkPwm &pwm,
                       const XWalkServoConfiguration &configuration)
    : pwmObject(&pwm), configurationValue(configuration) {
  validateConfiguration(configurationValue);
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

boolean XWalkServo::initialize() {
  if (initializedValue) {
    return false;
  }

  pwmObject->setPeriod(XHAL_RPI5CAR_SERVO_PERIOD);

  const float64 pwmClockHz = static_cast<float64>(XHAL_RPI5CAR_PWM_CLOCK_HZ);
  const float64 servoFrequencyHz =
      static_cast<float64>(XHAL_RPI5CAR_SERVO_FREQUENCY_HZ);
  const float64 servoPeriod = static_cast<float64>(XHAL_RPI5CAR_SERVO_PERIOD);
  const float64 servoTimerDivisor = servoFrequencyHz * servoPeriod;
  const float64 prescaler = pwmClockHz / servoTimerDivisor;
  pwmObject->setPrescaler(prescaler);
  initializedValue = true;
  XWALK_HAL_TRACE_UID0(RPI .183,
                       "Servo initialized with a 4095-count, 50 Hz frame");
  return true;
}

boolean XWalkServo::isInitialized() const noexcept { return initializedValue; }

void XWalkServo::moveToSafePosition() {
  setAngle(configurationValue.centreAngleDegrees);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the servo controller.
 *
 * @note
 * The PWM pointer is non-owning and is not released.
 */
XWalkServo::~XWalkServo() = default;

} /* namespace xwalk::hal */
