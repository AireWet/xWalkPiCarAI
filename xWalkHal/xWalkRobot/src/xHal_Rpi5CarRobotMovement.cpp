/******************************************************************************
 * @file        xHal_Rpi5CarRobotMovement.cpp
 * @brief       Implements interpolated xWalk robot motion and named actions.
 *
 * @details
 * Calculates bounded motion duration, limits degrees per second, advances all
 * servos together, compensates for command time, and executes stored frames.
 *
 * @project     xWalk Firmware
 * @module      xWalkRobot
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

#include "xHal_Rpi5CarRobot.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Interpolates every servo from its current position to one target frame.
 *
 * @param[in] targets
 * One finite logical target angle in degrees per servo.
 *
 * @param[in] speedPercent
 * Finite speed clamped to 0.0 through 100.0 percent.
 *
 * @param[in] beatsPerMinute
 * Optional finite value greater than zero that overrides speed-based duration.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @throws invalidargument
 * If an input is non-finite, the target count is wrong, or BPM is not positive.
 *
 * @throws outofrange
 * If the largest angular change exceeds the unsigned 32-bit range.
 */
void XWalkRobot::servoMove(const float64vector& targets, float64 speedPercent,
    optionalfloat64 beatsPerMinute)
{
    requireInitialized();
    validateAngles(targets, "Robot movement target");
    const hal::boolean speedPercentNotFinite =
        static_cast<hal::boolean>(
            !XHAL_IS_FINITE(speedPercent));
    if (speedPercentNotFinite)
    {
        XHAL_THROW_INVALID_ARGUMENT("Robot movement speed must be finite");
    }
    const hal::boolean beatsPerMinuteHasValueValueInvalid =
        static_cast<hal::boolean>(
            beatsPerMinute.has_value() &&
        (!XHAL_IS_FINITE(beatsPerMinute.value()) || (beatsPerMinute.value() <= 0.0)));
    if (beatsPerMinuteHasValueValueInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Robot movement BPM must be finite and positive");
    }

    float64 clampedSpeedPercent = speedPercent;
    if (clampedSpeedPercent < 0.0)
    {
        clampedSpeedPercent = 0.0;
    }
    if (clampedSpeedPercent > 100.0)
    {
        clampedSpeedPercent = 100.0;
    }

    float64 maximumDeltaDegrees = 0.0;
    for (uint32 index = 0U; index < servoCountValue; ++index)
    {
        const float64 deltaDegrees = XHAL_ABSOLUTE_VALUE(targets[index] - servoPositionsValue[index]);
        maximumDeltaDegrees = XHAL_MAXIMUM_VALUE(maximumDeltaDegrees, deltaDegrees);
    }
    const float64 maximumUint32 = static_cast<float64>(common::UINT32_MAXIMUM);
    if (maximumDeltaDegrees > maximumUint32)
    {
        XHAL_THROW_OUT_OF_RANGE("Robot movement delta exceeds its supported range");
    }

    const uint32 truncatedMaximumDelta = static_cast<uint32>(maximumDeltaDegrees);
    if (truncatedMaximumDelta == 0U)
    {
        common::sleepMilliseconds(XHAL_RPI5CAR_ROBOT_STEP_TIME_MS);
        return;
    }

    float64 totalTimeMs = 0.0;
    const hal::boolean beatsPerMinuteProvided =
        static_cast<hal::boolean>(
            beatsPerMinute.has_value());
    if (beatsPerMinuteProvided)
    {
        const float64 minuteMs = 60'000.0;
        totalTimeMs = minuteMs / beatsPerMinute.value();
    }
    else
    {
        const float64 speedScaleMs = 9.9 * clampedSpeedPercent;
        totalTimeMs = 1'000.0 - speedScaleMs;
    }

    const float64 truncatedDeltaDegrees = static_cast<float64>(truncatedMaximumDelta);
    const float64 millisecondsPerSecond = 1'000.0;
    const float64 scaledDeltaDegrees = truncatedDeltaDegrees * millisecondsPerSecond;
    const float64 currentMaximumDps = scaledDeltaDegrees / totalTimeMs;
    if (currentMaximumDps > XHAL_RPI5CAR_ROBOT_MAX_DPS)
    {
        totalTimeMs = scaledDeltaDegrees / XHAL_RPI5CAR_ROBOT_MAX_DPS;
    }

    const float64 stepTimeMs = static_cast<float64>(XHAL_RPI5CAR_ROBOT_STEP_TIME_MS);
    const float64 calculatedStepCount = totalTimeMs / stepTimeMs;
    const hal::boolean calculatedStepCountMaximumUint32Invalid =
        static_cast<hal::boolean>(
            !XHAL_IS_FINITE(calculatedStepCount) || (calculatedStepCount > maximumUint32));
    if (calculatedStepCountMaximumUint32Invalid)
    {
        XHAL_THROW_OUT_OF_RANGE("Robot movement duration exceeds its supported range");
    }
    uint32 stepCount = static_cast<uint32>(calculatedStepCount);
    if (stepCount == 0U)
    {
        stepCount = 1U;
    }

    const float64vector startPositions = servoPositionsValue;
    for (uint32 stepIndex = 0U; stepIndex < stepCount; ++stepIndex)
    {
        const uint64 startTimeUs = common::monotonicMicroseconds();
        const float64 completedSteps = static_cast<float64>(stepIndex + 1U);
        const float64 totalSteps = static_cast<float64>(stepCount);
        const float64 completionRatio = completedSteps / totalSteps;

        for (uint32 servoIndex = 0U; servoIndex < servoCountValue; ++servoIndex)
        {
            const float64 deltaDegrees = targets[servoIndex] - startPositions[servoIndex];
            const float64 completedDeltaDegrees = deltaDegrees * completionRatio;
            servoPositionsValue[servoIndex] = startPositions[servoIndex] + completedDeltaDegrees;
        }
        servoWriteAll(servoPositionsValue);

        const uint64 elapsedUs = common::monotonicMicroseconds() - startTimeUs;
        const uint64 stepDurationUs = static_cast<uint64>(XHAL_RPI5CAR_ROBOT_STEP_TIME_MS) * 1'000U;
        if (elapsedUs < stepDurationUs)
        {
            const uint64 remainingUs = stepDurationUs - elapsedUs;
            common::sleepMilliseconds(static_cast<uint32>(remainingUs / 1'000U));
        }
    }
}

/**
 * @brief Stores or replaces a named sequence of logical servo frames.
 *
 * @param[in] actionName
 * Non-empty action identifier.
 *
 * @param[in] motions
 * Ordered logical frames, each containing one finite angle per servo.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @throws invalidargument
 * If the action name is empty or any frame is invalid.
 */
void XWalkRobot::setAction(stringview actionName, const float64vectorvector& motions)
{
    requireInitialized();
    const hal::boolean actionNameEmpty =
        static_cast<hal::boolean>(
            actionName.empty());
    if (actionNameEmpty)
    {
        XHAL_THROW_INVALID_ARGUMENT("Robot action name must not be empty");
    }
    for (const float64vector& motion : motions)
    {
        validateAngles(motion, "Robot action frame");
    }
    actions[string(actionName)] = motions;
}

/**
 * @brief Executes a named action for the requested repetition count.
 *
 * @param[in] actionName
 * Previously registered action identifier.
 *
 * @param[in] repetitions
 * Number of complete action executions; zero performs no movement.
 *
 * @param[in] speedPercent
 * Finite speed clamped to 0.0 through 100.0 percent.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @throws invalidargument
 * If the speed or a stored action frame is invalid.
 *
 * @throws outofrange
 * If `actionName` is unknown or the calculated movement duration is unsupported.
 */
void XWalkRobot::doAction(stringview actionName, uint32 repetitions, float64 speedPercent)
{
    requireInitialized();
    const auto action = actions.find(string(actionName));
    const hal::boolean actionActionsMatched =
        static_cast<hal::boolean>(
            action == actions.end());
    if (actionActionsMatched)
    {
        XHAL_THROW_OUT_OF_RANGE("Robot action was not found");
    }
    for (uint32 repetition = 0U; repetition < repetitions; ++repetition)
    {
        for (const float64vector& motion : action->second)
        {
            servoMove(motion, speedPercent);
        }
    }
}

} /* namespace xwalk::hal */
