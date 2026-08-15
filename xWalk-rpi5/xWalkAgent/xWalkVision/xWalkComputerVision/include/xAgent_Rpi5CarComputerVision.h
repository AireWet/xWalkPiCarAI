/******************************************************************************
 * @file        xAgent_Rpi5CarComputerVision.h
 * @brief       Declares the interactive computer-vision Agent.
 *
 * @details
 * Ports `example/7.computer_vision.py` into a backend-neutral key-driven state
 * machine with caller-owned camera, detection, timing, and cancellation.
 *
 * @project     xWalk Firmware
 * @module      xWalkComputerVision
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_COMPUTER_VISION_H
#define XAGENT_RPI5CAR_COMPUTER_VISION_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarComputerVisionTypes.h"

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
     * @class XWalkComputerVision
     * @brief Coordinates source-compatible computer-vision keys and retained modes.
     *
     * @details
     * Owns no camera or provider resource. All callbacks are invoked synchronously
     * and the caller retains provider and callback-context lifetime ownership.
     */
    class XWalkComputerVision final
    {
        private:
            /** @brief Nullable non-owning context forwarded to every callback. */
            agent::contextpointer callbackContext{nullptr};
            /** @brief Complete validated callback table copied at construction. */
            XWalkComputerVisionCallbacks callbacks{};
            /** @brief Active source-compatible color mode. */
            XWalkComputerVisionColor colorValue{XWalkComputerVisionColor::Close};
            /** @brief True while face detection is enabled. */
            agent::boolean faceEnabledValue{};
            /** @brief True while QR detection is enabled. */
            agent::boolean qrEnabledValue{};
            /** @brief Last non-empty QR data reported to the application. */
            agent::string lastQrData{};
            /** @brief True after the provider has started successfully. */
            agent::boolean startedValue{};

        protected:
            /** @brief Validates that the complete callback boundary is non-null. */
            static void validateCallbacks(const XWalkComputerVisionCallbacks& providerCallbacks);
            /** @brief Waits 500 milliseconds in cancellable 20-millisecond slices. */
            agent::boolean waitAfterCommand() const;
            /** @brief Polls and records a newly decoded QR value when enabled. */
            void pollQr(XWalkComputerVisionResult& result);

        public:
            /**
             * @brief Binds one caller-owned provider and scheduling context.
             * @param[in,out] context Optional callback context that outlives this Agent.
             * @param[in] providerCallbacks Complete synchronous callback table.
             * @throws std::invalid_argument If any required callback is null.
             */
            XWalkComputerVision(agent::contextpointer context, const XWalkComputerVisionCallbacks& providerCallbacks);

            /** @brief Stops an active provider without releasing caller-owned resources. */
            ~XWalkComputerVision() noexcept;

            XWalkComputerVision(const XWalkComputerVision&) = delete;
            XWalkComputerVision(XWalkComputerVision&&) = delete;
            XWalkComputerVision& operator=(const XWalkComputerVision&) = delete;
            XWalkComputerVision& operator=(XWalkComputerVision&&) = delete;

            /**
             * @brief Starts camera acquisition and resets every retained detector mode.
             * @return `true` when the provider started; otherwise `false`.
             */
            agent::boolean start();

            /**
             * @brief Applies one source-compatible interactive key.
             * @param[in] keyText One-character key, case-insensitive.
             * @return Completed action, retained modes, observation, and QR change.
             * @throws std::logic_error If `start()` has not succeeded.
             */
            XWalkComputerVisionResult handleKey(agent::stringview keyText);

            /** @brief Disables detectors and stops the active provider. */
            void stop() noexcept;

            /** @brief Returns the lowercase name used by the upstream example. */
            static agent::string colorName(XWalkComputerVisionColor color);

            /** @brief Reports whether the provider is currently active. */
            agent::boolean started() const noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_COMPUTER_VISION_H */
