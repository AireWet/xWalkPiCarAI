/******************************************************************************
 * @file        xAgent_Rpi5CarLineTracking.cpp
 * @brief       Implements bounded PiCar-X line-following decisions.
 *
 * @details
 * Preserves the example's classification priority, steering signs, speeds,
 * and last-direction recovery while bounding line-recovery sensor acquisition.
 *
 * @project     xWalk Firmware
 * @module      xWalkLineTracking
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
#include "xAgent_Rpi5CarLineTracking.h"

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

    /**
     * @brief Applies one non-stop line-following movement decision.
     *
     * @param[in] state
     * Forward, left, or right decision to apply.
     */
    void XWalkLineTracking::applyTrackingState(XWalkLineTrackingState state)
    {
        if (state == XWalkLineTrackingState::Forward)
        {
            picarxObject->setDirectionServoAngle(0.0);
            picarxObject->forward(configurationValue.powerPercent);
        }
        else if (state == XWalkLineTrackingState::Left)
        {
            picarxObject->setDirectionServoAngle(configurationValue.steeringOffsetDegrees);
            picarxObject->forward(configurationValue.powerPercent);
        }
        else if (state == XWalkLineTrackingState::Right)
        {
            picarxObject->setDirectionServoAngle(-configurationValue.steeringOffsetDegrees);
            picarxObject->forward(configurationValue.powerPercent);
        }
        else
        {
            picarxObject->stop();
        }
    }

    /**
     * @brief Performs one bounded line-lost recovery attempt.
     *
     * @param[in] initialReadings
     * All-background sample that triggered recovery.
     *
     * @return
     * Last recovery sample, classified state, and timeout outcome.
     */
    XWalkLineTrackingResult XWalkLineTracking::recoverLine(const hal::linetrackervalues& initialReadings)
    {
        XWalkLineTrackingResult result{initialReadings, XWalkLineTrackingState::Stop, true, false};

        if (lastStateValue == XWalkLineTrackingState::Left)
        {
            picarxObject->setDirectionServoAngle(-configurationValue.recoverySteeringDegrees);
            picarxObject->backward(configurationValue.recoveryPowerPercent);
        }
        else if (lastStateValue == XWalkLineTrackingState::Right)
        {
            picarxObject->setDirectionServoAngle(configurationValue.recoverySteeringDegrees);
            picarxObject->backward(configurationValue.recoveryPowerPercent);
        }
        else
        {
            picarxObject->stop();
        }

        for (agent::uint32 sampleIndex = 0U; sampleIndex < configurationValue.maximumRecoverySamples; ++sampleIndex)
        {
            result.readings = picarxObject->grayscaleData();
            result.state = classify(picarxObject->lineStatus(result.readings));
            currentStateValue = result.state;
            if (result.state != XWalkLineTrackingState::Stop)
            {
                delay(configurationValue.recoveryCompletionDelayMs);
                return result;
            }
        }

        picarxObject->stop();
        result.recoveryTimedOut = true;
        delay(configurationValue.recoveryCompletionDelayMs);
        return result;
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Classifies a grayscale status using upstream decision priority.
     *
     * @param[in] status
     * Left, middle, and right values where one identifies the line.
     *
     * @return
     * Stop for all zeroes, then forward, right, or left by upstream priority.
     */
    XWalkLineTrackingState XWalkLineTracking::classify(const hal::linetrackerstatus& status) noexcept
    {
        if ((status[0U] == 0U) && (status[1U] == 0U) && (status[2U] == 0U))
        {
            return XWalkLineTrackingState::Stop;
        }
        if (status[1U] == 1U)
        {
            return XWalkLineTrackingState::Forward;
        }
        if (status[0U] == 1U)
        {
            return XWalkLineTrackingState::Right;
        }
        return XWalkLineTrackingState::Left;
    }

    /**
     * @brief Acquires sensors and performs one bounded line-following iteration.
     *
     * @return
     * Final readings, classified state, and recovery outcome.
     */
    XWalkLineTrackingResult XWalkLineTracking::step()
    {
        const hal::linetrackervalues readings = picarxObject->grayscaleData();
        currentStateValue = classify(picarxObject->lineStatus(readings));
        if (currentStateValue == XWalkLineTrackingState::Stop)
        {
            return recoverLine(readings);
        }

        lastStateValue = currentStateValue;
        applyTrackingState(currentStateValue);
        return {readings, currentStateValue, false, false};
    }

} /* namespace xwalk::agent */
