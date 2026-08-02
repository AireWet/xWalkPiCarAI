/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToText.h
 * @brief       Declares the xWalk speech-recognition coordinator.
 *
 * @details
 * Provides bounded microphone recognition and audio-file transcription through
 * a complete callback table owned and implemented by the application.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT
 *
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_SPEECH_TO_TEXT_H
#define XHAL_RPI5CAR_SPEECH_TO_TEXT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToTextTypes.h"

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
 * @class XWalkSpeechToText
 * @brief Coordinates synchronous speech recognition through an injected backend.
 *
 * @details
 * Represents the non-streaming Robot HAT STT behavior without owning a
 * microphone, model, process, filesystem object, network client, or worker.
 * The caller-owned backend context must remain valid throughout this object's
 * lifetime. Calls require external serialization.
 */
class XWalkSpeechToText final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Nullable non-owning speech-recognition backend context.
         *
         * @note
         * Null is permitted only when every callback supports it. Any non-null
         * object must outlive this coordinator and every callback invocation.
         */
        contextpointer backendContextPointer;

        /**
         * @brief Complete backend callback table copied during construction.
         *
         * @note
         * The callbacks are non-owning function pointers and are never replaced.
         */
        XWalkSpeechToTextCallbacks callbacks;

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Validates that every required backend callback is non-null.
         *
         * @param[in] backendCallbacks
         * Callback table to validate before storing or invoking it.
         *
         * @throws std::invalid_argument
         * If any callback is null.
         */
        static void validateCallbacks(const XWalkSpeechToTextCallbacks& backendCallbacks);

        /**
         * @brief Validates one bounded microphone-recognition interval.
         *
         * @param[in] timeoutMs
         * Requested interval in milliseconds.
         *
         * @throws std::out_of_range
         * If the interval is zero or greater than the configured maximum.
         */
        static void validateTimeout(uint32 timeoutMs);

        /**
         * @brief Validates one audio-file path before backend dispatch.
         *
         * @param[in] filePath
         * Path view that must contain at least one character.
         *
         * @throws std::invalid_argument
         * If `filePath` is empty.
         */
        static void validateFilePath(stringview filePath);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a speech coordinator from a complete backend table.
         *
         * @param[in,out] context
         * Nullable non-owning backend context. A non-null object must outlive
         * this coordinator, and null requires explicit support from all callbacks.
         *
         * @param[in] backendCallbacks
         * Complete callback table copied into this coordinator.
         *
         * @throws std::invalid_argument
         * If any required callback is null.
         */
        XWalkSpeechToText(contextpointer context,
            const XWalkSpeechToTextCallbacks& backendCallbacks);

        /**
         * @brief Requests recognition stop and releases no backend ownership.
         *
         * @warning
         * The stop callback must not throw because the destructor cannot report
         * backend shutdown failure.
         */
        ~XWalkSpeechToText();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables copying of the non-owning backend binding. */
        XWalkSpeechToText(const XWalkSpeechToText&) = delete;
        /** @brief Disables copy assignment of the non-owning backend binding. */
        XWalkSpeechToText& operator=(const XWalkSpeechToText&) = delete;
        /** @brief Disables moving because backend context identity is retained. */
        XWalkSpeechToText(XWalkSpeechToText&&) = delete;
        /** @brief Disables move assignment because backend context identity is retained. */
        XWalkSpeechToText& operator=(XWalkSpeechToText&&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Reports whether the backend can begin recognition.
         *
         * @return
         * `true` when the backend reports readiness; otherwise `false`.
         *
         * @note
         * Any exception raised by the injected callback is propagated.
         */
        boolean isReady() const;

        /**
         * @brief Records microphone input and returns final recognized text.
         *
         * @param[in] timeoutMs
         * Non-zero maximum recognition interval in milliseconds, no greater
         * than `XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS`.
         *
         * @return
         * Owned recognized text, or an empty string when no speech is recognized.
         *
         * @throws std::out_of_range
         * If `timeoutMs` is outside the supported range.
         *
         * @note
         * Any exception raised by the injected callback is propagated.
         */
        string listen(uint32 timeoutMs = XHAL_RPI5CAR_SPEECH_TO_TEXT_DEFAULT_TIMEOUT_MS);

        /**
         * @brief Transcribes one backend-supported audio file.
         *
         * @param[in] filePath
         * Non-empty path forwarded synchronously without filesystem inspection.
         *
         * @return
         * Owned recognized text, or an empty string when no speech is recognized.
         *
         * @throws std::invalid_argument
         * If `filePath` is empty.
         *
         * @note
         * Backend file, format, model, and exception behavior is propagated.
         */
        string transcribeFile(stringview filePath);

        /**
         * @brief Requests cancellation of active recognition.
         *
         * @post
         * The stop request has been delivered synchronously to the backend.
         *
         * @note
         * Any exception raised by the injected callback is propagated.
         */
        void stop();
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEECH_TO_TEXT_H */
