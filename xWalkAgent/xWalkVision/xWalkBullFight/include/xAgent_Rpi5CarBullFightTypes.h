/******************************************************************************
 * @file        xAgent_Rpi5CarBullFightTypes.h
 * @brief       Declares red-target pursuit configuration and results.
 * @project     xWalk Firmware
 * @module      xWalkBullFight
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_BULL_FIGHT_TYPES_H
#define XAGENT_RPI5CAR_BULL_FIGHT_TYPES_H

#include "xAgent_Rpi5CarComputerVisionTypes.h"

namespace xwalk::agent
{

/** @brief Identifies one red-target pursuit outcome. */
enum class XWalkBullFightState : agent::uint8
{
    /** @brief No red target was detected and forward power was set to zero. */
    Searching = 0U,
    /** @brief A red target updated the camera, steering, and forward motion. */
    Pursuing,
    /** @brief Cancellation interrupted the step. */
    Cancelled
};

/** @brief Stores source-compatible geometry, limits, movement, and timing. */
struct XWalkBullFightConfiguration
{
    agent::uint32 frameWidthPixels{640U};
    agent::uint32 frameHeightPixels{480U};
    agent::float64 correctionSpanDegrees{10.0};
    agent::float64 maximumCameraAngleDegrees{35.0};
    agent::float64 speedPercent{50.0};
    agent::uint32 sampleDelayMs{50U};
    agent::uint32 finalDelayMs{100U};
};

/** @brief Reports one observation and the resulting retained commands. */
struct XWalkBullFightResult
{
    XWalkBullFightState state{XWalkBullFightState::Searching};
    XWalkComputerVisionDetection target{};
    agent::float64 panAngleDegrees{};
    agent::float64 tiltAngleDegrees{};
    agent::float64 directionAngleDegrees{};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_BULL_FIGHT_TYPES_H */
