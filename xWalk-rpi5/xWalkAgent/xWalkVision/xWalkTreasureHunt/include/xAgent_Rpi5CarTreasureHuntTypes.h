/******************************************************************************
 * @file        xAgent_Rpi5CarTreasureHuntTypes.h
 * @brief       Declares treasure-hunt callbacks, configuration, and results.
 *
 * @details
 * Defines the hardware-independent boundary used to port
 * `example/20.treasure_hunt.py` without owning camera or platform randomness.
 *
 * @project     xWalk Firmware
 * @module      xWalkTreasureHunt
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

#ifndef XAGENT_RPI5CAR_TREASURE_HUNT_TYPES_H
#define XAGENT_RPI5CAR_TREASURE_HUNT_TYPES_H

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
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Selects one source-compatible treasure color.
     * @param[in,out] context Nullable non-owning context that outlives callback use.
     * @return Red, orange, yellow, green, blue, or purple.
     */
    using treasurehuntselectcolorcallback = XWalkComputerVisionColor (*)(agent::contextpointer context);

    /******************************************************************************
     * Enumeration declarations
     ******************************************************************************/

    /** @brief Identifies the operator action completed by one treasure-hunt step. */
    enum class XWalkTreasureHuntAction : agent::uint8
    {
        /** @brief No keyboard action was recognized. */
        Ignored = 0U,
        /** @brief The car completed one bounded movement. */
        Moved,
        /** @brief The current target prompt was spoken again. */
        TargetRepeated,
        /** @brief The operator requested normal termination. */
        Quit,
        /** @brief Cancellation interrupted the current step. */
        Cancelled
    };

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Groups the vision provider and random target-selection boundary. */
    struct XWalkTreasureHuntCallbacks
    {
            /** @brief Complete caller-owned camera and scheduling callback table. */
            XWalkComputerVisionCallbacks vision{};
            /** @brief Non-null source-compatible color-selection operation. */
            treasurehuntselectcolorcallback selectColor{nullptr};
    };

    /** @brief Stores source-compatible detection, movement, and timing settings. */
    struct XWalkTreasureHuntConfiguration
    {
            /** @brief Minimum accepted color-blob width in pixels. */
            agent::uint32 detectionWidthThresholdPixels{100U};
            /** @brief Requested forward or backward motor power in percent. */
            agent::float64 driveSpeedPercent{80.0};
            /** @brief Signed steering magnitude used for left and right movement. */
            agent::float64 turnAngleDegrees{30.0};
            /** @brief Camera warm-up interval in milliseconds. */
            agent::uint32 startupDelayMs{800U};
            /** @brief Delay after the game-start or success prompt in milliseconds. */
            agent::uint32 promptDelayMs{100U};
            /** @brief Duration of each movement command in milliseconds. */
            agent::uint32 movementDelayMs{500U};
            /** @brief Delay after one ordinary loop iteration in milliseconds. */
            agent::uint32 loopDelayMs{50U};
            /** @brief Final delay after the goodbye prompt in milliseconds. */
            agent::uint32 finalDelayMs{200U};
    };

    /** @brief Reports one observation, target change, and keyboard action. */
    struct XWalkTreasureHuntResult
    {
            /** @brief Operator action completed during the step. */
            XWalkTreasureHuntAction action{XWalkTreasureHuntAction::Ignored};
            /** @brief True when a sufficiently wide target was detected. */
            agent::boolean targetFound{};
            /** @brief Active target after any successful detection and reselection. */
            XWalkComputerVisionColor targetColor{XWalkComputerVisionColor::Red};
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_TREASURE_HUNT_TYPES_H */
