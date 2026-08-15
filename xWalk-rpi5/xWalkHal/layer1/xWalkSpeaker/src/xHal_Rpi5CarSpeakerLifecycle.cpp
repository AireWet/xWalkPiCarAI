/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerLifecycle.cpp
 * @brief       Implements speaker construction, destruction, and output state.
 *
 * @details
 * Validates the injected backend, enables output during construction, and
 * stops retained tasks, and disables output during destruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker
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

#include "xHal_Rpi5CarSpeaker.h"

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
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs a speaker controller and enables physical output.
     *
     * @param[in,out] context
     * Non-owning backend context; nullability is backend-specific.
     *
     * @param[in] backendCallbacks
     * Complete callback table copied into the controller.
     *
     * @pre
     * Any non-null context outlives this controller and all playback workers.
     *
     * @post
     * `isSpeakerEnabled()` returns `true` after the enable callback succeeds.
     *
     * @throws std::invalid_argument
     * If any required backend callback is null.
     */
    XWalkSpeaker::XWalkSpeaker(contextpointer context, const XWalkSpeakerCallbacks& backendCallbacks)
        : backendContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
        enableSpeaker();
        XWALK_HAL_TRACE_UID0(RPI .308, "Speaker controller constructed with output enabled");
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Stops all tasks and disables output.
     *
     * @note
     * Backend stream and output resources are released through callbacks; the
     * non-owning context itself is never released.
     *
     * @warning
     * Backend cleanup callbacks invoked during destruction must not throw.
     */
    XWalkSpeaker::~XWalkSpeaker()
    {
        stopAllTasks();
        disableSpeaker();
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Enables physical output if it is currently disabled.
     *
     * @post
     * `isSpeakerEnabled()` returns `true` after the backend callback succeeds.
     */
    void XWalkSpeaker::enableSpeaker()
    {
        {
            const mutexlock lock(stateMutex);
            if (speakerEnabled)
            {
                return;
            }
        }
        callbacks.enableOutput(backendContext);
        {
            const mutexlock lock(stateMutex);
            speakerEnabled = true;
        }
        XWALK_HAL_TRACE_UID0(RPI .309, "Speaker output enabled");
    }

    /**
     * @brief Disables physical output if it is currently enabled.
     *
     * @post
     * `isSpeakerEnabled()` returns `false` after the backend callback succeeds.
     */
    void XWalkSpeaker::disableSpeaker()
    {
        {
            const mutexlock lock(stateMutex);
            if (!speakerEnabled)
            {
                return;
            }
        }
        callbacks.disableOutput(backendContext);
        const mutexlock lock(stateMutex);
        speakerEnabled = false;
    }

    /**
     * @brief Reports the logical speaker-enable state.
     *
     * @return
     * `true` after successful enable and before successful disable.
     */
    boolean XWalkSpeaker::isSpeakerEnabled() const noexcept
    {
        const mutexlock lock(stateMutex);
        return speakerEnabled;
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Validates that every required backend callback is non-null.
     *
     * @param[in] backendCallbacks
     * Callback table to validate before any operation is invoked.
     *
     * @throws std::invalid_argument
     * If any callback is null.
     */
    void XWalkSpeaker::validateCallbacks(const XWalkSpeakerCallbacks& backendCallbacks)
    {
        const boolean outputMissing =
            (backendCallbacks.enableOutput == nullptr) || (backendCallbacks.disableOutput == nullptr);
        const boolean streamMissing = (backendCallbacks.openStream == nullptr) ||
                                      (backendCallbacks.writeStream == nullptr) ||
                                      (backendCallbacks.closeStream == nullptr);
        const boolean dataMissing =
            (backendCallbacks.decodeAudio == nullptr) || (backendCallbacks.createTaskId == nullptr);
        if (outputMissing || streamMissing || dataMissing)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Speaker backend requires a complete callback table");
        }
    }

} /* namespace xwalk::hal */
