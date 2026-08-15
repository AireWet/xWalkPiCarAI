/******************************************************************************
 * @file        xAgent_Rpi5CarGrayscaleCalibration.cpp
 * @brief       Implements bounded automatic grayscale calibration.
 *
 * @details
 * Preserves the upstream steering, drive, sampling, reference calculation, and
 * deferred save sequence without retaining its unbounded worker threads.
 *
 * @project     xWalk Firmware
 * @module      xWalkGrayscaleCalibration
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

#include "xAgent_Rpi5CarGrayscaleCalibration.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /** @brief Samples once and updates line-channel extrema. */
    void XWalkGrayscaleCalibration::sampleLine()
    {
        const hal::linetrackervalues sample = picarxObject->grayscaleData();
        for (agent::uint32 index = 0U; index < 3U; ++index)
        {
            if (sample[index] < lineMinimumValues[index])
            {
                lineMinimumValues[index] = sample[index];
            }
            if (sample[index] > lineMaximumValues[index])
            {
                lineMaximumValues[index] = sample[index];
            }
            resultValue.lineReference[index] = (lineMinimumValues[index] + lineMaximumValues[index]) / 2;
        }
    }

    /**
     * @brief Samples line extrema at 200-millisecond intervals.
     * @param[in] durationMs Duration divisible by 200 milliseconds.
     * @return `true` after all samples or `false` after cancellation.
     */
    agent::boolean XWalkGrayscaleCalibration::sampleLineFor(agent::uint32 durationMs)
    {
        constexpr agent::uint32 sampleIntervalMs{200U};
        const agent::uint32 sampleCount = durationMs / sampleIntervalMs;
        for (agent::uint32 sampleIndex = 0U; sampleIndex < sampleCount; ++sampleIndex)
        {
            const agent::boolean operationRequested = continueCallback(callbackContext);
            if (operationRequested == false)
            {
                return false;
            }
            sampleLine();
            const agent::boolean delayCompleted = wait(sampleIntervalMs);
            if (delayCompleted == false)
            {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Applies one movement phase and collects line extrema.
     * @param[in] steeringDegrees Steering command in degrees.
     * @param[in] forwardMovement `true` for forward or `false` for backward.
     * @param[in] durationMs Movement duration divisible by 200 milliseconds.
     * @return `true` after all samples or `false` after cancellation.
     */
    agent::boolean XWalkGrayscaleCalibration::runLinePhase(agent::float64 steeringDegrees,
                                                           agent::boolean forwardMovement,
                                                           agent::uint32 durationMs)
    {
        picarxObject->setDirectionServoAngle(steeringDegrees);
        if (forwardMovement)
        {
            picarxObject->forward(10.0);
        }
        else
        {
            picarxObject->backward(10.0);
        }
        return sampleLineFor(durationMs);
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Sweeps steering through minus 30, plus 30, and zero degrees.
     * @return `true` after completion or `false` after cancellation.
     * @warning Physically moves the steering mechanism.
     */
    agent::boolean XWalkGrayscaleCalibration::runSteeringCheck()
    {
        picarxObject->setDirectionServoAngle(-30.0);
        const agent::boolean leftDelayCompleted = wait(500U);
        if (leftDelayCompleted == false)
        {
            stop();
            return false;
        }
        picarxObject->setDirectionServoAngle(30.0);
        const agent::boolean rightDelayCompleted = wait(500U);
        if (rightDelayCompleted == false)
        {
            stop();
            return false;
        }
        picarxObject->setDirectionServoAngle(0.0);
        const agent::boolean centerDelayCompleted = wait(500U);
        if (centerDelayCompleted == false)
        {
            stop();
            return false;
        }
        return true;
    }

    /**
     * @brief Drives the source left/right pattern while deriving line references.
     * @return `true` after completion or `false` after cancellation.
     * @warning Drives forward and backward; wheels require a reviewed clear surface.
     */
    agent::boolean XWalkGrayscaleCalibration::calibrateLine()
    {
        lineMinimumValues = {4'096, 4'096, 4'096};
        lineMaximumValues = {0, 0, 0};
        const agent::boolean leftForwardCompleted = runLinePhase(-35.0, true, 800U);
        if (leftForwardCompleted == false)
        {
            stop();
            return false;
        }
        const agent::boolean leftBackwardCompleted = runLinePhase(-35.0, false, 800U);
        if (leftBackwardCompleted == false)
        {
            stop();
            return false;
        }
        picarxObject->setDirectionServoAngle(0.0);
        picarxObject->stop();
        const agent::boolean centerSampleCompleted = sampleLineFor(200U);
        if (centerSampleCompleted == false)
        {
            stop();
            return false;
        }
        const agent::boolean rightForwardCompleted = runLinePhase(35.0, true, 800U);
        if (rightForwardCompleted == false)
        {
            stop();
            return false;
        }
        const agent::boolean rightBackwardCompleted = runLinePhase(35.0, false, 800U);
        if (rightBackwardCompleted == false)
        {
            stop();
            return false;
        }
        picarxObject->setDirectionServoAngle(0.0);
        picarxObject->stop();
        const agent::boolean finalSampleCompleted = sampleLineFor(200U);
        if (finalSampleCompleted == false)
        {
            stop();
            return false;
        }

        if ((resultValue.cliffReference[0U] < resultValue.lineReference[0U]) &&
            (resultValue.cliffReference[1U] < resultValue.lineReference[1U]) &&
            (resultValue.cliffReference[2U] < resultValue.lineReference[2U]))
        {
            for (agent::uint32 index = 0U; index < 3U; ++index)
            {
                resultValue.cliffReference[index] =
                    (resultValue.cliffReference[index] + resultValue.lineReference[index]) / 2;
            }
        }
        return true;
    }

    /**
     * @brief Averages ten stationary samples into pending cliff references.
     * @return `true` after completion or `false` after cancellation.
     */
    agent::boolean XWalkGrayscaleCalibration::calibrateCliff()
    {
        picarxObject->stop();
        hal::linetrackervalues totals{0, 0, 0};
        constexpr agent::int32 sampleCount{10};
        for (agent::int32 sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            const agent::boolean operationRequested = continueCallback(callbackContext);
            if (operationRequested == false)
            {
                stop();
                return false;
            }
            const hal::linetrackervalues sample = picarxObject->grayscaleData();
            for (agent::uint32 index = 0U; index < 3U; ++index)
            {
                totals[index] += sample[index];
            }
            const agent::boolean delayCompleted = wait(200U);
            if (delayCompleted == false)
            {
                stop();
                return false;
            }
        }
        hal::linetrackervalues averages{0, 0, 0};
        for (agent::uint32 index = 0U; index < 3U; ++index)
        {
            averages[index] = totals[index] / sampleCount;
        }
        if ((averages[0U] < lineMinimumValues[0U]) && (averages[1U] < lineMinimumValues[1U]) &&
            (averages[2U] < lineMinimumValues[2U]))
        {
            for (agent::uint32 index = 0U; index < 3U; ++index)
            {
                averages[index] = (averages[index] + lineMinimumValues[index]) / 2;
            }
        }
        resultValue.cliffReference = averages;
        return true;
    }

    /** @brief Persists both pending reference arrays through PiCar-X. */
    void XWalkGrayscaleCalibration::save()
    {
        picarxObject->setGrayscaleReference(resultValue.lineReference);
        picarxObject->setCliffReference(resultValue.cliffReference);
    }

    /**
     * @brief Returns pending values without persisting them.
     * @return Non-owning result reference valid for this object lifetime.
     */
    const XWalkGrayscaleCalibrationResult& XWalkGrayscaleCalibration::result() const noexcept
    {
        return resultValue;
    }

} /* namespace xwalk::agent */
