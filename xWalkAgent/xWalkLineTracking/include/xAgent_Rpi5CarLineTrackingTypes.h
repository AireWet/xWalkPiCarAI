/******************************************************************************
 * @file        xAgent_Rpi5CarLineTrackingTypes.h
 * @brief       Declares PiCar-X line-tracking configuration and result types.
 *
 * @details
 * Defines the control states, bounded recovery settings, timing callback, and
 * observable result used by the line-tracking agent coordinator.
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

#ifndef XAGENT_RPI5CAR_LINE_TRACKING_TYPES_H
#define XAGENT_RPI5CAR_LINE_TRACKING_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "xHal_Rpi5CarLineTrackerTypes.h"

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
 * Constants
 ******************************************************************************/

/** @brief Largest supported sample count for one recovery attempt. */
inline constexpr hal::uint32 XAGENT_RPI5CAR_LINE_TRACKING_MAX_RECOVERY_SAMPLES = 100'000U;
/** @brief Largest supported recovery-completion delay, in milliseconds. */
inline constexpr hal::uint32 XAGENT_RPI5CAR_LINE_TRACKING_MAX_RECOVERY_DELAY_MS = 1'000U;

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/**
 * @enum XWalkLineTrackingState
 * @brief Identifies one line-following movement decision.
 */
enum class XWalkLineTrackingState : hal::uint8
{
    /**
     * @brief Indicates that no grayscale channel currently detects the line.
     */
    Stop,
    /**
     * @brief Selects centered steering and forward movement.
     */
    Forward,
    /**
     * @brief Selects positive steering to follow a line on the right sensor.
     */
    Left,
    /**
     * @brief Selects negative steering to follow a line on the left sensor.
     */
    Right
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Suspends line-recovery execution for a bounded interval.
 *
 * @param[in,out] context
 * Non-owning application context that must outlive the coordinator.
 *
 * @param[in] durationMs
 * Requested delay in milliseconds.
 */
using linetrackingdelaycallback = void (*)(hal::contextpointer context, hal::uint32 durationMs);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @struct XWalkLineTrackingConfiguration
 * @brief Contains movement and bounded recovery settings.
 */
struct XWalkLineTrackingConfiguration
{
    /**
     * @brief Forward tracking power from zero through one hundred percent.
     */
    hal::float64 powerPercent{10.0};
    /**
     * @brief Absolute tracking steering offset from zero through thirty degrees.
     */
    hal::float64 steeringOffsetDegrees{20.0};
    /**
     * @brief Absolute reverse-recovery steering angle from zero through thirty degrees.
     */
    hal::float64 recoverySteeringDegrees{30.0};
    /**
     * @brief Reverse-recovery power from zero through one hundred percent.
     */
    hal::float64 recoveryPowerPercent{10.0};
    /**
     * @brief Recovery sample bound from one through 100,000 samples.
     */
    hal::uint32 maximumRecoverySamples{1'000U};
    /**
     * @brief Delay after recovery from zero through 1,000 milliseconds.
     */
    hal::uint32 recoveryCompletionDelayMs{1U};
};

/**
 * @struct XWalkLineTrackingResult
 * @brief Reports the final sample and decision from one controller step.
 */
struct XWalkLineTrackingResult
{
    /**
     * @brief Final left, middle, and right grayscale values in ADC counts.
     */
    hal::linetrackervalues readings{};
    /**
     * @brief State classified from `readings`.
     */
    XWalkLineTrackingState state{XWalkLineTrackingState::Stop};
    /**
     * @brief `true` when the initial state entered line-lost recovery.
     */
    hal::boolean recoveryAttempted{};
    /**
     * @brief `true` when recovery reached its configured sample bound.
     */
    hal::boolean recoveryTimedOut{};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_LINE_TRACKING_TYPES_H */
