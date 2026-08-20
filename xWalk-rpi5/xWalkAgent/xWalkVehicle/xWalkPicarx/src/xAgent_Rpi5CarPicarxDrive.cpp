/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxDrive.cpp
 * @brief       Implements PiCar-X drive and steering operations.
 *
 * @details
 * Applies upstream-compatible motor scaling and Ackermann-style inside-wheel
 *reduction through paired motors.
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

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains application coordinators for the
 * xWalk firmware. */
namespace xwalk::agent
{

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Converts and applies one Python-compatible raw motor command.
     * @param[in] motorId One-based motor identifier in the range one through two.
     * @param[in] speedPercent Raw speed command in the range -100 to 100 percent.
     * @return Signed calibrated HAL speed in percent.
     */
    agent::float64 XWalkPicarx::calibratedMotorSpeed(agent::uint8 motorId, agent::float64 speedPercent) const
    {
        if ((motorId < 1U) || (motorId > 2U))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "PiCar-X motor identifier must be 1 or 2");
        }
        const agent::float64 constrained = constrain(speedPercent, -100.0, 100.0);
        if (constrained == 0.0)
        {
            return 0.0;
        }
        const agent::float64 sign = (constrained < 0.0) ? -1.0 : 1.0;
        const agent::float64 magnitude = XHAL_ABSOLUTE_VALUE(constrained);
        const agent::float64 scaledMagnitude = (magnitude / 2.0) + 50.0;
        const agent::uint32 index = static_cast<agent::uint32>(motorId - 1U);
        const agent::float64 calibratedMagnitude =
            constrain(scaledMagnitude - motorSpeedCalibrationValues[index], 0.0, 100.0);
        const agent::float64 limitedMagnitude = constrain(calibratedMagnitude, 0.0, maximumMotorOutputPercentValue);
        return sign * limitedMagnitude;
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Sets one motor command using a one-based motor identifier.
     * @param[in] motorId One-based motor identifier in the range one through two.
     * @param[in] speedPercent Raw speed command constrained to -100 through 100
     * percent.
     */
    void XWalkPicarx::setMotorSpeed(agent::uint8 motorId, agent::float64 speedPercent)
    {
        const agent::boolean emergencyStopRequested = static_cast<agent::boolean>(emergencyStopRequestedValue.load());
        if (emergencyStopRequested)
        {
            return;
        }
        const agent::float64 command = calibratedMotorSpeed(motorId, speedPercent);
        const agent::float64 left = (motorId == 1U) ? command : motorsObject->left().speed();
        const agent::float64 right = (motorId == 2U) ? command : motorsObject->right().speed();
        motorsObject->setSpeed(left, right);
    }

    /**
     * @brief Drives both sides with the same raw command.
     * @param[in] speedPercent Raw speed command constrained to -100 through 100
     * percent.
     */
    void XWalkPicarx::setPower(agent::float64 speedPercent)
    {
        const agent::boolean emergencyStopRequested = static_cast<agent::boolean>(emergencyStopRequestedValue.load());
        if (emergencyStopRequested)
        {
            return;
        }
        const agent::float64 left = calibratedMotorSpeed(1U, speedPercent);
        const agent::float64 right = calibratedMotorSpeed(2U, speedPercent);
        motorsObject->setSpeed(left, right);
    }

    /**
     * @brief Drives forward while reducing the inside wheel for steering.
     * @param[in] speedPercent Raw speed command constrained to -100 through 100
     * percent.
     */
    void XWalkPicarx::forward(agent::float64 speedPercent)
    {
        const agent::boolean emergencyStopRequested = static_cast<agent::boolean>(emergencyStopRequestedValue.load());
        if (emergencyStopRequested)
        {
            return;
        }
        const agent::float64 speed = constrain(speedPercent, -100.0, 100.0);
        const agent::float64 absoluteAngle = XHAL_ABSOLUTE_VALUE(directionAngleDegreesValue);
        const agent::float64 powerScale = (100.0 - absoluteAngle) / 100.0;
        const agent::float64 leftRaw = (directionAngleDegreesValue > 0.0) ? speed * powerScale : speed;
        const agent::float64 rightRaw = (directionAngleDegreesValue < 0.0) ? speed * powerScale : speed;
        motorsObject->setSpeed(calibratedMotorSpeed(1U, leftRaw), calibratedMotorSpeed(2U, rightRaw));
    }

    /**
     * @brief Drives backward while reducing the inside wheel for steering.
     * @param[in] speedPercent Raw speed command constrained to -100 through 100
     * percent before negation.
     */
    void XWalkPicarx::backward(agent::float64 speedPercent)
    {
        const agent::boolean emergencyStopRequested = static_cast<agent::boolean>(emergencyStopRequestedValue.load());
        if (emergencyStopRequested)
        {
            return;
        }
        const agent::float64 speed = -constrain(speedPercent, -100.0, 100.0);
        const agent::float64 absoluteAngle = XHAL_ABSOLUTE_VALUE(directionAngleDegreesValue);
        const agent::float64 powerScale = (100.0 - absoluteAngle) / 100.0;
        const agent::float64 leftRaw = (directionAngleDegreesValue > 0.0) ? speed : speed * powerScale;
        const agent::float64 rightRaw = (directionAngleDegreesValue > 0.0) ? speed * powerScale : speed;
        motorsObject->setSpeed(calibratedMotorSpeed(1U, leftRaw), calibratedMotorSpeed(2U, rightRaw));
    }

    /** @brief Stops both drive motors. */
    void XWalkPicarx::stop()
    {
        motorsObject->stop();
    }

    /**
     * @brief Attempts to refresh the active motor watchdog.
     * @return `true` while motor movement remains armed; otherwise `false`.
     */
    agent::boolean XWalkPicarx::refreshMotorWatchdog() noexcept
    {
        return motorsObject->heartbeatSafely();
    }

    /**
     * @brief Latches actuator suppression and makes a non-throwing paired motor
     * stop attempt.
     * @return `true` when every motor PWM output accepted zero percent; otherwise
     * `false`.
     * @post Later actuator commands are suppressed until `clearEmergencyStop()` is
     * called.
     */
    agent::boolean XWalkPicarx::emergencyStop() noexcept
    {
        emergencyStopRequestedValue.store(true);
        return motorsObject->disarm();
    }

    /**
     * @brief Clears the emergency latch before a new application-controlled
     * operation.
     * @post Later actuator commands are accepted until another emergency stop is
     * requested.
     */
    void XWalkPicarx::clearEmergencyStop()
    {
        motorsObject->arm();
        emergencyStopRequestedValue.store(false);
    }

    /**
     * @brief Returns whether emergency actuator suppression is latched.
     * @return `true` after an emergency stop and before the next explicit clear;
     * otherwise `false`.
     */
    agent::boolean XWalkPicarx::emergencyStopRequested() const noexcept
    {
        return emergencyStopRequestedValue.load();
    }

    /**
     * @brief Returns the configured maximum applied motor PWM magnitude.
     * @return Limit in the inclusive range zero through one hundred percent.
     */
    agent::float64 XWalkPicarx::maximumMotorOutputPercent() const noexcept
    {
        return maximumMotorOutputPercentValue;
    }

    /**
     * @brief Stops the motors and centers all logical actuator commands.
     * @post Motors are stopped and logical steering, pan, and tilt commands are
     * zero.
     */
    void XWalkPicarx::reset()
    {
        stop();
        setDirectionServoAngle(0.0);
        setCameraTiltAngle(0.0);
        setCameraPanAngle(0.0);
    }

    /**
     * @brief Disarms motors and closes ultrasonic registrations without commanding
     * servo movement.
     * @post Motors are disarmed, actuator commands are suppressed, and ultrasonic
     * registrations are cancelled.
     */
    void XWalkPicarx::close()
    {
        const agent::boolean stopped = emergencyStop();
        initializedValue = false;
        if (stopped == false)
        {
            XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, "PiCar-X shutdown could not disable every motor output");
        }
        ultrasonicObject->close();
    }

    /** @brief Returns the current constrained steering command in degrees. */
    agent::float64 XWalkPicarx::directionAngleDegrees() const noexcept
    {
        return directionAngleDegreesValue;
    }

} /* namespace xwalk::agent */
