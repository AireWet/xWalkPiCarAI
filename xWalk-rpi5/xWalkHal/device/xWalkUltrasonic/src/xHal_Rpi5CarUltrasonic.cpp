/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonic.cpp
 * @brief       Implements ultrasonic trigger, timing, and distance behavior.
 *
 * @details
 * Generates the sensor trigger pulse, measures echo transitions against a
 * monotonic timeout, converts pulse duration, and performs timeout-only
 *retries.
 *
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarUltrasonic.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Acquires one ultrasonic distance sample.
     *
     * @return
     * Distance in centimeters rounded to two decimal places;
     * `XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM` on timeout; or
     * `XHAL_RPI5CAR_ULTRASONIC_INVALID_PULSE_RESULT_CM` for an incomplete pulse.
     *
     * @post
     * The trigger output is inactive after a successful GPIO write sequence.
     */
    float64 XWalkUltrasonic::readOnce()
    {
        static_cast<void>(triggerPin->off());
        common::sleepMicroseconds(XHAL_RPI5CAR_ULTRASONIC_SETTLE_TIME_US);
        static_cast<void>(triggerPin->on());
        common::sleepMicroseconds(XHAL_RPI5CAR_ULTRASONIC_TRIGGER_TIME_US);
        static_cast<void>(triggerPin->off());

        uint64 pulseStartMicroseconds = 0U;
        uint64 pulseEndMicroseconds = 0U;
        const uint64 timeoutStartMicroseconds = common::monotonicMicroseconds();
        const uint64 timeoutLimitMicroseconds = static_cast<uint64>(timeoutMicrosecondsValue);

        const hal::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const hal::boolean echoLow = static_cast<hal::boolean>(!echoPin->read());
            if (echoLow == false)
            {
                break;
            }
            pulseStartMicroseconds = common::monotonicMicroseconds();
            const uint64 elapsedMicroseconds = pulseStartMicroseconds - timeoutStartMicroseconds;
            if (elapsedMicroseconds > timeoutLimitMicroseconds)
            {
                return XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM;
            }
        }

        const hal::boolean echoCompletionWaitRequested{true};
        while (echoCompletionWaitRequested)
        {
            const hal::boolean readSucceeded = static_cast<hal::boolean>(echoPin->read());
            if (readSucceeded == false)
            {
                break;
            }
            pulseEndMicroseconds = common::monotonicMicroseconds();
            const uint64 elapsedMicroseconds = pulseEndMicroseconds - timeoutStartMicroseconds;
            if (elapsedMicroseconds > timeoutLimitMicroseconds)
            {
                return XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM;
            }
        }

        if ((pulseStartMicroseconds == 0U) || (pulseEndMicroseconds == 0U))
        {
            return XHAL_RPI5CAR_ULTRASONIC_INVALID_PULSE_RESULT_CM;
        }

        const uint64 durationMicroseconds = pulseEndMicroseconds - pulseStartMicroseconds;
        const float64 durationUs = static_cast<float64>(durationMicroseconds);
        const float64 microsecondsPerSecond = 1'000'000.0;
        const float64 durationSeconds = durationUs / microsecondsPerSecond;
        const float64 soundSpeedMps = static_cast<float64>(XHAL_RPI5CAR_ULTRASONIC_SOUND_SPEED_MPS);
        const float64 roundTripDistanceMeters = durationSeconds * soundSpeedMps;
        const float64 pathCount = 2.0;
        const float64 oneWayDistanceMeters = roundTripDistanceMeters / pathCount;
        const float64 centimetersPerMeter = 100.0;
        const float64 distanceCentimeters = oneWayDistanceMeters * centimetersPerMeter;
        const float64 fractionalScale = 100.0;
        const float64 scaledDistance = distanceCentimeters * fractionalScale;
        const float64 roundedDistance = XHAL_ROUND_NEAREST(scaledDistance);
        const float64 result = roundedDistance / fractionalScale;
        XWALK_HAL_TRACE_UID1(RPI .203, "Ultrasonic pulse measured at %.2f centimeters", result);
        return result;
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Acquires a distance sample with timeout-only retries.
     *
     * @param[in] attempts
     * Maximum number of measurements; zero performs no GPIO operations.
     *
     * @return
     * First non-timeout result in centimeters, including the invalid-pulse result,
     * or `XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM` when every attempt times out.
     */
    float64 XWalkUltrasonic::read(uint32 attempts)
    {
        for (uint32 attempt = 0U; attempt < attempts; ++attempt)
        {
            const float64 distanceCentimeters = readOnce();
            if (distanceCentimeters != XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM)
            {
                XWALK_HAL_TRACE_UID2(RPI .204,
                                     "Ultrasonic read completed after %u attempt(s) with "
                                     "result %.2f centimeters",
                                     attempt + 1U,
                                     distanceCentimeters);
                return distanceCentimeters;
            }
        }
        return XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM;
    }

    /**
     * @brief Cancels interrupt registrations associated with both GPIO
     * dependencies.
     *
     * @post
     * Neither GPIO object retains an interrupt registration managed by `XWalkGpio`.
     */
    void XWalkUltrasonic::close()
    {
        triggerPin->close();
        echoPin->close();
        XWALK_HAL_TRACE_UID0(RPI .205, "Ultrasonic GPIO registrations closed");
    }

    /**
     * @brief Returns the configured echo acquisition timeout.
     *
     * @return
     * Timeout interval in microseconds.
     */
    uint32 XWalkUltrasonic::timeoutMicroseconds() const noexcept
    {
        return timeoutMicrosecondsValue;
    }

} /* namespace xwalk::hal */
