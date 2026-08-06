/******************************************************************************
 * @file        xHal_Rpi5CarLed.cpp
 * @brief       Implements LED output and background blink behavior.
 *
 * @details
 * Serializes direct output operations with worker shutdown, converts public
 * timing values, and performs interruptible delays.
 *
 * @project     xWalk Firmware
 * @module      xWalkLed
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

#include "xHal_Rpi5CarLed.h"

#include "xHal_Rpi5CarExceptions.h"

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
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Stops blinking and activates the LED.
 *
 * @post
 * The GPIO is logically active and `isOn()` returns `true`.
 */
void XWalkLed::on()
{
    stopBlinking();
    static_cast<void>(gpioObject->on());
    outputValue.store(true);
}

/**
 * @brief Stops blinking and deactivates the LED.
 *
 * @post
 * The GPIO is logically inactive and `isOn()` returns `false`.
 */
void XWalkLed::off()
{
    stopBlinking();
    static_cast<void>(gpioObject->off());
    outputValue.store(false);
}

/**
 * @brief Stops blinking and reverses the logical LED state.
 *
 * @post
 * `isOn()` returns the opposite of its value before this call.
 */
void XWalkLed::toggle()
{
    stopBlinking();
    toggleFromWorker();
}

/**
 * @brief Starts continuous background blink sequences.
 *
 * @param[in] cycleCount
 * Complete on/off cycles per sequence; valid range starts at one and must
 * permit multiplication by two in `uint32`.
 *
 * @param[in] toggleDelaySeconds
 * Finite delay between transitions in seconds, greater than or equal to zero.
 *
 * @param[in] pauseSeconds
 * Finite inactive delay after each sequence in seconds, greater than or equal to zero.
 *
 * @post
 * A joinable worker repeatedly performs the configured sequence until stopped.
 *
 * @throws std::out_of_range
 * If the cycle count or a converted duration exceeds its supported range.
 *
 * @throws std::invalid_argument
 * If either duration is not finite.
 *
 * @throws std::system_error
 * If the worker thread cannot be created.
 */
void XWalkLed::blink(uint32 cycleCount, float64 toggleDelaySeconds, float64 pauseSeconds)
{
    if ((cycleCount == 0U) || (cycleCount > XHAL_RPI5CAR_LED_MAX_BLINK_COUNT))
    {
        XHAL_THROW_OUT_OF_RANGE("LED blink count is outside its supported range");
    }

    const uint32 requestedToggleDelayUs =
        durationMicroseconds(toggleDelaySeconds, "LED toggle delay");
    const uint32 toggleDelayUs = XHAL_MAXIMUM_VALUE(requestedToggleDelayUs,
        XHAL_RPI5CAR_LED_STOP_POLL_INTERVAL_US);
    const uint32 pauseUs = durationMicroseconds(pauseSeconds, "LED pause");

    stopBlinking();
    blinkRunning.store(true);
    blinkThread = threadhandle(&XWalkLed::blinkLoop, this, cycleCount, toggleDelayUs, pauseUs);
}

/**
 * @brief Stops and joins the active or completed blink worker.
 *
 * @post
 * No joinable worker remains and `isBlinking()` returns `false`.
 *
 */
void XWalkLed::stopBlinking()
{
    stopWorker();
}

/**
 * @brief Stops blinking and requests the inactive LED state.
 *
 * @details
 * The caller-owned GPIO remains configured and is not released.
 */
void XWalkLed::close()
{
    stopBlinking();
    static_cast<void>(gpioObject->off());
    outputValue.store(false);
}

/**
 * @brief Reports the most recently completed logical output state.
 *
 * @return
 * `true` when the LED is logically active; otherwise `false`.
 */
boolean XWalkLed::isOn() const noexcept
{
    return outputValue.load();
}

/**
 * @brief Reports whether the blink worker is requested to continue.
 *
 * @return
 * `true` while background blinking remains requested; otherwise `false`.
 */
