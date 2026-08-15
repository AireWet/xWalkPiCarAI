/******************************************************************************
 * @file        xAgent_Rpi5CarFaceTrackingTypes.h
 * @brief       Declares face-tracking configuration and step results.
 *
 * @details
 * Defines the bounded geometry, timing, and observable state used to port
 * `example/8.stare_at_you.py` through the shared computer-vision provider.
 *
 * @project     xWalk Firmware
 * @module      xWalkFaceTracking
 *
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_FACE_TRACKING_TYPES_H
#define XAGENT_RPI5CAR_FACE_TRACKING_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarComputerVisionTypes.h"

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

    /**
     * @enum XWalkFaceTrackingState
     * @brief Identifies one face-tracking step outcome.
     */
    enum class XWalkFaceTrackingState : agent::uint8
    {
        /** @brief No face was available and the retained camera pose was unchanged. */
        Searching = 0U,
        /** @brief One selected face updated both camera-servo commands. */
        Tracking,
        /** @brief Cancellation prevented or interrupted the step. */
        Cancelled
    };

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkFaceTrackingConfiguration
     * @brief Stores source-compatible frame geometry, servo limits, and timing.
     */
    struct XWalkFaceTrackingConfiguration
    {
            /** @brief Source frame width in pixels from 16 through 7680. */
            agent::uint32 frameWidthPixels{640U};
            /** @brief Source frame height in pixels from 16 through 4320. */
            agent::uint32 frameHeightPixels{480U};
            /** @brief Full correction span applied across one frame, in degrees. */
            agent::float64 correctionSpanDegrees{10.0};
            /** @brief Absolute retained pan and tilt limit, in degrees. */
            agent::float64 maximumAngleDegrees{35.0};
            /** @brief Delay after every sample, in milliseconds. */
            agent::uint32 sampleDelayMs{50U};
            /** @brief Final delay after motor stop, in milliseconds. */
            agent::uint32 finalDelayMs{100U};
    };

    /**
     * @struct XWalkFaceTrackingResult
     * @brief Reports one observation and the resulting retained camera pose.
     */
    struct XWalkFaceTrackingResult
    {
            /** @brief Step decision after sampling and optional servo movement. */
            XWalkFaceTrackingState state{XWalkFaceTrackingState::Searching};
            /** @brief Face observation used by the step. */
            XWalkComputerVisionDetection face{};
            /** @brief Retained logical camera-pan command in degrees. */
            agent::float64 panAngleDegrees{};
            /** @brief Retained logical camera-tilt command in degrees. */
            agent::float64 tiltAngleDegrees{};
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_FACE_TRACKING_TYPES_H */
