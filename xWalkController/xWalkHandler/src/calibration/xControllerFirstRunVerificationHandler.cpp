/******************************************************************************
 * @file        xControllerFirstRunVerificationHandler.cpp
 * @brief       Implements the FirstRunVerificationHandler command
 *responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based
 *handler group.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-08-06
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

#include "xController.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl {

/******************************************************************************
 * Member function definitions
 ******************************************************************************/

/**
 * @brief Performs capped raised-wheel motor and steering verification.
 * @return `true` only when the operator confirms every required check.
 */
::ctrl::boolean XWalkController::XWALK_handlerFirstRunVerification() {
  picarxObject->recordCalibrationVerified(false);
  XWALK_CTRL_TRACE_UID0(CTRL .014,
                        "--- Raised-Wheel Actuator Verification ---");
  XWALK_CTRL_TRACE_UID0(
      CTRL .015,
      "Raise all wheels, clear the area, and be ready to stop the vehicle.");
  const ::ctrl::string readiness =
      input("Type 'raised' to begin low-output checks, or 'skip': ");
  if ((readiness != "raised") && (readiness != "RAISED")) {
    return false;
  }

  picarxObject->setMotorSpeed(1U, 100.0);
  const ::ctrl::boolean leftMotorDelayCompleted =
      delayWhileOperationRequested(500U);
  if (leftMotorDelayCompleted == false) {
    return false;
  }
  picarxObject->stop();
  const ::ctrl::string leftPassed =
      input("Did the left motor rotate forward? (y/n): ");
  if ((leftPassed != "y") && (leftPassed != "Y")) {
    return false;
  }

  picarxObject->setMotorSpeed(2U, 100.0);
  const ::ctrl::boolean rightMotorDelayCompleted =
      delayWhileOperationRequested(500U);
  if (rightMotorDelayCompleted == false) {
    return false;
  }
  picarxObject->stop();
  const ::ctrl::string rightPassed =
      input("Did the right motor rotate forward? (y/n): ");
  if ((rightPassed != "y") && (rightPassed != "Y")) {
    return false;
  }

  picarxObject->setPower(100.0);
  const ::ctrl::boolean pairedMotorDelayCompleted =
      delayWhileOperationRequested(500U);
  if (pairedMotorDelayCompleted == false) {
    return false;
  }
  picarxObject->stop();
  const ::ctrl::string balancePassed = input(
      "Did both motors run in the expected direction and balance? (y/n): ");
  if ((balancePassed != "y") && (balancePassed != "Y")) {
    return false;
  }
  const ::ctrl::string steeringPassed =
      input("Is the steering centered? (y/n): ");
  return (steeringPassed == "y") || (steeringPassed == "Y");
}

} /* namespace xwalk::ctrl */
