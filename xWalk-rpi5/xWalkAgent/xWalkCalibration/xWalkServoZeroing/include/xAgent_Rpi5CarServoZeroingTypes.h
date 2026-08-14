/******************************************************************************
 * @file        xAgent_Rpi5CarServoZeroingTypes.h
 * @brief       Declares servo-zeroing callbacks and configuration.
 *
 * @details
 * Defines the hardware-independent boundary used to port
 * `example/servo_zeroing.py` without owning PWM or scheduling resources.
 *
 * @project     xWalk Firmware
 * @module      xWalkServoZeroing
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

#ifndef XAGENT_RPI5CAR_SERVO_ZEROING_TYPES_H
#define XAGENT_RPI5CAR_SERVO_ZEROING_TYPES_H

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
 * Constants
 ******************************************************************************/

/** @brief Number of source Robot HAT servo channels. */
constexpr agent::uint8 XAGENT_RPI5CAR_SERVO_ZEROING_CHANNEL_COUNT = 12U;

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Commands one logical Robot HAT servo angle.
 * @param[in,out] context Nullable non-owning context that outlives callback use.
 * @param[in] servoId Servo channel from zero through eleven.
 * @param[in] angleDegrees Logical angle in degrees.
 */
using servozeroingsetanglecallback = void (*)(agent::contextpointer context,
    agent::uint8 servoId, agent::float64 angleDegrees);

/**
 * @brief Suspends one source-compatible timing interval.
 * @param[in,out] context Nullable non-owning context that outlives callback use.
 * @param[in] durationMs Requested delay in milliseconds.
 */
using servozeroingdelaycallback = void (*)(agent::contextpointer context,
    agent::uint32 durationMs);

/**
 * @brief Reports whether the foreground example may continue.
 * @param[in,out] context Nullable non-owning context that outlives callback use.
 * @return `true` to continue; otherwise `false` to stop.
 */
using servozeroingcontinuecallback = agent::boolean (*)(agent::contextpointer context);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Groups complete synchronous servo and scheduling operations. */
struct XWalkServoZeroingCallbacks
{
    /** @brief Non-null logical servo-angle operation. */
    servozeroingsetanglecallback setAngle{nullptr};
    /** @brief Non-null delay operation. */
    servozeroingdelaycallback delay{nullptr};
    /** @brief Non-null cancellation query. */
    servozeroingcontinuecallback continueOperation{nullptr};
};

/** @brief Stores source-compatible angles and timing intervals. */
struct XWalkServoZeroingConfiguration
{
    /** @brief Initial pulse angle in degrees. */
    agent::float64 pulseAngleDegrees{10.0};
    /** @brief Final zero angle in degrees. */
    agent::float64 zeroAngleDegrees{0.0};
    /** @brief Delay following every angle command in milliseconds. */
    agent::uint32 commandDelayMs{100U};
    /** @brief Source idle-loop interval in milliseconds. */
    agent::uint32 idleDelayMs{1'000U};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_SERVO_ZEROING_TYPES_H */
