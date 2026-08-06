/******************************************************************************
 * @file        xAgent_Rpi5CarServoMotorCalibration.cpp
 * @brief       Implements bounded servo and motor calibration behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkServoMotorCalibration
 *
 * @author      Joxy John
 * @date        2026-08-04
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

#include "xAgent_Rpi5CarServoMotorCalibration.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/**
 * @brief Commands one logical angle on a selected servo.
 * @param[in] servoId Zero for steering, one for pan, or two for tilt.
 * @param[in] angleDegrees Logical angle in degrees.
 * @throws std::out_of_range If `servoId` exceeds two.
 */
void XWalkServoMotorCalibration::setServoAngle(
    agent::uint8 servoId, agent::float64 angleDegrees)
{
    if (servoId == 0U)
    {
        picarxObject->setDirectionServoAngle(angleDegrees);
    }
    else if (servoId == 1U)
    {
        picarxObject->setCameraPanAngle(angleDegrees);
    }
    else if (servoId == 2U)
    {
        picarxObject->setCameraTiltAngle(angleDegrees);
    }
    else
    {
        XHAL_THROW_OUT_OF_RANGE("Servo calibration identifier must be 0, 1, or 2");
    }
}

/**
 * @brief Applies one pending servo offset and centers its logical command.
 * @param[in] servoId Zero for steering, one for pan, or two for tilt.
 * @return `true` after the centered preview or `false` after cancellation.
 */
agent::boolean XWalkServoMotorCalibration::applyServoOffset(agent::uint8 servoId)
{
    if (servoId == 0U)
    {
        picarxObject->previewDirectionServoCalibration(resultValue.servoOffsets[servoId]);
    }
    else if (servoId == 1U)
    {
        picarxObject->previewCameraPanServoCalibration(resultValue.servoOffsets[servoId]);
    }
    else if (servoId == 2U)
    {
        picarxObject->previewCameraTiltServoCalibration(resultValue.servoOffsets[servoId]);
    }
    else
    {
        XHAL_THROW_OUT_OF_RANGE("Servo calibration identifier must be 0, 1, or 2");
    }
    setServoAngle(servoId, 0.0);
    return wait(200U);
}

/**
 * @brief Centers all three logical servo commands with source timing.
 * @return `true` after completion or `false` after cancellation.
 */
agent::boolean XWalkServoMotorCalibration::resetServos()
{
    for (agent::uint8 servoId = 0U; servoId < 3U; ++servoId)
    {
        setServoAngle(servoId, 0.0);
        const agent::boolean delayCompleted = wait(200U);
        if (delayCompleted == false)
        {
            stop();
            return false;
        }
    }
    return true;
}

/**
 * @brief Sweeps steering, pan, and tilt through minus 30, plus 30, and zero.
 * @return `true` after all nine commands or `false` after cancellation.
 * @warning Physically moves all three servo mechanisms.
 */
agent::boolean XWalkServoMotorCalibration::testServos()
{
    constexpr agent::fixedarray<agent::float64, 3U> testAngles{-30.0, 30.0, 0.0};
    for (agent::uint8 servoId = 0U; servoId < 3U; ++servoId)
    {
        for (const agent::float64 angleDegrees : testAngles)
        {
            setServoAngle(servoId, angleDegrees);
            const agent::boolean delayCompleted = wait(500U);
            if (delayCompleted == false)
            {
                stop();
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Applies one pending servo offset without persisting it.
 * @param[in] servoId Zero for steering, one for pan, or two for tilt.
 * @param[in] offsetDegrees Offset from minus 20 through plus 20 degrees.
 * @return `true` after centered preview or `false` after cancellation.
 */
agent::boolean XWalkServoMotorCalibration::setServoOffset(
    agent::uint8 servoId, agent::float64 offsetDegrees)
{
    const agent::boolean offsetDegreesNotFinite =
        static_cast<agent::boolean>(
            !XHAL_IS_FINITE(offsetDegrees));
    if (offsetDegreesNotFinite)
    {
        XHAL_THROW_INVALID_ARGUMENT("Servo calibration offset must be finite");
    }
    if ((servoId > 2U) || (offsetDegrees < -20.0) || (offsetDegrees > 20.0))
    {
        XHAL_THROW_OUT_OF_RANGE("Servo calibration requires identifier 0-2 and offset -20 through 20");
    }
    resultValue.servoOffsets[servoId] = offsetDegrees;
    return applyServoOffset(servoId);
}

/**
 * @brief Applies one pending motor direction without persisting it.
 * @param[in] motorId One for left or two for right.
 * @param[in] direction Direction equal to 1 or -1.
 */
void XWalkServoMotorCalibration::setMotorDirection(
    agent::uint8 motorId, agent::int32 direction)
{
    picarxObject->previewMotorDirection(motorId, direction);
    resultValue.motorDirections[static_cast<agent::size>(motorId - 1U)] = direction;
}

/**
 * @brief Reverses one pending motor direction and starts source preview power.
 * @param[in] motorId One for left or two for right.
 */
void XWalkServoMotorCalibration::toggleMotorDirection(agent::uint8 motorId)
{
    if ((motorId < 1U) || (motorId > 2U))
    {
        XHAL_THROW_OUT_OF_RANGE("Motor calibration identifier must be 1 or 2");
    }
    const agent::size index = static_cast<agent::size>(motorId - 1U);
    setMotorDirection(motorId, -resultValue.motorDirections[index]);
    setMotorRunning(true);
}

/**
 * @brief Starts or stops the source-compatible 30-percent forward preview.
 * @param[in] running `true` to run or `false` to stop both motors.
 */
void XWalkServoMotorCalibration::setMotorRunning(agent::boolean running)
{
    if (running)
    {
        picarxObject->forward(30.0);
    }
    else
    {
        picarxObject->stop();
    }
}

/** @brief Persists all pending servo offsets and motor directions. */
void XWalkServoMotorCalibration::save()
{
    picarxObject->calibrateDirectionServo(resultValue.servoOffsets[0U]);
    picarxObject->calibrateCameraPanServo(resultValue.servoOffsets[1U]);
    picarxObject->calibrateCameraTiltServo(resultValue.servoOffsets[2U]);
    picarxObject->calibrateMotorDirection(1U, resultValue.motorDirections[0U]);
    picarxObject->calibrateMotorDirection(2U, resultValue.motorDirections[1U]);
    static_cast<void>(wait(200U));
}

/**
 * @brief Returns pending values without persisting them.
 * @return Non-owning result reference valid for this object lifetime.
 */
const XWalkServoMotorCalibrationResult& XWalkServoMotorCalibration::result() const noexcept
{
    return resultValue;
}

} /* namespace xwalk::agent */
