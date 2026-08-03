/******************************************************************************
 * @file        xHal_Rpi5CarButtonEventSequence.h
 * @brief       Declares the physical D0 button-event sequence test.
 *
 * @details
 * Adapts the Robot HAT Python button_event_test.py behavior to the xWalk GPIO
 * abstraction with injected timing/output callbacks and bounded execution.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_BUTTON_EVENT_SEQUENCE_H
#define XHAL_RPI5CAR_BUTTON_EVENT_SEQUENCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains bounded HAL sequence and integration tests.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Waits for the requested duration in milliseconds. */
using sequencewaitcallback = void (*)(contextpointer context, uint32 durationMilliseconds);

/** @brief Returns an event timestamp in seconds. */
using sequencetimecallback = float64 (*)(contextpointer context);

/** @brief Reports one button press or release event. */
using sequenceeventcallback = void (*)(contextpointer context, boolean pressed,
    float64 timestampSeconds);

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkButtonEventSequence
 * @brief Prints physical D0 press and release events for a bounded interval.
 *
 * @details
 * Registers one combined rising/falling interrupt with ten-millisecond
 * debounce. The D0 input starts released through its pull-up, so each accepted
 * edge alternates between pressed and released state.
 */
class XWalkButtonEventSequence
{
private:

/**************************************************************************
 * Private data members
 **************************************************************************/

    /** @brief Caller-owned D0 GPIO that outlives this sequence. */
    XWalkGpio* gpioObject;
    /** @brief Non-owning context forwarded to injected sequence callbacks. */
    contextpointer callbackContext;
    /** @brief Non-null bounded-wait operation. */
    sequencewaitcallback waitCallback;
    /** @brief Non-null timestamp operation. */
    sequencetimecallback timeCallback;
    /** @brief Non-null event-output operation. */
    sequenceeventcallback eventCallback;
    /** @brief State reported by the most recently accepted edge. */
    atomicboolean pressedValue{false};

/**************************************************************************
 * Private member functions
 **************************************************************************/

    /** @brief Handles one accepted rising or falling D0 edge. */
    void handleEvent() noexcept;

    /**
     * @brief Adapts the GPIO callback to the owning sequence instance.
     *
     * @param[in,out] context
     * Non-null `XWalkButtonEventSequence` context.
     */
    static void eventHandler(contextpointer context) noexcept;

public:

/**************************************************************************
 * Public constructors and destructor
 **************************************************************************/

    /**
     * @brief Binds one caller-owned D0 GPIO.
     *
     * @param[in] gpio
     * D0 input configured through a Linux GPIO backend.
     *
     * @param[in,out] context
     * Non-owning context forwarded to every callback.
     *
     * @param[in] wait
     * Non-null bounded wait operation.
     *
     * @param[in] time
     * Non-null event timestamp operation.
     *
     * @param[in] event
     * Non-null event reporting operation.
     *
     * @throws std::invalid_argument
     * If any callback is null.
     */
    XWalkButtonEventSequence(XWalkGpio& gpio, contextpointer context,
        sequencewaitcallback wait, sequencetimecallback time,
        sequenceeventcallback event);

/**************************************************************************
 * Public special member functions
 **************************************************************************/

    XWalkButtonEventSequence(const XWalkButtonEventSequence&) = delete;
    XWalkButtonEventSequence(XWalkButtonEventSequence&&) = delete;
    XWalkButtonEventSequence& operator=(const XWalkButtonEventSequence&) = delete;
    XWalkButtonEventSequence& operator=(XWalkButtonEventSequence&&) = delete;

/**************************************************************************
 * Public member functions
 **************************************************************************/

    /**
     * @brief Monitors D0 for the requested bounded duration.
     *
     * @param[in] durationSeconds
     * Inclusive duration range of one through 3600 seconds.
     *
     * @throws std::out_of_range
     * If the duration is outside its supported range.
     */
    void run(uint32 durationSeconds);
};

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_BUTTON_EVENT_SEQUENCE_H */
