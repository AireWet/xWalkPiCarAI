/******************************************************************************
 * @file        xAgent_Rpi5CarLineTracking.h
 * @brief       Declares the PiCar-X line-tracking agent coordinator.
 *
 * @details
 * Ports `example/6.line_tracking.py` into bounded steering, forward movement,
 * and line-lost recovery through one caller-owned PiCar-X coordinator.
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

#ifndef XAGENT_RPI5CAR_LINE_TRACKING_H
#define XAGENT_RPI5CAR_LINE_TRACKING_H

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "xAgent_Rpi5CarLineTrackingTypes.h"
#include "xAgent_Rpi5CarPicarx.h"

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
     * @class XWalkLineTracking
     * @brief Performs one bounded iteration of the upstream line-following example.
     *
     * @details
     * Stores a non-owning PiCar-X pointer and copied control settings. Applications
     * own repeated scheduling, diagnostics, cancellation, and hardware composition.
     */
    class XWalkLineTracking
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning PiCar-X pointer that is never null after construction. */
            XWalkPicarx* picarxObject{nullptr};
            /** @brief Nullable non-owning context passed to the delay callback. */
            agent::contextpointer callbackContext{nullptr};
            /** @brief Non-null synchronous delay operation copied from the caller. */
            linetrackingdelaycallback delayCallback{nullptr};
            /** @brief Validated movement and recovery settings stored by value. */
            XWalkLineTrackingConfiguration configurationValue{};
            /** @brief State classified from the most recent sensor sample. */
            XWalkLineTrackingState currentStateValue{XWalkLineTrackingState::Stop};
            /** @brief Most recent non-stop state used to select recovery direction. */
            XWalkLineTrackingState lastStateValue{XWalkLineTrackingState::Stop};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Validates the complete movement, recovery, and callback contract. */
            static void validateConfiguration(const XWalkLineTrackingConfiguration& configuration,
                                              linetrackingdelaycallback callback);
            /** @brief Invokes the application-owned delay operation. */
            void delay(agent::uint32 durationMs) const;
            /** @brief Applies one non-stop line-following movement decision. */
            void applyTrackingState(XWalkLineTrackingState state);
            /** @brief Performs one bounded line-lost recovery attempt. */
            XWalkLineTrackingResult recoverLine(const hal::linetrackervalues& initialReadings);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

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
            XWalkLineTracking(XWalkPicarx& picarx,
                              agent::contextpointer context,
                              linetrackingdelaycallback callback,
                              const XWalkLineTrackingConfiguration& configuration = {});

            /**
             * @brief Stops the drive motors without releasing the PiCar-X dependency.
             *
             * @warning
             * The injected motor backend must not throw during destruction.
             */
            ~XWalkLineTracking();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction to preserve dependency identity. */
            XWalkLineTracking(XWalkLineTracking&&) = delete;
            /** @brief Disables copying of the non-owning dependency binding. */
            XWalkLineTracking(const XWalkLineTracking&) = delete;
            /** @brief Disables move assignment to preserve dependency identity. */
            XWalkLineTracking& operator=(XWalkLineTracking&&) = delete;
            /** @brief Disables copying of the non-owning dependency binding. */
            XWalkLineTracking& operator=(const XWalkLineTracking&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Classifies a grayscale status using upstream decision priority.
             *
             * @param[in] status
             * Left, middle, and right values where one identifies the line.
             *
             * @return
             * Stop for all zeroes, then forward, right, or left by upstream priority.
             */
            static XWalkLineTrackingState classify(const hal::linetrackerstatus& status) noexcept;

            /**
             * @brief Acquires sensors and performs one bounded line-following iteration.
             *
             * @return
             * Final readings, classified state, and recovery outcome.
             */
            XWalkLineTrackingResult step();

            /**
             * @brief Stops the drive motors and resets both retained states.
             *
             * @post
             * `currentState()` and `lastState()` return Stop.
             */
            void stop();

            /**
             * @brief Stops, resets retained state, and applies the example's final delay.
             *
             * @post
             * `currentState()` and `lastState()` return Stop and the delay callback
             * has received 100 milliseconds.
             */
            void finish();

            /**
             * @brief Returns the most recently classified state.
             *
             * @return
             * Current line-tracking state.
             */
            XWalkLineTrackingState currentState() const noexcept;

            /**
             * @brief Returns the most recent non-stop tracking state.
             *
             * @return
             * Direction retained for the next line-lost recovery attempt.
             */
            XWalkLineTrackingState lastState() const noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_LINE_TRACKING_H */
