/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxCalibration.cpp
 * @brief       Implements PiCar-X calibration and servo operations.
 *
 * @details
 * Persists motor and servo calibration and applies constrained actuator
 *commands.
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

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains application coordinators for the
 * xWalk firmware. */
namespace xwalk::agent
{

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Sets the signed motor-speed calibration.
     * @param[in] value Correction from -100 through 100 percentage points; negative
     * corrects the right motor.
     */
    void XWalkPicarx::calibrateMotorSpeed(agent::float64 value)
    {
        const agent::float64 correction = constrain(value, -100.0, 100.0);
        motorSpeedCalibrationValues = {};
        if (correction < 0.0)
        {
            motorSpeedCalibrationValues[1U] = -correction;
        }
        else
        {
            motorSpeedCalibrationValues[0U] = correction;
        }
        configStoreObject->set("picarx_motor_speed_calibration", hal::common::float64ToString(correction));
    }

    /**
     * @brief Persists whether all first-run motor and steering checks passed.
     * @param[in] verified `true` only after motor direction, steering center, and
     * motor balance checks pass.
     */
    void XWalkPicarx::recordCalibrationVerified(agent::boolean verified)
    {
        calibrationVerifiedValue = verified;
        configStoreObject->set("picarx_calibration_verified", verified ? "true" : "false");
        maximumMotorOutputPercentValue =
            verified
                ? configuredMaximumMotorOutputPercentValue
                : ((configuredMaximumMotorOutputPercentValue < 20.0) ? configuredMaximumMotorOutputPercentValue : 20.0);
    }

    /** @brief Reports whether all first-run motor and steering checks passed. */
    agent::boolean XWalkPicarx::calibrationVerified() const noexcept
    {
        return calibrationVerifiedValue;
    }

    /**
     * @brief Persists motor direction as 1 or -1 for one one-based motor
     * identifier.
     * @param[in] motorId One-based motor identifier in the range one through two.
     * @param[in] direction Direction value equal to 1 or -1.
     */
    void XWalkPicarx::calibrateMotorDirection(agent::uint8 motorId, agent::int32 direction)
    {
        previewMotorDirection(motorId, direction);
        const agent::fixedarray<agent::int32, 2U> directions = motorDirections();
        const agent::string stored = agent::string("[") + hal::common::int32ToString(directions[0U]) + ", " +
                                     hal::common::int32ToString(directions[1U]) + "]";
        configStoreObject->set("picarx_dir_motor", stored);
    }

