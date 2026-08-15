/******************************************************************************
 * @file        xAgent_Rpi5CarObstacleAvoidanceTypes.h
 * @brief       Declares obstacle-avoidance scheduling and result types.
 *
 * @details
 * Defines the injected timing boundary and observable decision outcome used by
 * the obstacle-avoidance Agent.
 *
 * @project     xWalk Firmware
 * @module      xWalkObstacleAvoidance
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

#ifndef XAGENT_RPI5CAR_OBSTACLE_AVOIDANCE_TYPES_H
#define XAGENT_RPI5CAR_OBSTACLE_AVOIDANCE_TYPES_H

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
     * Enumeration declarations
     ******************************************************************************/

    /** @brief Identifies the decision made for one ultrasonic distance sample. */
    enum class XWalkObstacleAvoidanceResult : agent::uint8
    {
        /** @brief The path is at least 40 centimeters and straight motion was commanded. */
        Forward,
        /** @brief The path is 20 through less than 40 centimeters and a right turn was commanded. */
        TurnRight,
        /** @brief The path is positive and less than 20 centimeters and reverse-left was commanded. */
        ReverseLeft,
        /** @brief The supplied ultrasonic sample is non-finite, zero, or negative. */
        SensorInvalid,
        /** @brief Cancellation prevented or interrupted the requested movement step. */
        Cancelled
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Suspends obstacle-avoidance execution for one bounded interval.
     * @param[in,out] context Non-owning context that outlives the Agent.
     * @param[in] durationMs Requested delay in milliseconds.
     */
    using obstacleavoidancedelaycallback = void (*)(agent::contextpointer context, agent::uint32 durationMs);

    /**
     * @brief Reports whether obstacle-avoidance execution may continue.
     * @param[in,out] context Non-owning context that outlives the Agent.
     * @return `true` to continue or `false` to request motor cleanup.
     */
    using obstacleavoidancecontinuecallback = agent::boolean (*)(agent::contextpointer context);

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_OBSTACLE_AVOIDANCE_TYPES_H */
