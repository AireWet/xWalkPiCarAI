/******************************************************************************
 * @file        xAgent_Rpi5CarVideoCarTypes.h
 * @brief       Declares interactive video-car configuration and results.
 * @project     xWalk Firmware
 * @module      xWalkVideoCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VIDEO_CAR_TYPES_H
#define XAGENT_RPI5CAR_VIDEO_CAR_TYPES_H

#include "xAgent_Rpi5CarComputerVisionTypes.h"

namespace xwalk::agent
{

    /** @brief Identifies the retained vehicle motion selected by the operator. */
    enum class XWalkVideoCarMotion : agent::uint8
    {
        Stop = 0U,
        Forward,
        Backward,
        TurnLeft,
        TurnRight
    };

    /** @brief Identifies the outcome of one interactive key. */
    enum class XWalkVideoCarEvent : agent::uint8
    {
        Ignored = 0U,
        MotionChanged,
        SpeedChanged,
        PhotoCaptured,
        Cancelled
    };

    /** @brief Stores source-compatible speed, steering, and timing settings. */
    struct XWalkVideoCarConfiguration
    {
            agent::uint32 speedStepPercent{10U};
            agent::uint32 maximumSpeedPercent{100U};
            agent::uint32 directionChangeCapPercent{60U};
            agent::float64 steeringAngleDegrees{30.0};
            agent::uint32 startupDelayMs{2'000U};
            agent::uint32 keyDelayMs{100U};
    };

    /** @brief Reports one key result and the retained control state. */
    struct XWalkVideoCarResult
    {
            XWalkVideoCarEvent event{XWalkVideoCarEvent::Ignored};
            XWalkVideoCarMotion motion{XWalkVideoCarMotion::Stop};
            agent::uint32 speedPercent{};
            agent::string photoPath{};
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VIDEO_CAR_TYPES_H */
