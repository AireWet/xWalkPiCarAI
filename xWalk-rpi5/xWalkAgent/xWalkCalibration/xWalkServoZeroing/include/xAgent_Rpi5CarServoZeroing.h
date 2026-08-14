/******************************************************************************
 * @file        xAgent_Rpi5CarServoZeroing.h
 * @brief       Declares the twelve-channel servo-zeroing coordinator.
 *
 * @details
 * Ports `example/servo_zeroing.py` through caller-owned servo and scheduling
 * callbacks while preserving bounded cancellation responsiveness.
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

#ifndef XAGENT_RPI5CAR_SERVO_ZEROING_H
#define XAGENT_RPI5CAR_SERVO_ZEROING_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarServoZeroingTypes.h"

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
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkServoZeroing
 * @brief Pulses and zeros all twelve Robot HAT servo channels in order.
 *
 * @details
 * Owns only copied configuration and callbacks. The composition root owns all
 * PWM, Servo, MCU-reset, timing, and cancellation resources.
 */
class XWalkServoZeroing final
{
    private:
        /** @brief Nullable non-owning callback context retained for synchronous use. */
        agent::contextpointer callbackContext{nullptr};
        /** @brief Nullable non-owning context used only by cancellation queries. */
        agent::contextpointer cancellationContext{nullptr};
        /** @brief Complete callback table copied during construction. */
        XWalkServoZeroingCallbacks callbacks{};
        /** @brief Source-compatible settings copied during construction. */
        XWalkServoZeroingConfiguration configurationValue{};

    protected:
        /** @brief Validates callbacks, finite angles, and bounded delays. */
        static void validate(const XWalkServoZeroingCallbacks& backendCallbacks,
            const XWalkServoZeroingConfiguration& configuration);
        /** @brief Performs one cancellable delay in at most 20-millisecond slices. */
        agent::boolean wait(agent::uint32 durationMs) const;

    public:
        /**
         * @brief Stores caller-owned servo and scheduling boundaries.
         * @param[in,out] context Nullable context that outlives this coordinator.
         * @param[in] backendCallbacks Complete synchronous callback table.
         * @param[in] configuration Source-compatible angles and timing values.
         */
        XWalkServoZeroing(agent::contextpointer context,
            const XWalkServoZeroingCallbacks& backendCallbacks,
            const XWalkServoZeroingConfiguration& configuration = {});

        /** @brief Destroys copied state without issuing additional servo commands. */
        ~XWalkServoZeroing() = default;

        XWalkServoZeroing(const XWalkServoZeroing&) = delete;
        XWalkServoZeroing(XWalkServoZeroing&&) = delete;
        XWalkServoZeroing& operator=(const XWalkServoZeroing&) = delete;
        XWalkServoZeroing& operator=(XWalkServoZeroing&&) = delete;

        /**
         * @brief Rebinds cancellation without changing the servo hardware context.
         * @param[in,out] context Nullable context that outlives the next `run` call.
         * @param[in] continueOperation Non-null cancellation query.
         * @throws std::invalid_argument If `continueOperation` is null.
         */
        void setCancellation(agent::contextpointer context,
            servozeroingcontinuecallback continueOperation);

        /**
         * @brief Pulses channels zero through eleven, zeros them, and idles.
         * @return `true` after every channel reaches zero; otherwise `false` for early
         * cancellation.
         * @warning Commands physical servo movement when bound to the Raspberry Pi provider.
         */
        agent::boolean run();
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_SERVO_ZEROING_H */
