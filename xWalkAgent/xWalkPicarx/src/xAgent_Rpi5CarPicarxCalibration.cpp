/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxCalibration.cpp
 * @brief       Implements PiCar-X calibration and servo operations.
 *
 * @details
 * Persists motor and servo calibration and applies constrained actuator commands.
 *
 * @project     xWalk Firmware
 * @module      xWalkPicarx
 *
 * @author      Joxy John
 * @date        2026-07-31
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

#include "xAgent_Rpi5CarPicarx.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains application coordinators for the xWalk firmware. */
namespace xwalk::agent
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Sets the signed motor-speed calibration.
 * @param[in] value Correction from -100 through 100 percentage points; negative corrects the right motor.
 */
void XWalkPicarx::calibrateMotorSpeed(hal::float64 value)
{
    const hal::float64 correction = constrain(value, -100.0, 100.0);
    motorSpeedCalibrationValues = {};
    if (correction < 0.0)
    {
        motorSpeedCalibrationValues[1U] = -correction;
    }
    else
    {
        motorSpeedCalibrationValues[0U] = correction;
    }
    configStoreObject->set("picarx_motor_speed_calibration",
        hal::common::float64ToString(correction));
}

/**
 * @brief Persists whether all first-run motor and steering checks passed.
 * @param[in] verified `true` only after motor direction, steering center, and motor balance checks pass.
 */
void XWalkPicarx::recordCalibrationVerified(hal::boolean verified)
{
    calibrationVerifiedValue = verified;
    configStoreObject->set("picarx_calibration_verified", verified ? "true" : "false");
    maximumMotorOutputPercentValue = verified ? configuredMaximumMotorOutputPercentValue :
        ((configuredMaximumMotorOutputPercentValue < 20.0) ?
            configuredMaximumMotorOutputPercentValue : 20.0);
}

/** @brief Reports whether all first-run motor and steering checks passed. */
hal::boolean XWalkPicarx::calibrationVerified() const noexcept
{
    return calibrationVerifiedValue;
}

/**
 * @brief Persists motor direction as 1 or -1 for one one-based motor identifier.
 * @param[in] motorId One-based motor identifier in the range one through two.
 * @param[in] direction Direction value equal to 1 or -1.
 */
void XWalkPicarx::calibrateMotorDirection(hal::uint8 motorId, hal::int32 direction)
{
    if ((motorId < 1U) || (motorId > 2U))
    {
        XHAL_THROW_OUT_OF_RANGE("PiCar-X motor identifier must be 1 or 2");
    }
    if ((direction != 1) && (direction != -1))
    {
        XHAL_THROW_INVALID_ARGUMENT("PiCar-X motor direction must be 1 or -1");
    }
    if (motorId == 1U)
    {
        motorsObject->setLeftReversed(direction < 0);
    }
    else
    {
        motorsObject->setRightReversed(direction < 0);
    }
    const hal::XWalkMotorsConfiguration configuration = motorsObject->configuration();
    const hal::int32 left = configuration.leftReversed ? -1 : 1;
    const hal::int32 right = configuration.rightReversed ? -1 : 1;
    const hal::string stored = hal::string("[") + hal::common::int32ToString(left) + ", " +
        hal::common::int32ToString(right) + "]";
    configStoreObject->set("picarx_dir_motor", stored);
}

/** @brief Persists and applies the steering-servo calibration offset in degrees. */
void XWalkPicarx::calibrateDirectionServo(hal::float64 value)
{
    directionCalibrationDegreesValue = constrain(value, -90.0, 90.0);
    configStoreObject->set(
        "picarx_dir_servo", hal::common::float64ToString(directionCalibrationDegreesValue));
    directionServoObject->setAngle(directionCalibrationDegreesValue);
}

/** @brief Sets the steering command, constrained to -30 through 30 degrees. */
void XWalkPicarx::setDirectionServoAngle(hal::float64 value)
{
    if (emergencyStopRequestedValue.load())
    {
        return;
    }
    directionAngleDegreesValue = constrain(value, -30.0, 30.0);
    directionServoObject->setAngle(directionAngleDegreesValue + directionCalibrationDegreesValue);
}

/** @brief Persists and applies the camera-pan calibration offset in degrees. */
void XWalkPicarx::calibrateCameraPanServo(hal::float64 value)
{
    cameraPanCalibrationDegreesValue = constrain(value, -90.0, 90.0);
    configStoreObject->set("picarx_cam_pan_servo", hal::common::float64ToString(
        cameraPanCalibrationDegreesValue));
    cameraPanServoObject->setAngle(cameraPanCalibrationDegreesValue);
}

/** @brief Persists and applies the camera-tilt calibration offset in degrees. */
void XWalkPicarx::calibrateCameraTiltServo(hal::float64 value)
{
    cameraTiltCalibrationDegreesValue = constrain(value, -90.0, 90.0);
    configStoreObject->set("picarx_cam_tilt_servo", hal::common::float64ToString(
        cameraTiltCalibrationDegreesValue));
    cameraTiltServoObject->setAngle(cameraTiltCalibrationDegreesValue);
}

/** @brief Sets camera pan, constrained to -90 through 90 degrees. */
void XWalkPicarx::setCameraPanAngle(hal::float64 value)
{
    if (emergencyStopRequestedValue.load())
    {
        return;
    }
    const hal::float64 constrained = constrain(value, -90.0, 90.0);
    cameraPanServoObject->setAngle(cameraPanCalibrationDegreesValue - constrained);
}

/** @brief Sets camera tilt, constrained to -35 through 65 degrees. */
void XWalkPicarx::setCameraTiltAngle(hal::float64 value)
{
    if (emergencyStopRequestedValue.load())
    {
        return;
    }
    const hal::float64 constrained = constrain(value, -35.0, 65.0);
    cameraTiltServoObject->setAngle(cameraTiltCalibrationDegreesValue - constrained);
}

} /* namespace xwalk::agent */
