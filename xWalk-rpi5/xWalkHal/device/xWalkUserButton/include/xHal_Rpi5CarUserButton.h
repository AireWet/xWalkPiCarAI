/******************************************************************************
 * @file        xHal_Rpi5CarUserButton.h
 * @brief       Declares active-low user-button event monitoring.
 *
 * @details
 * Provides synchronized button state, press duration, context-based event
 * callbacks, long-press recognition, and an owned polling worker over a
 * caller-created GPIO input.
 *
 * @project     xWalk Firmware
 * @module      xWalkUserButton
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

#ifndef XHAL_RPI5CAR_USER_BUTTON_H
#define XHAL_RPI5CAR_USER_BUTTON_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarUserButtonTypes.h"

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
     * @class XWalkUserButton
     * @brief Monitors an active-low user button and dispatches configured events.
     *
     * @details
     * Polls a caller-owned GPIO every 50 milliseconds. Press, release, click,
     * state-change, long-press, and long-press-release callbacks execute on the
     * monitoring worker and must return promptly.
     *
     * @warning
     * Callbacks must not call `stop()`, `close()`, or destroy this object because
     * those operations would attempt to join the currently executing worker.
     * Callback contexts must remain valid until cleared and the worker is joined.
     */
    class XWalkUserButton
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Non-owning pointer to the active-low user-button GPIO input.
             *
             * @note
             * Initialized from a constructor reference, never null, and must
             * outlive this object and its monitoring worker.
             */
            XWalkGpio* gpioObject;

            /** @brief Mutex protecting state, timing, and callback configuration. */
            mutable mutexhandle stateMutex;
            /** @brief Atomic request controlling monitoring-worker execution. */
            atomicboolean monitorRunning{false};
            /** @brief Monitoring thread owned and joined by this controller. */
            threadhandle monitorThread;
            /** @brief `true` between a recognized press and release transition. */
            boolean pressedValue{false};
            /** @brief `true` after the active press crosses its armed threshold. */
            boolean longPressTriggeredValue{false};
            /** @brief `true` when long-press callbacks existed at the active press. */
            boolean longPressArmedValue{false};
            /** @brief Monotonic timestamp of the active press in microseconds. */
            uint64 pressedAtMicrosecondsValue{};
            /** @brief Duration of the most recently released press in seconds. */
            float64 pressedForSecondsValue{};
            /** @brief Shared configured long-press threshold in seconds. */
            float64 longPressDurationSecondsValue{XHAL_RPI5CAR_USER_BUTTON_DEFAULT_LONG_PRESS_SECONDS};
            /** @brief Threshold captured for the active press in microseconds. */
            uint64 activeLongPressDurationMicroseconds{};

            /** @brief Non-owning context supplied to the click callback. */
            contextpointer clickContext{nullptr};
            /** @brief Nullable callback invoked after a short press is released. */
            userbuttoncallback clickCallback{nullptr};
            /** @brief Non-owning context supplied to the press callback. */
            contextpointer pressContext{nullptr};
            /** @brief Nullable callback invoked when a press is recognized. */
            userbuttoncallback pressCallback{nullptr};
            /** @brief Non-owning context supplied to the release callback. */
            contextpointer releaseContext{nullptr};
            /** @brief Nullable callback invoked when a release is recognized. */
            userbuttoncallback releaseCallback{nullptr};
            /** @brief Non-owning context supplied to the state-change callback. */
            contextpointer stateContext{nullptr};
            /** @brief Nullable callback invoked for both press and release states. */
            userbuttonstatecallback stateCallback{nullptr};
            /** @brief Non-owning context supplied to the long-press callback. */
            contextpointer longPressContext{nullptr};
            /** @brief Nullable callback invoked once when the long threshold is crossed. */
            userbuttoncallback longPressCallback{nullptr};
            /** @brief Non-owning context supplied to the long-release callback. */
            contextpointer longReleaseContext{nullptr};
            /** @brief Nullable callback invoked when a triggered long press is released. */
            userbuttoncallback longReleaseCallback{nullptr};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Polls the GPIO and dispatches transitions until stopped. */
            void monitorLoop() noexcept;

            /** @brief Records and dispatches one active-low press transition. */
            void handlePress();

            /** @brief Records and dispatches one release transition. */
            void handleRelease();

            /** @brief Triggers an armed long press once its threshold is reached. */
            void handleLongPress();

            /** @brief Stops and joins the monitoring worker. */
            void stopWorker();

            /**
             * @brief Clamps a finite long-press threshold to the supported range.
             *
             * @param[in] durationSeconds
             * Requested threshold in seconds.
             *
             * @return
             * Threshold clamped to the inclusive range 2.0 to 5.0 seconds.
             *
             * @throws std::invalid_argument
             * If the requested threshold is not finite.
             */
            static float64 validatedLongPressDuration(float64 durationSeconds);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a stopped user-button monitor.
             *
             * @param[in] gpio
             * Non-owning GPIO configured as the active-low pull-up button input.
             *
             * @pre
             * `gpio` outlives this object and its monitoring worker.
             */
            explicit XWalkUserButton(XWalkGpio& gpio);

            /** @brief Stops the monitoring worker without releasing the caller-owned GPIO. */
            ~XWalkUserButton();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction because the worker retains object identity. */
            XWalkUserButton(XWalkUserButton&&) = delete;
            /** @brief Disables copying of GPIO, worker, and callback bindings. */
            XWalkUserButton(const XWalkUserButton&) = delete;
            /** @brief Disables move assignment because the worker retains object identity. */
            XWalkUserButton& operator=(XWalkUserButton&&) = delete;
            /** @brief Disables copy assignment of GPIO, worker, and callback bindings. */
            XWalkUserButton& operator=(const XWalkUserButton&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Starts monitoring when no worker is currently running.
             *
             * @post
             * `isRunning()` returns `true` after successful worker creation.
             *
             * @throws std::system_error
             * If the monitoring thread cannot be created.
             *
             * @warning
             * GPIO and callback operations on the worker must not throw.
             */
            void start();

            /**
             * @brief Stops and joins the monitoring worker.
             *
             * @post
             * No joinable worker remains and `isRunning()` returns `false`.
             *
             */
            void stop();

            /**
             * @brief Provides the Python-compatible control alias for `stop()`.
             *
             */
            void close();

            /**
             * @brief Configures or clears the short-click callback and context.
             *
             * @param[in,out] context
             * Non-owning callback context; nullability is callback-specific.
             *
             * @param[in] callback
             * Nullable callback invoked after a short press is released.
             *
             * @warning
             * A non-null context must remain valid until the callback is cleared and
             * the monitoring worker is joined.
             */
            void setOnClick(contextpointer context, userbuttoncallback callback);

            /**
             * @brief Configures or clears the press callback and context.
             *
             * @param[in,out] context
             * Non-owning callback context; nullability is callback-specific.
             *
             * @param[in] callback
             * Nullable callback invoked when a press is recognized.
             */
            void setOnPress(contextpointer context, userbuttoncallback callback);

            /**
             * @brief Configures or clears the release callback and context.
             *
             * @param[in,out] context
             * Non-owning callback context; nullability is callback-specific.
             *
             * @param[in] callback
             * Nullable callback invoked when a release is recognized.
             */
            void setOnRelease(contextpointer context, userbuttoncallback callback);

            /**
             * @brief Configures or clears the press/release state callback and context.
             *
             * @param[in,out] context
             * Non-owning callback context; nullability is callback-specific.
             *
             * @param[in] callback
             * Nullable callback receiving `true` for press and `false` for release.
             */
            void setOnPressReleased(contextpointer context, userbuttonstatecallback callback);

            /**
             * @brief Configures or clears the long-press callback and shared threshold.
             *
             * @param[in,out] context
             * Non-owning callback context; nullability is callback-specific.
             *
             * @param[in] callback
             * Nullable callback invoked once for an armed long press.
             *
             * @param[in] durationSeconds
             * Finite threshold clamped to the inclusive range 2.0 to 5.0 seconds.
             *
             * @throws std::invalid_argument
             * If `durationSeconds` is not finite.
             */
            void setOnLongPress(contextpointer context,
                                userbuttoncallback callback,
                                float64 durationSeconds = XHAL_RPI5CAR_USER_BUTTON_DEFAULT_LONG_PRESS_SECONDS);

            /**
             * @brief Configures or clears the long-press-release callback and threshold.
             *
             * @param[in,out] context
             * Non-owning callback context; nullability is callback-specific.
             *
             * @param[in] callback
             * Nullable callback invoked when a triggered long press is released.
             *
             * @param[in] durationSeconds
             * Finite threshold clamped to the inclusive range 2.0 to 5.0 seconds.
             *
             * @throws std::invalid_argument
             * If `durationSeconds` is not finite.
             */
            void setOnLongPressReleased(contextpointer context,
                                        userbuttoncallback callback,
                                        float64 durationSeconds = XHAL_RPI5CAR_USER_BUTTON_DEFAULT_LONG_PRESS_SECONDS);

            /**
             * @brief Returns whether the button is recognized as pressed.
             *
             * @return
             * `true` between a recognized press and release; otherwise `false`.
             */
            boolean state() const;

            /**
             * @brief Provides the Python-compatible state alias for `state()`.
             *
             * @return
             * `true` while the button is recognized as pressed; otherwise `false`.
             */
            boolean isPressed() const;

            /**
             * @brief Returns the active or most recently completed press duration.
             *
             * @return
             * Elapsed duration in seconds.
             */
            float64 pressedForSeconds() const;

            /**
             * @brief Returns the shared long-press threshold.
             *
             * @return
             * Configured threshold in the inclusive range 2.0 to 5.0 seconds.
             */
            float64 longPressDurationSeconds() const;

            /**
             * @brief Returns whether the monitoring worker is requested to run.
             *
             * @return
             * `true` while monitoring remains requested; otherwise `false`.
             */
            boolean isRunning() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_USER_BUTTON_H */
