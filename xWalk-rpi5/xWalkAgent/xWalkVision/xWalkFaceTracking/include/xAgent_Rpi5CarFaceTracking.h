/******************************************************************************
 * @file        xAgent_Rpi5CarFaceTracking.h
 * @brief       Declares bounded camera-servo face tracking.
 *
 * @details
 * Ports `example/8.stare_at_you.py` through caller-owned PiCar-X and
 * computer-vision services with explicit cancellation and cleanup.
 *
 * @project     xWalk Firmware
 * @module      xWalkFaceTracking
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

#ifndef XAGENT_RPI5CAR_FACE_TRACKING_H
#define XAGENT_RPI5CAR_FACE_TRACKING_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarFaceTrackingTypes.h"
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
     * @class XWalkFaceTracking
     * @brief Converts detected face centers into bounded pan and tilt updates.
     *
     * @details
     * Stores non-owning pointers to caller-owned vehicle and provider state. The
     * class owns only retained logical angles, copied settings, and start state.
     */
    class XWalkFaceTracking final
    {
        private:
            /** @brief Non-owning PiCar-X pointer that remains valid for this lifetime. */
            XWalkPicarx* picarxObject{nullptr};
            /** @brief Nullable non-owning provider and scheduling callback context. */
            agent::contextpointer callbackContext{nullptr};
            /** @brief Complete validated computer-vision callback table. */
            XWalkComputerVisionCallbacks callbacks{};
            /** @brief Validated source geometry and timing copied by value. */
            XWalkFaceTrackingConfiguration configurationValue{};
            /** @brief Retained logical camera-pan command in degrees. */
            agent::float64 panAngleDegreesValue{};
            /** @brief Retained logical camera-tilt command in degrees. */
            agent::float64 tiltAngleDegreesValue{};
            /** @brief True after camera start and before finish. */
            agent::boolean startedValue{};

        protected:
            /** @brief Validates required callbacks, geometry, angles, and delays. */
            static void validate(const XWalkComputerVisionCallbacks& providerCallbacks,
                                 const XWalkFaceTrackingConfiguration& configuration);
            /** @brief Constrains one retained camera angle to the configured symmetric limit. */
            agent::float64 constrainAngle(agent::float64 angleDegrees) const noexcept;
            /** @brief Waits in cancellable slices no longer than 20 milliseconds. */
            agent::boolean wait(agent::uint32 durationMs) const;

        public:
            /**
             * @brief Binds caller-owned vehicle, vision provider, and scheduling operations.
             * @param[in] picarx PiCar-X coordinator that must outlive this Agent.
             * @param[in,out] context Callback context that must satisfy every operation.
             * @param[in] providerCallbacks Vision and scheduling callbacks copied by value.
             * @param[in] configuration Frame geometry, servo limits, and timing copied by value.
             * @throws std::invalid_argument If a callback or floating-point setting is invalid.
             * @throws std::out_of_range If geometry, angles, or delays exceed supported ranges.
             */
            XWalkFaceTracking(XWalkPicarx& picarx,
                              agent::contextpointer context,
                              const XWalkComputerVisionCallbacks& providerCallbacks,
                              const XWalkFaceTrackingConfiguration& configuration = {});

            /** @brief Stops the provider and latches a non-throwing motor emergency stop. */
            ~XWalkFaceTracking() noexcept;

            XWalkFaceTracking(const XWalkFaceTracking&) = delete;
            XWalkFaceTracking(XWalkFaceTracking&&) = delete;
            XWalkFaceTracking& operator=(const XWalkFaceTracking&) = delete;
            XWalkFaceTracking& operator=(XWalkFaceTracking&&) = delete;

            /**
             * @brief Starts camera acquisition, enables face detection, and resets angles.
             * @return `true` when the provider starts; otherwise `false`.
             */
            agent::boolean start();

            /**
             * @brief Applies one source-compatible face-tracking iteration.
             * @return Observation, search/tracking state, and retained camera angles.
             * @throws std::logic_error If `start()` has not succeeded.
             */
            XWalkFaceTrackingResult step();

            /** @brief Stops motors and provider, resets angles, and applies the final delay. */
            void finish();

            /** @brief Returns the retained logical camera-pan command in degrees. */
            agent::float64 panAngleDegrees() const noexcept;
            /** @brief Returns the retained logical camera-tilt command in degrees. */
            agent::float64 tiltAngleDegrees() const noexcept;
            /** @brief Reports whether camera acquisition is active. */
            agent::boolean started() const noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_FACE_TRACKING_H */