boolean XWalkLed::isBlinking() const noexcept
{
    return blinkRunning.load();
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Executes repeated LED blink sequences on the owned worker.
 *
 * @param[in] cycleCount
 * Complete on/off cycles in each sequence; valid range starts at one.
 *
 * @param[in] toggleDelayUs
 * Delay between transitions in microseconds.
 *
 * @param[in] pauseUs
 * Inactive delay after each complete sequence in microseconds.
 *
 * @post
 * Normal completion leaves the LED logically inactive.
 *
 * @warning
 * A hardware exception terminates the process because this worker does not
 * install an exception handler.
 */
void XWalkLed::blinkLoop(uint32 cycleCount, uint32 toggleDelayUs, uint32 pauseUs) noexcept
{
    static_cast<void>(gpioObject->off());
    outputValue.store(false);
    const uint32 toggleCount = cycleCount * XHAL_RPI5CAR_LED_TOGGLES_PER_CYCLE;

    const hal::boolean processingLoopRequested{true};
    while (processingLoopRequested)
    {
        const hal::boolean blinkRequested =
            static_cast<hal::boolean>(
                blinkRunning.load());
        if (blinkRequested == false)
        {
            break;
        }
        for (uint32 toggleIndex = 0U;
            (toggleIndex < toggleCount) && blinkRunning.load(); ++toggleIndex)
        {
            const hal::boolean waitWhileBlinkingSucceeded =
                static_cast<hal::boolean>(
                    waitWhileBlinking(toggleDelayUs));
            if (waitWhileBlinkingSucceeded)
            {
                toggleFromWorker();
            }
        }

        const hal::boolean blinkStillActive =
            static_cast<hal::boolean>(
                blinkRunning.load());
        if (blinkStillActive)
        {
            static_cast<void>(waitWhileBlinking(pauseUs));
            static_cast<void>(waitWhileBlinking(XHAL_RPI5CAR_LED_STOP_POLL_INTERVAL_US));
        }
    }

    static_cast<void>(gpioObject->off());
    outputValue.store(false);
    blinkRunning.store(false);
}

/**
 * @brief Waits for a duration while periodically checking the stop request.
 *
 * @param[in] durationUs
 * Total requested delay in microseconds.
 *
 * @return
 * `true` if blinking remains requested after the delay; otherwise `false`.
 */
boolean XWalkLed::waitWhileBlinking(uint32 durationUs) const
{
    uint32 remainingUs = durationUs;
    const hal::boolean blinkWaitRequested{true};
    while (blinkWaitRequested)
    {
        const hal::boolean blinkMayContinue =
            static_cast<hal::boolean>(
                (remainingUs > 0U) && blinkRunning.load());
        if (blinkMayContinue == false)
        {
            break;
        }
        const uint32 delayUs = remainingUs > XHAL_RPI5CAR_LED_STOP_POLL_INTERVAL_US
            ? XHAL_RPI5CAR_LED_STOP_POLL_INTERVAL_US : remainingUs;
        common::sleepMicroseconds(delayUs);
        remainingUs -= delayUs;
    }
    return blinkRunning.load();
}

/**
 * @brief Toggles the LED without stopping the current blink worker.
 *
 * @post
 * `isOn()` reflects the successfully written opposite logical state.
 */
void XWalkLed::toggleFromWorker()
{
    const boolean nextValue = !outputValue.load();
    static_cast<void>(gpioObject->write(nextValue));
    outputValue.store(nextValue);
}

/**
 * @brief Stops and joins the worker.
 */
void XWalkLed::stopWorker()
{
    blinkRunning.store(false);
    const hal::boolean blinkThreadJoinable =
        static_cast<hal::boolean>(
            blinkThread.joinable());
    if (blinkThreadJoinable)
    {
        blinkThread.join();
    }

}

/**
 * @brief Converts a non-negative LED timing value to microseconds.
 *
 * @param[in] durationSeconds
 * Finite duration in seconds, greater than or equal to zero.
 *
 * @param[in] parameterName
 * Non-null parameter name used in validation messages.
 *
 * @return
 * Rounded duration in microseconds.
 *
 * @throws std::invalid_argument
 * If the duration is not finite.
 *
 * @throws std::out_of_range
 * If the duration is negative or exceeds the supported microsecond range.
 */
uint32 XWalkLed::durationMicroseconds(float64 durationSeconds, cstring parameterName)
{
    const hal::boolean durationSecondsNotFinite =
        static_cast<hal::boolean>(
            !XHAL_IS_FINITE(durationSeconds));
    if (durationSecondsNotFinite)
    {
        XHAL_THROW_INVALID_ARGUMENT_DETAIL(parameterName, " must be finite");
    }
    if (durationSeconds < 0.0)
    {
        XHAL_THROW_OUT_OF_RANGE_DETAIL(parameterName, " must not be negative");
    }

    const float64 durationUs = durationSeconds * XHAL_RPI5CAR_LED_MICROSECONDS_PER_SECOND;
    return common::roundedValue(durationUs, parameterName, 0U, XHAL_RPI5CAR_UINT32_MAX);
}

} /* namespace xwalk::hal */
