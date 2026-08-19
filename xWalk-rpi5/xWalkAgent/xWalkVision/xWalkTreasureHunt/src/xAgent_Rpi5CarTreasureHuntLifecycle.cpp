/******************************************************************************
 * @file        xAgent_Rpi5CarTreasureHuntLifecycle.cpp
 * @brief       Implements treasure-hunt construction, validation, and
 *lifecycle.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarTreasureHunt.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains xWalk application coordinators. */
namespace xwalk::agent
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Binds caller-owned vehicle, speech, vision, and target-selection
     * services.
     * @param[in] picarx Vehicle coordinator that must outlive this object.
     * @param[in] textToSpeech Speech coordinator that must outlive this object.
     * @param[in,out] context Nullable callback context that outlives callback use.
     * @param[in] backendCallbacks Complete synchronous callback table.
     * @param[in] configuration Source-compatible bounded settings.
     */
    XWalkTreasureHunt::XWalkTreasureHunt(XWalkPicarx& picarx,
                                         hal::XWalkTextToSpeech& textToSpeech,
                                         agent::contextpointer context,
                                         const XWalkTreasureHuntCallbacks& backendCallbacks,
                                         const XWalkTreasureHuntConfiguration& configuration)
        : picarxObject(&picarx), textToSpeechObject(&textToSpeech), callbackContext(context),
          callbacks(backendCallbacks), configurationValue(configuration)
    {
        validate(callbacks, configurationValue);
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /** @brief Stops active vision and requests non-throwing fail-safe vehicle
     * shutdown. */
    XWalkTreasureHunt::~XWalkTreasureHunt() noexcept
    {
        if (startedValue)
        {
            callbacks.vision.stop(callbackContext);
        }
        static_cast<void>(picarxObject->emergencyStop());
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Starts vision, completes warm-up, and announces the first target.
     * @return `true` after complete startup; otherwise `false` after cancellation
     * or provider failure.
     */
    agent::boolean XWalkTreasureHunt::start()
    {
        if (startedValue)
        {
            return true;
        }
        startedValue = callbacks.vision.start(callbackContext);
        if (!startedValue)
        {
            return false;
        }
        const agent::boolean startupDelayCompleted = wait(configurationValue.startupDelayMs);
        if (startupDelayCompleted == false)
        {
            callbacks.vision.stop(callbackContext);
            startedValue = false;
            return false;
        }
        textToSpeechObject->speak("Game start!");
        const agent::boolean promptDelayCompleted = wait(configurationValue.promptDelayMs);
        if (promptDelayCompleted == false)
        {
            callbacks.vision.stop(callbackContext);
            startedValue = false;
            return false;
        }
        renewTarget();
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .033, "Treasure-hunt vision provider started and target selected");
        return true;
    }

    /**
     * @brief Stops vision and motion, speaks goodbye, and performs the final delay.
     * @warning Stops the physical drive motors and active camera provider.
     */
    void XWalkTreasureHunt::finish()
    {
        if (startedValue)
        {
            callbacks.vision.stop(callbackContext);
        }
        startedValue = false;
        picarxObject->stop();
        textToSpeechObject->speak("Goodbye!");
        callbacks.vision.delay(callbackContext, configurationValue.finalDelayMs);
    }

    /** @brief Reports whether the vision provider is active. @return Current
     * provider state. */
    agent::boolean XWalkTreasureHunt::started() const noexcept
    {
        return startedValue;
    }

    /** @brief Returns the currently selected target color. @return Active target
     * color. */
    XWalkComputerVisionColor XWalkTreasureHunt::targetColor() const noexcept
    {
        return targetColorValue;
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Validates callbacks and bounded source-compatible settings.
     * @param[in] backendCallbacks Callback table to validate.
     * @param[in] configuration Numeric settings to validate.
     * @throws std::invalid_argument If callbacks or finite numeric values are
     * invalid.
     * @throws std::out_of_range If a bounded setting is outside its supported
     * range.
     */
    void XWalkTreasureHunt::validate(const XWalkTreasureHuntCallbacks& backendCallbacks,
                                     const XWalkTreasureHuntConfiguration& configuration)
    {
        const XWalkComputerVisionCallbacks& vision = backendCallbacks.vision;
        if ((vision.start == nullptr) || (vision.stop == nullptr) || (vision.setColor == nullptr) ||
            (vision.observe == nullptr) || (vision.delay == nullptr) || (vision.continueOperation == nullptr) ||
            (backendCallbacks.selectColor == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Treasure hunt requires complete callbacks");
        }
        const agent::boolean configurationInvalid = static_cast<agent::boolean>(
            !XHAL_IS_FINITE(configuration.driveSpeedPercent) || !XHAL_IS_FINITE(configuration.turnAngleDegrees));
        if (configurationInvalid)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Treasure-hunt numeric settings must be finite");
        }
        if ((configuration.detectionWidthThresholdPixels == 0U) ||
            (configuration.detectionWidthThresholdPixels > 7'680U) || (configuration.driveSpeedPercent < 0.0) ||
            (configuration.driveSpeedPercent > 100.0) || (configuration.turnAngleDegrees <= 0.0) ||
            (configuration.turnAngleDegrees > 45.0) || (configuration.startupDelayMs > 10'000U) ||
            (configuration.promptDelayMs > 1'000U) || (configuration.movementDelayMs == 0U) ||
            (configuration.movementDelayMs > 10'000U) || (configuration.loopDelayMs == 0U) ||
            (configuration.loopDelayMs > 1'000U) || (configuration.finalDelayMs > 1'000U))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "Treasure-hunt configuration is out of range");
        }
    }

    /**
     * @brief Performs a cancellable delay in at most 20-millisecond slices.
     * @param[in] durationMs Requested total delay in milliseconds.
     * @return `true` after the complete delay; otherwise `false` after
     * cancellation.
     */
    agent::boolean XWalkTreasureHunt::wait(agent::uint32 durationMs) const
    {
        constexpr agent::uint32 cancellationIntervalMs{20U};
        agent::uint32 remainingMs = durationMs;
        while (remainingMs > 0U)
        {
            const agent::boolean operationRequested = callbacks.vision.continueOperation(callbackContext);
            if (operationRequested == false)
            {
                return false;
            }
            const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ? remainingMs : cancellationIntervalMs;
            callbacks.vision.delay(callbackContext, sliceMs);
            remainingMs -= sliceMs;
        }
        return callbacks.vision.continueOperation(callbackContext);
    }

    /**
     * @brief Selects, activates, and announces a new target color.
     * @throws std::invalid_argument If the selection callback returns `Close` or an
     * invalid color.
     */
    void XWalkTreasureHunt::renewTarget()
    {
        targetColorValue = callbacks.selectColor(callbackContext);
        const agent::string selectedName = colorName(targetColorValue);
        callbacks.vision.setColor(callbackContext, targetColorValue);
        textToSpeechObject->speak("Look for " + selectedName + "!");
    }

} /* namespace xwalk::agent */
