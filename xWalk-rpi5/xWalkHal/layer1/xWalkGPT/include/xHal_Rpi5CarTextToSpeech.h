/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeech.h
 * @brief       Declares the Robot HAT text-to-speech coordinator.
 *
 * @details
 * Activates caller-owned Robot HAT speaker control during construction and
 * forwards requested text to a caller-selected speech backend.
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

#ifndef XHAL_RPI5CAR_TEXT_TO_SPEECH_H
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarTextToSpeechTypes.h"

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
     * @class XWalkTextToSpeech
     * @brief Coordinates speaker activation and an injected speech backend.
     *
     * @details
     * Represents the common Robot HAT behavior added by the Piper, Pico2Wave,
     * Espeak, OpenAI TTS, and EdgeTTS Python wrappers. The class stores non-owning
     * pointers to the caller-created board controller and backend context, enables
     * speaker power during construction, and forwards speech requests synchronously.
     * It does not own a model, audio resource, network client, or worker thread.
     */
    class XWalkTextToSpeech final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Non-owning pointer to the Robot HAT board controller.
             *
             * @note
             * Never null. The controller and its hardware dependencies must outlive
             * this object. Speaker power is activated through it during construction.
             */
            XWalkBoardControl* boardControlPointer;

            /**
             * @brief Nullable non-owning context supplied to the speech backend.
             *
             * @note
             * Null is permitted only when `speakCallback` supports it. Any non-null
             * object must outlive this object and every callback invocation.
             */
            contextpointer backendContextPointer;

            /**
             * @brief Non-null synchronous speech callback supplied by the application.
             *
             * @note
             * The function is stored by value and is never owned or modified.
             */
            texttospeechspeakcallback speakCallback;

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Validates the injected speech backend before speaker activation.
             *
             * @param[in] callback
             * Speech callback that must be non-null.
             *
             * @throws std::invalid_argument
             * If `callback` is null.
             */
            static void validateBackend(texttospeechspeakcallback callback);

            /**
             * @brief Activates and primes Robot HAT speaker output.
             *
             * @post
             * Speaker power remains active after successful completion.
             *
             * @note
             * Exceptions from board control are propagated and construction fails.
             */
            void prepareSpeaker();

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a text-to-speech coordinator and activates the speaker.
             *
             * @param[in,out] boardControl
             * Caller-created Robot HAT controller that must outlive this object.
             *
             * @param[in,out] backendContext
             * Nullable non-owning speech-backend context. A non-null object must
             * outlive this object, and null requires explicit callback support.
             *
             * @param[in] backendSpeak
             * Non-null synchronous callback that accepts speech text.
             *
             * @post
             * Robot HAT speaker power has been enabled and primed.
             *
             * @throws std::invalid_argument
             * If `backendSpeak` is null.
             *
             * @note
             * Exceptions from speaker activation or priming are propagated.
             */
            XWalkTextToSpeech(XWalkBoardControl& boardControl,
                              contextpointer backendContext,
                              texttospeechspeakcallback backendSpeak);

            /**
             * @brief Destroys the coordinator without disabling shared speaker power.
             *
             * @details
             * This preserves the Python wrapper behavior and avoids changing a
             * board-level resource that may be shared with another audio component.
             */
            ~XWalkTextToSpeech();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables copying of non-owning hardware and backend bindings. */
            XWalkTextToSpeech(const XWalkTextToSpeech&) = delete;
            /** @brief Disables copy assignment of non-owning bindings. */
            XWalkTextToSpeech& operator=(const XWalkTextToSpeech&) = delete;
            /** @brief Disables moving because callback context identity is retained. */
            XWalkTextToSpeech(XWalkTextToSpeech&&) = delete;
            /** @brief Disables move assignment because dependency identity is retained. */
            XWalkTextToSpeech& operator=(XWalkTextToSpeech&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Synthesizes and outputs one text value through the backend.
             *
             * @param[in] text
             * Text view forwarded synchronously without modification. The backend
             * defines supported encoding, language, length, and empty-text behavior.
             *
             * @pre
             * The Robot HAT speaker was enabled successfully during construction.
             *
             * @note
             * Any exception raised by the injected callback is propagated.
             */
            void speak(stringview text);
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_TEXT_TO_SPEECH_H */
