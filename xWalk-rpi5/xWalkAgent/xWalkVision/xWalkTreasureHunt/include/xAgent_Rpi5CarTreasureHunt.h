/******************************************************************************
 * @file        xAgent_Rpi5CarTreasureHunt.h
 * @brief       Declares the interactive color treasure-hunt coordinator.
 *
 * @details
 * Ports `example/20.treasure_hunt.py` through caller-owned PiCar-X, vision,
 * speech, scheduling, and target-selection dependencies.
 *
 * @project     xWalk Firmware
 * @module      xWalkTreasureHunt
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

#ifndef XAGENT_RPI5CAR_TREASURE_HUNT_H
#define XAGENT_RPI5CAR_TREASURE_HUNT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarTreasureHuntTypes.h"
#include "xHal_Rpi5CarTextToSpeech.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains xWalk application coordinators. */
namespace xwalk::agent
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkTreasureHunt
     * @brief Coordinates random color targets, bounded driving, and spoken prompts.
     *
     * @details
     * Observes caller-owned dependencies and owns only retained target state. The
     * composition root must keep every dependency and callback context alive.
     */
    class XWalkTreasureHunt final
    {
        private:
            /** @brief Non-owning PiCar-X pointer that remains valid for this object's lifetime. */
            XWalkPicarx* picarxObject{nullptr};
            /** @brief Non-owning speech pointer that remains valid for this object's lifetime. */
            hal::XWalkTextToSpeech* textToSpeechObject{nullptr};
            /** @brief Nullable non-owning context that outlives callback use. */
            agent::contextpointer callbackContext{nullptr};
            /** @brief Complete copied callback table. */
            XWalkTreasureHuntCallbacks callbacks{};
            /** @brief Owned source-compatible settings. */
            XWalkTreasureHuntConfiguration configurationValue{};
            /** @brief Currently selected target color. */
            XWalkComputerVisionColor targetColorValue{XWalkComputerVisionColor::Red};
            /** @brief True while the vision provider is active. */
            agent::boolean startedValue{};

        protected:
            /** @brief Validates callbacks and bounded numeric configuration. */
            static void validate(const XWalkTreasureHuntCallbacks& backendCallbacks,
                                 const XWalkTreasureHuntConfiguration& configuration);
            /** @brief Performs one cancellable delay. */
            agent::boolean wait(agent::uint32 durationMs) const;
            /** @brief Selects, activates, and announces a new target color. */
            void renewTarget();
            /** @brief Applies one source-compatible movement key. */
            agent::boolean move(char key);

        public:
            /**
             * @brief Binds caller-owned vehicle, speech, vision, and selection services.
             * @param[in] picarx Vehicle coordinator that must outlive this object.
             * @param[in] textToSpeech Speech coordinator that must outlive this object.
             * @param[in,out] context Nullable callback context that outlives callback use.
             * @param[in] backendCallbacks Complete synchronous callback table.
             * @param[in] configuration Source-compatible bounded settings.
             */
            XWalkTreasureHunt(XWalkPicarx& picarx,
                              hal::XWalkTextToSpeech& textToSpeech,
                              agent::contextpointer context,
                              const XWalkTreasureHuntCallbacks& backendCallbacks,
                              const XWalkTreasureHuntConfiguration& configuration = {});

            /** @brief Stops active vision and requests fail-safe vehicle shutdown. */
            ~XWalkTreasureHunt() noexcept;

            XWalkTreasureHunt(const XWalkTreasureHunt&) = delete;
            XWalkTreasureHunt(XWalkTreasureHunt&&) = delete;
            XWalkTreasureHunt& operator=(const XWalkTreasureHunt&) = delete;
            XWalkTreasureHunt& operator=(XWalkTreasureHunt&&) = delete;

            /** @brief Starts vision, completes warm-up, and announces the first target. */
            agent::boolean start();
            /** @brief Checks the target and applies one keyboard command. */
            XWalkTreasureHuntResult step(const agent::string& key);
            /** @brief Stops vision and motion, speaks goodbye, and performs the final delay. */
            void finish();
            /** @brief Reports whether the vision provider is active. */
            agent::boolean started() const noexcept;
            /** @brief Returns the currently selected target color. */
            XWalkComputerVisionColor targetColor() const noexcept;
            /** @brief Returns the lowercase display name for one selectable color. */
            static agent::string colorName(XWalkComputerVisionColor color);
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_TREASURE_HUNT_H */
