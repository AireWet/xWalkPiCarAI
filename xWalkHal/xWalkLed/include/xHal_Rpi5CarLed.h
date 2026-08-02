/******************************************************************************
 * @file        xHal_Rpi5CarLed.h
 * @brief       Declares GPIO-backed LED control and background blinking.
 *
 * @details
 * Provides synchronous logical output operations and an owned, joinable worker
 * that repeatedly performs configurable blink sequences on a caller-owned GPIO.
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

#ifndef XHAL_RPI5CAR_LED_H
#define XHAL_RPI5CAR_LED_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkLed
 * @brief Controls one logical LED output and its optional blink worker.
 *
 * @details
 * Uses the polarity already configured by `XWalkGpio`. Starting a blink worker
 * replaces any previous worker, while direct output operations stop blinking
 * before changing the LED state.
 */
class XWalkLed
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Non-owning pointer to the GPIO output connected to the LED.
         *
         * @note
         * Initialized from a constructor reference, never null, and must
         * outlive this object and its blink worker.
         */
        XWalkGpio* gpioObject;

        /** @brief Atomic logical LED level shared with the blink worker. */
        atomicboolean outputValue{false};
        /** @brief Atomic request controlling continued blink-worker execution. */
        atomicboolean blinkRunning{false};

        /**
         * @brief Worker thread owned and joined by this LED controller.
         *
         * @note
         * A default-constructed handle represents no active or joinable worker.
         */
        threadhandle blinkThread;

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

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
         * A hardware exception terminates the process because the worker does
         * not install an exception handler.
         */
        void blinkLoop(uint32 cycleCount, uint32 toggleDelayUs, uint32 pauseUs) noexcept;

        /**
         * @brief Waits for a duration while periodically checking the stop request.
         *
         * @param[in] durationUs
         * Total requested delay in microseconds.
         *
         * @return
         * `true` if blinking remains requested after the delay; otherwise `false`.
         */
        boolean waitWhileBlinking(uint32 durationUs) const;

        /**
         * @brief Toggles the LED without stopping the current blink worker.
         *
         * @post
         * `isOn()` reflects the successfully written opposite logical state.
         */
        void toggleFromWorker();

        /** @brief Stops and joins the worker. */
        void stopWorker();

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
        static uint32 durationMicroseconds(float64 durationSeconds, cstring parameterName);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs an inactive LED controller.
         *
         * @param[in] gpio
         * Non-owning GPIO output whose logical polarity represents LED state.
         *
         * @pre
         * `gpio` outlives this object and any blink operation.
         *
         * @post
         * The GPIO output is logically inactive.
         */
        explicit XWalkLed(XWalkGpio& gpio);

        /**
         * @brief Stops the blink worker before destroying the LED controller.
         *
         * @note
         * The non-owning GPIO is not released and no exception is propagated.
         */
        ~XWalkLed();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction because the worker retains object identity. */
        XWalkLed(XWalkLed&&) = delete;
        /** @brief Disables copying of the GPIO and worker bindings. */
        XWalkLed(const XWalkLed&) = delete;
        /** @brief Disables move assignment because the worker retains object identity. */
        XWalkLed& operator=(XWalkLed&&) = delete;
        /** @brief Disables copy assignment of GPIO and worker bindings. */
        XWalkLed& operator=(const XWalkLed&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Stops blinking and activates the LED.
         *
         * @post
         * The GPIO is logically active and `isOn()` returns `true`.
         */
        void on();

        /**
         * @brief Stops blinking and deactivates the LED.
         *
         * @post
         * The GPIO is logically inactive and `isOn()` returns `false`.
         */
        void off();

        /**
         * @brief Stops blinking and reverses the logical LED state.
         *
         * @post
         * `isOn()` returns the opposite of its value before this call.
         */
        void toggle();

        /**
         * @brief Starts continuous background blink sequences.
         *
         * @param[in] cycleCount
         * Complete on/off cycles per sequence; valid range starts at one and
         * must permit multiplication by two in `uint32`.
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
        void blink(uint32 cycleCount = XHAL_RPI5CAR_LED_DEFAULT_BLINK_COUNT,
            float64 toggleDelaySeconds = XHAL_RPI5CAR_LED_DEFAULT_TOGGLE_DELAY_SECONDS,
            float64 pauseSeconds = XHAL_RPI5CAR_LED_DEFAULT_PAUSE_SECONDS);

        /**
         * @brief Stops and joins the active or completed blink worker.
         *
         * @post
         * No joinable worker remains and `isBlinking()` returns `false`.
         *
         */
        void stopBlinking();

        /**
         * @brief Stops blinking and requests the inactive LED state.
         *
         * @details
         * The caller-owned GPIO remains configured and is not released.
         */
        void close();

        /**
         * @brief Reports the most recently completed logical output state.
         *
         * @return
         * `true` when the LED is logically active; otherwise `false`.
         */
        boolean isOn() const noexcept;

        /**
         * @brief Reports whether the blink worker is requested to continue.
         *
         * @return
         * `true` while background blinking remains requested; otherwise `false`.
         */
        boolean isBlinking() const noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_LED_H */