    /**
     * @brief Applies one motor direction without persisting it.
     * @param[in] motorId One-based motor identifier in the range one through two.
     * @param[in] direction Direction value equal to 1 or -1.
     */
    void XWalkPicarx::previewMotorDirection(agent::uint8 motorId, agent::int32 direction)
    {
        if ((motorId < 1U) || (motorId > 2U))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "PiCar-X motor identifier must be 1 or 2");
        }
        if ((direction != 1) && (direction != -1))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "PiCar-X motor direction must be 1 or -1");
        }
        if (motorId == 1U)
        {
            motorsObject->setLeftReversed(direction < 0);
        }
        else
        {
            motorsObject->setRightReversed(direction < 0);
        }
    }

    /**
     * @brief Returns the active left and right motor directions.
     * @return Direction values containing only 1 or -1.
     */
    agent::fixedarray<agent::int32, 2U> XWalkPicarx::motorDirections() const noexcept
    {
        const hal::XWalkMotorsConfiguration configuration = motorsObject->configuration();
        return {configuration.leftReversed ? -1 : 1, configuration.rightReversed ? -1 : 1};
    }

    /** @brief Persists and applies the steering-servo calibration offset in
     * degrees. */
    void XWalkPicarx::calibrateDirectionServo(agent::float64 value)
    {
        previewDirectionServoCalibration(value);
        configStoreObject->set("picarx_dir_servo", hal::common::float64ToString(directionCalibrationDegreesValue));
    }

    /** @brief Applies a steering-servo calibration offset without persistence. */
    void XWalkPicarx::previewDirectionServoCalibration(agent::float64 value)
    {
        directionCalibrationDegreesValue = constrain(value, -90.0, 90.0);
        directionServoObject->setAngle(directionCalibrationDegreesValue);
    }

    /** @brief Returns the active steering-servo calibration offset. */
    agent::float64 XWalkPicarx::directionServoCalibration() const noexcept
    {
        return directionCalibrationDegreesValue;
    }

    /** @brief Sets the steering command, constrained to -30 through 30 degrees. */
    void XWalkPicarx::setDirectionServoAngle(agent::float64 value)
    {
        const agent::boolean emergencyStopRequested = static_cast<agent::boolean>(emergencyStopRequestedValue.load());
        if (emergencyStopRequested)
        {
            return;
        }
        directionAngleDegreesValue = constrain(value, -30.0, 30.0);
        directionServoObject->setAngle(directionAngleDegreesValue + directionCalibrationDegreesValue);
    }

    /** @brief Persists and applies the camera-pan calibration offset in degrees. */
    void XWalkPicarx::calibrateCameraPanServo(agent::float64 value)
    {
        previewCameraPanServoCalibration(value);
        configStoreObject->set("picarx_cam_pan_servo", hal::common::float64ToString(cameraPanCalibrationDegreesValue));
    }

    /** @brief Applies a camera-pan calibration offset without persistence. */
    void XWalkPicarx::previewCameraPanServoCalibration(agent::float64 value)
    {
        cameraPanCalibrationDegreesValue = constrain(value, -90.0, 90.0);
        cameraPanServoObject->setAngle(cameraPanCalibrationDegreesValue);
    }

    /** @brief Returns the active camera-pan calibration offset. */
    agent::float64 XWalkPicarx::cameraPanServoCalibration() const noexcept
    {
        return cameraPanCalibrationDegreesValue;
    }

    /** @brief Persists and applies the camera-tilt calibration offset in degrees.
     */
    void XWalkPicarx::calibrateCameraTiltServo(agent::float64 value)
    {
        previewCameraTiltServoCalibration(value);
        configStoreObject->set("picarx_cam_tilt_servo",
                               hal::common::float64ToString(cameraTiltCalibrationDegreesValue));
    }

    /** @brief Applies a camera-tilt calibration offset without persistence. */
    void XWalkPicarx::previewCameraTiltServoCalibration(agent::float64 value)
    {
        cameraTiltCalibrationDegreesValue = constrain(value, -90.0, 90.0);
        cameraTiltServoObject->setAngle(cameraTiltCalibrationDegreesValue);
    }

    /** @brief Returns the active camera-tilt calibration offset. */
    agent::float64 XWalkPicarx::cameraTiltServoCalibration() const noexcept
    {
        return cameraTiltCalibrationDegreesValue;
    }

    /** @brief Sets camera pan, constrained to -90 through 90 degrees. */
    void XWalkPicarx::setCameraPanAngle(agent::float64 value)
    {
        const agent::boolean emergencyStopRequested = static_cast<agent::boolean>(emergencyStopRequestedValue.load());
        if (emergencyStopRequested)
        {
            return;
        }
        const agent::float64 constrained = constrain(value, -90.0, 90.0);
        cameraPanServoObject->setAngle(cameraPanCalibrationDegreesValue - constrained);
    }

    /** @brief Sets camera tilt, constrained to -35 through 65 degrees. */
    void XWalkPicarx::setCameraTiltAngle(agent::float64 value)
    {
        const agent::boolean emergencyStopRequested = static_cast<agent::boolean>(emergencyStopRequestedValue.load());
        if (emergencyStopRequested)
        {
            return;
        }
        const agent::float64 constrained = constrain(value, -35.0, 65.0);
        cameraTiltServoObject->setAngle(cameraTiltCalibrationDegreesValue - constrained);
    }

} /* namespace xwalk::agent */
