/******************************************************************************
 * @file        xHal_Rpi5CarPiperStreamSequence.h
 * @brief       Declares the injected Piper stream-comparison sequence.
 *
 * @details
 * Defines the exact upstream model, text, streamed and buffered request order,
 * monotonic timing boundaries, and reporting operations without owning Piper.
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

#ifndef XHAL_RPI5CAR_PIPER_STREAM_SEQUENCE_H
#define XHAL_RPI5CAR_PIPER_STREAM_SEQUENCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

    /******************************************************************************
     * Constants
     ******************************************************************************/

    /** @brief Piper voice model selected by the upstream comparison script. */
    inline constexpr stringview XHAL_RPI5CAR_PIPER_STREAM_MODEL{"en_US-amy-low"};

    /** @brief Exact English sentence synthesized by both upstream requests. */
    inline constexpr stringview XHAL_RPI5CAR_PIPER_STREAM_TEXT{
        "Hi, I'm piper TTS. A fast and local neural text-to-speech engine that "
        "embeds espeak-ng for phonemization."};

    /** @brief Separator printed between the streamed and buffered requests. */
    inline constexpr stringview XHAL_RPI5CAR_PIPER_STREAM_SEPARATOR{
        "===================================================="};

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Synthesizes the sequence text with explicit Piper streaming behavior.
     *
     * @param[in,out] context
     * Non-owning provider context whose nullability is implementation-defined.
     *
     * @param[in] model
     * Piper model name retained only for this synchronous call.
     *
     * @param[in] text
     * Speech text retained only for this synchronous call.
     *
     * @param[in] stream
     * `true` for incremental playback; `false` for buffered playback.
     */
    using piperstreamspeakcallback = void (*)(contextpointer context,
                                              stringview model,
                                              stringview text,
                                              boolean stream);

    /**
     * @brief Acquires monotonic elapsed time for request measurement.
     *
     * @param[in,out] context
     * Non-owning clock context whose nullability is implementation-defined.
     *
     * @return
     * Monotonic time in seconds.
     */
    using piperstreamtimecallback = float64 (*)(contextpointer context);

    /**
     * @brief Reports one literal status line from the upstream script.
     *
     * @param[in,out] context
     * Non-owning output context whose nullability is implementation-defined.
     *
     * @param[in] message
     * Message view valid only during the callback.
     */
    using piperstreammessagecallback = void (*)(contextpointer context, stringview message);

    /**
     * @brief Reports one measured synthesis duration.
     *
     * @param[in,out] context
     * Non-owning output context whose nullability is implementation-defined.
     *
     * @param[in] durationSeconds
     * Non-negative elapsed synthesis duration in seconds.
     */
    using piperstreamdurationcallback = void (*)(contextpointer context, float64 durationSeconds);

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @brief Compares one streamed and one buffered Piper synthesis request.
     *
     * @details
     * Stores only non-owning callback context. The provider, clock, and reporting
     * implementations must outlive the sequence and remain synchronous.
     */
    class XWalkPiperStreamSequence
    {
        private:
            /** @brief Non-owning context forwarded to every callback. */
            contextpointer callbackContext;
            /** @brief Non-null provider operation supporting both stream modes. */
            piperstreamspeakcallback speakCallback;
            /** @brief Non-null monotonic time operation. */
            piperstreamtimecallback timeCallback;
            /** @brief Non-null literal-message reporting operation. */
            piperstreammessagecallback messageCallback;
            /** @brief Non-null duration reporting operation. */
            piperstreamdurationcallback durationCallback;

        public:
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
            XWalkPiperStreamSequence(contextpointer context,
                                     piperstreamspeakcallback speak,
                                     piperstreamtimecallback time,
                                     piperstreammessagecallback reportMessage,
                                     piperstreamdurationcallback reportDuration);

            /** @brief Prevents copying of callback bindings. */
            XWalkPiperStreamSequence(const XWalkPiperStreamSequence&) = delete;
            /** @brief Prevents moving of callback bindings. */
            XWalkPiperStreamSequence(XWalkPiperStreamSequence&&) = delete;
            /** @brief Prevents copy assignment of callback bindings. */
            XWalkPiperStreamSequence& operator=(const XWalkPiperStreamSequence&) = delete;
            /** @brief Prevents move assignment of callback bindings. */
            XWalkPiperStreamSequence& operator=(XWalkPiperStreamSequence&&) = delete;

            /**
             * @brief Runs streamed synthesis followed by buffered synthesis.
             *
             * @post
             * Both requests receive the same model and text, and each duration is
             * reported immediately after its corresponding request.
             *
             * @throws std::runtime_error
             * If the monotonic clock moves backwards during either request.
             *
             * @note
             * Exceptions from provider or reporting callbacks are propagated.
             */
            void run();
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_PIPER_STREAM_SEQUENCE_H */
