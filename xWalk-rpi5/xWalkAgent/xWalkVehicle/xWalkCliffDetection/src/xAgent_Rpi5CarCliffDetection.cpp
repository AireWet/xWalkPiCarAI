/******************************************************************************
 * @file        xAgent_Rpi5CarCliffDetection.cpp
 * @brief       Implements the grayscale cliff-response state machine.
 *
 * @details
 * Acquires grayscale samples, applies calibrated cliff classification, and
 * preserves transition-only reverse delay behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkCliffDetection
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarCliffDetection.h"

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
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Acquires grayscale data and applies one source-compatible response.
     * @return Safe, danger, or cancelled step result.
     * @warning Danger commands physical reverse movement at 80-percent requested speed.
     */
    XWalkCliffDetectionResult XWalkCliffDetection::step()
    {
        const agent::boolean operationRequested = continueCallback(callbackContext);
        if (operationRequested == false)
        {
            picarxObject->stop();
            return XWalkCliffDetectionResult::Cancelled;
        }

        const hal::linetrackervalues grayscaleValues = picarxObject->grayscaleData();
        const agent::boolean cliffDetected = picarxObject->cliffStatus(grayscaleValues);
        if (cliffDetected == false)
        {
            picarxObject->stop();
            lastDangerValue = false;
            return XWalkCliffDetectionResult::Safe;
        }

        picarxObject->backward(80.0);
        if (lastDangerValue == false)
        {
            const agent::boolean delayCompleted = wait(100U);
            if (delayCompleted == false)
            {
                picarxObject->stop();
                return XWalkCliffDetectionResult::Cancelled;
            }
        }
        lastDangerValue = true;
        return XWalkCliffDetectionResult::Danger;
    }

    /**
     * @brief Stops both motors and resets the retained state to safe.
     * @post Both motor commands are zero and `lastDanger()` returns false.
     */
    void XWalkCliffDetection::stop()
    {
        picarxObject->stop();
        lastDangerValue = false;
    }

    /**
     * @brief Reports whether the preceding completed sample selected danger.
     * @return `true` for danger or `false` for safe/reset state.
     */
    agent::boolean XWalkCliffDetection::lastDanger() const noexcept
    {
        return lastDangerValue;
    }

} /* namespace xwalk::agent */
