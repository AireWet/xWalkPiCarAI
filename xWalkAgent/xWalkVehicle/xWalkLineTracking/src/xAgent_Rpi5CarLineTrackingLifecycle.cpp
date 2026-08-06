/******************************************************************************
 * @file        xAgent_Rpi5CarLineTrackingLifecycle.cpp
 * @brief       Implements line-tracking lifecycle and configuration validation.
 *
 * @details
 * Binds caller-owned PiCar-X and timing dependencies and validates every
 * movement and bounded recovery setting before use.
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

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "xAgent_Rpi5CarLineTracking.h"
#include "xHal_Rpi5CarExceptions.h"

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
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a line-tracking coordinator around one PiCar-X object.
 *
 * @param[in] picarx
 * PiCar-X coordinator that must outlive this object.
 *
 * @param[in,out] context
 * Optional timing context that must outlive this object.
 *
 * @param[in] callback
 * Non-null synchronous delay operation.
 *
 * @param[in] configuration
 * Movement and bounded recovery settings copied into this object.
 *
 * @throws std::invalid_argument
 * If a floating-point setting is not finite or `callback` is null.
 *
 * @throws std::out_of_range
 * If a speed, angle, or recovery-sample setting is outside its range.
 */
XWalkLineTracking::XWalkLineTracking(XWalkPicarx& picarx, agent::contextpointer context,
    linetrackingdelaycallback callback, const XWalkLineTrackingConfiguration& configuration)
    : picarxObject(&picarx), callbackContext(context), delayCallback(callback),
      configurationValue(configuration)
{
    validateConfiguration(configurationValue, delayCallback);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Stops the drive motors without releasing the PiCar-X dependency.
 *
 * @warning
 * The injected motor backend must not throw during destruction.
 */
XWalkLineTracking::~XWalkLineTracking()
{
    stop();
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates the complete movement, recovery, and callback contract.
 *
 * @param[in] configuration
 * Configuration whose speeds, angles, and sample bound are validated.
 *
 * @param[in] callback
 * Delay callback that must not be null.
 *
 * @throws std::invalid_argument
 * If a floating-point setting is not finite or `callback` is null.
 *
 * @throws std::out_of_range
 * If a speed, angle, or recovery-sample setting is outside its range.
 */
void XWalkLineTracking::validateConfiguration(const XWalkLineTrackingConfiguration& configuration,
    linetrackingdelaycallback callback)
{
    if (callback == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Line-tracking delay callback must not be null");
    }
    const agent::boolean configurationInvalid =
        static_cast<agent::boolean>(
            (!XHAL_IS_FINITE(configuration.powerPercent)) ||
        (!XHAL_IS_FINITE(configuration.steeringOffsetDegrees)) ||
        (!XHAL_IS_FINITE(configuration.recoverySteeringDegrees)) ||
        (!XHAL_IS_FINITE(configuration.recoveryPowerPercent)));
    if (configurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Line-tracking speeds and angles must be finite");
    }
    if ((configuration.powerPercent < 0.0) || (configuration.powerPercent > 100.0) ||
        (configuration.recoveryPowerPercent < 0.0) ||
        (configuration.recoveryPowerPercent > 100.0))
    {
        XHAL_THROW_OUT_OF_RANGE("Line-tracking power must be from 0 through 100 percent");
    }
    if ((configuration.steeringOffsetDegrees < 0.0) ||
        (configuration.steeringOffsetDegrees > 30.0) ||
        (configuration.recoverySteeringDegrees < 0.0) ||
        (configuration.recoverySteeringDegrees > 30.0))
    {
        XHAL_THROW_OUT_OF_RANGE("Line-tracking steering angles must be from 0 through 30 degrees");
    }
    if ((configuration.maximumRecoverySamples == 0U) ||
        (configuration.maximumRecoverySamples > XAGENT_RPI5CAR_LINE_TRACKING_MAX_RECOVERY_SAMPLES))
    {
        XHAL_THROW_OUT_OF_RANGE("Line-tracking recovery samples must be from 1 through 100000");
    }
    if (configuration.recoveryCompletionDelayMs >
        XAGENT_RPI5CAR_LINE_TRACKING_MAX_RECOVERY_DELAY_MS)
    {
        XHAL_THROW_OUT_OF_RANGE("Line-tracking recovery delay must not exceed 1000 milliseconds");
    }
}

/**
 * @brief Invokes the application-owned delay operation.
 *
 * @param[in] durationMs
 * Requested delay in milliseconds.
 */
void XWalkLineTracking::delay(agent::uint32 durationMs) const
{
    delayCallback(callbackContext, durationMs);
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Stops the drive motors and resets both retained states.
 *
 * @post
 * `currentState()` and `lastState()` return Stop.
 */
void XWalkLineTracking::stop()
{
    picarxObject->stop();
    currentStateValue = XWalkLineTrackingState::Stop;
    lastStateValue = XWalkLineTrackingState::Stop;
}

/**
 * @brief Stops, resets retained state, and applies the example's final delay.
 *
 * @post
 * `currentState()` and `lastState()` return Stop and the delay callback has
 * received 100 milliseconds.
 */
void XWalkLineTracking::finish()
{
    stop();
    delay(XAGENT_RPI5CAR_LINE_TRACKING_FINAL_DELAY_MS);
}

/**
 * @brief Returns the most recently classified state.
 *
 * @return
 * Current line-tracking state.
 */
XWalkLineTrackingState XWalkLineTracking::currentState() const noexcept
{
    return currentStateValue;
}

/**
 * @brief Returns the most recent non-stop tracking state.
 *
 * @return
 * Direction retained for the next line-lost recovery attempt.
 */
XWalkLineTrackingState XWalkLineTracking::lastState() const noexcept
{
    return lastStateValue;
}

} /* namespace xwalk::agent */
