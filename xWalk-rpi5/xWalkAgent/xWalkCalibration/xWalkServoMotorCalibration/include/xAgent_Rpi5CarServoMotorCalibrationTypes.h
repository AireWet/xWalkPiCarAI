/******************************************************************************
 * @file        xAgent_Rpi5CarServoMotorCalibrationTypes.h
 * @brief       Declares servo/motor calibration callbacks and pending values.
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

#ifndef XAGENT_RPI5CAR_SERVO_MOTOR_CALIBRATION_TYPES_H
#define XAGENT_RPI5CAR_SERVO_MOTOR_CALIBRATION_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Suspends calibration for one bounded interval.
     * @param[in,out] context Non-owning application context valid for the call duration.
     * @param[in] durationMs Requested delay in milliseconds.
     */
    using servomotorcalibrationdelaycallback = void (*)(agent::contextpointer context, agent::uint32 durationMs);

    /**
     * @brief Reports whether the active calibration may continue.
     * @param[in,out] context Non-owning application context valid for the call duration.
     * @return `true` to continue or `false` to stop all motors.
     */
    using servomotorcalibrationcontinuecallback = agent::boolean (*)(agent::contextpointer context);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Pending servo offsets and motor directions retained until save. */
    struct XWalkServoMotorCalibrationResult
    {
            /** @brief Steering, camera-pan, and camera-tilt offsets in degrees. */
            agent::fixedarray<agent::float64, 3U> servoOffsets{};
            /** @brief Left and right motor directions containing only 1 or -1. */
            agent::fixedarray<agent::int32, 2U> motorDirections{1, 1};
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_SERVO_MOTOR_CALIBRATION_TYPES_H */
