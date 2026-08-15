/******************************************************************************
 * @file        xHal_Rpi5CarPiperStreamSequence.cpp
 * @brief       Implements the injected Piper stream-comparison sequence.
 *
 * @details
 * Preserves the upstream status, timing, synthesis, separator, and stream-mode
 * order while keeping provider and console behavior behind callbacks.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarPiperStreamSequence.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

    /**
     * @brief Binds the Piper provider, clock, and reporting operations.
     *
     * @param[in,out] context
     * Non-owning context forwarded to all callbacks.
     *
     * @param[in] speak
     * Non-null synchronous provider operation supporting both stream modes.
     *
     * @param[in] time
     * Non-null monotonic time operation returning seconds.
     *
     * @param[in] reportMessage
     * Non-null literal-message reporting operation.
     *
     * @param[in] reportDuration
     * Non-null elapsed-duration reporting operation.
     *
     * @throws std::invalid_argument
     * If any callback is null.
     */
    XWalkPiperStreamSequence::XWalkPiperStreamSequence(contextpointer context,
                                                       piperstreamspeakcallback speak,
                                                       piperstreamtimecallback time,
                                                       piperstreammessagecallback reportMessage,
                                                       piperstreamdurationcallback reportDuration)
        : callbackContext(context), speakCallback(speak), timeCallback(time), messageCallback(reportMessage),
          durationCallback(reportDuration)
    {
        if ((speakCallback == nullptr) || (timeCallback == nullptr) || (messageCallback == nullptr) ||
            (durationCallback == nullptr))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Piper-stream callbacks must not be null");
        }
    }

    /**
     * @brief Runs streamed synthesis followed by buffered synthesis.
     *
     * @post
     * Both requests receive the same model and text, and each duration is reported
     * immediately after its corresponding request.
     *
     * @throws std::runtime_error
     * If the monotonic clock moves backwards during either request.
     *
     * @note
     * Exceptions from provider or reporting callbacks are propagated.
     */
    void XWalkPiperStreamSequence::run()
    {
        messageCallback(callbackContext, "Say with stream");
        const float64 streamedStartSeconds = timeCallback(callbackContext);
        speakCallback(callbackContext, XHAL_RPI5CAR_PIPER_STREAM_MODEL, XHAL_RPI5CAR_PIPER_STREAM_TEXT, true);
        const float64 streamedEndSeconds = timeCallback(callbackContext);
        if (streamedEndSeconds < streamedStartSeconds)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Piper streamed timing moved backwards");
        }
        durationCallback(callbackContext, streamedEndSeconds - streamedStartSeconds);

        messageCallback(callbackContext, XHAL_RPI5CAR_PIPER_STREAM_SEPARATOR);
        messageCallback(callbackContext, "Say without stream");
        const float64 bufferedStartSeconds = timeCallback(callbackContext);
        speakCallback(callbackContext, XHAL_RPI5CAR_PIPER_STREAM_MODEL, XHAL_RPI5CAR_PIPER_STREAM_TEXT, false);
        const float64 bufferedEndSeconds = timeCallback(callbackContext);
        if (bufferedEndSeconds < bufferedStartSeconds)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Piper buffered timing moved backwards");
        }
        durationCallback(callbackContext, bufferedEndSeconds - bufferedStartSeconds);
    }

} /* namespace xwalk::hal::test */
