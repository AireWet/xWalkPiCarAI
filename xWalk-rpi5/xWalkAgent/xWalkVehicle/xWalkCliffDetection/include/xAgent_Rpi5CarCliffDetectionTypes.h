/******************************************************************************
 * @file        xAgent_Rpi5CarCliffDetectionTypes.h
 * @brief       Declares cliff-detection scheduling and result types.
 *
 * @details
 * Defines the injected timing boundary and observable state-machine outcome
 * used by the cliff-detection Agent.
 *
 * @project     xWalk Firmware
 * @module      xWalkCliffDetection
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

#ifndef XAGENT_RPI5CAR_CLIFF_DETECTION_TYPES_H
#define XAGENT_RPI5CAR_CLIFF_DETECTION_TYPES_H

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

    /** @brief Identifies the outcome of one cliff-detection sample and action. */
    enum class XWalkCliffDetectionResult : agent::uint8
    {
        /** @brief No channel reports a cliff and both motors were stopped. */
        Safe,
        /** @brief At least one channel reports a cliff and reverse motion was commanded. */
        Danger,
        /** @brief Cancellation prevented or interrupted the requested step. */
        Cancelled
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Suspends cliff-detection execution for one bounded interval.
     * @param[in,out] context Non-owning context that outlives the Agent.
     * @param[in] durationMs Requested delay in milliseconds.
     */
    using cliffdetectiondelaycallback = void (*)(agent::contextpointer context, agent::uint32 durationMs);

    /**
     * @brief Reports whether cliff-detection execution may continue.
     * @param[in,out] context Non-owning context that outlives the Agent.
     * @return `true` to continue or `false` to request motor cleanup.
     */
    using cliffdetectioncontinuecallback = agent::boolean (*)(agent::contextpointer context);

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_CLIFF_DETECTION_TYPES_H */
