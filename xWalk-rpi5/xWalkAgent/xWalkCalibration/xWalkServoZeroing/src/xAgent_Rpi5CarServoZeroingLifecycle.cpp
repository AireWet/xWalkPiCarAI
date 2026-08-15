/******************************************************************************
 * @file        xAgent_Rpi5CarServoZeroingLifecycle.cpp
 * @brief       Implements servo-zeroing construction, validation, and timing.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarServoZeroing.h"

#include "xHal_Rpi5CarCommonFunctions.h"

#include "xHal_Rpi5CarTrace.h"
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
     * @brief Stores caller-owned servo and scheduling boundaries.
     * @param[in,out] context Nullable context that outlives this coordinator.
     * @param[in] backendCallbacks Complete synchronous callback table.
     * @param[in] configuration Source-compatible angles and timing values.
     */
    XWalkServoZeroing::XWalkServoZeroing(agent::contextpointer context,
                                         const XWalkServoZeroingCallbacks& backendCallbacks,
                                         const XWalkServoZeroingConfiguration& configuration)
        : callbackContext(context), cancellationContext(context), callbacks(backendCallbacks),
          configurationValue(configuration)
    {
        validate(callbacks, configurationValue);
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Validates callbacks, finite angles, and bounded delays.
     * @param[in] backendCallbacks Callback table to validate.
     * @param[in] configuration Numeric settings to validate.
     * @throws std::invalid_argument If callbacks or angles are invalid.
     * @throws std::out_of_range If a timing value is zero or unreasonably large.
     */
    void XWalkServoZeroing::validate(const XWalkServoZeroingCallbacks& backendCallbacks,
                                     const XWalkServoZeroingConfiguration& configuration)
    {
        if ((backendCallbacks.setAngle == nullptr) || (backendCallbacks.delay == nullptr) ||
            (backendCallbacks.continueOperation == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Servo-zeroing callbacks must be complete");
        }
        const agent::boolean configurationInvalid = static_cast<agent::boolean>(
            !XHAL_IS_FINITE(configuration.pulseAngleDegrees) || !XHAL_IS_FINITE(configuration.zeroAngleDegrees));
        if (configurationInvalid)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Servo-zeroing angles must be finite");
        }
        if ((configuration.pulseAngleDegrees < -90.0) || (configuration.pulseAngleDegrees > 90.0) ||
            (configuration.zeroAngleDegrees < -90.0) || (configuration.zeroAngleDegrees > 90.0) ||
            (configuration.commandDelayMs == 0U) || (configuration.commandDelayMs > 10'000U) ||
            (configuration.idleDelayMs == 0U) || (configuration.idleDelayMs > 10'000U))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "Servo-zeroing configuration is out of range");
        }
    }

    /**
     * @brief Performs one cancellable delay in at most 20-millisecond slices.
     * @param[in] durationMs Requested total delay in milliseconds.
     * @return `true` after the complete interval; otherwise `false` after
     * cancellation.
     */
    agent::boolean XWalkServoZeroing::wait(agent::uint32 durationMs) const
    {
        constexpr agent::uint32 cancellationIntervalMs{20U};
        agent::uint32 remainingMs = durationMs;
        while (remainingMs > 0U)
        {
            const agent::boolean operationRequested = callbacks.continueOperation(cancellationContext);
            if (operationRequested == false)
            {
                return false;
            }
            const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ? remainingMs : cancellationIntervalMs;
            callbacks.delay(callbackContext, sliceMs);
            remainingMs -= sliceMs;
        }
        return true;
    }

    /**
     * @brief Rebinds cancellation without changing the servo hardware context.
     * @param[in,out] context Nullable context that outlives the next `run` call.
     * @param[in] continueOperation Non-null cancellation query.
     * @throws std::invalid_argument If `continueOperation` is null.
     */
    void XWalkServoZeroing::setCancellation(agent::contextpointer context,
                                            servozeroingcontinuecallback continueOperation)
    {
        if (continueOperation == nullptr)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Servo-zeroing cancellation callback is required");
        }
        cancellationContext = context;
        callbacks.continueOperation = continueOperation;
    }

} /* namespace xwalk::agent */
