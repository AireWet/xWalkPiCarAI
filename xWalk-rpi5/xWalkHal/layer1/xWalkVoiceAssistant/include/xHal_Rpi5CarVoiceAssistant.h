/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistant.h
 * @brief       Declares the xWalk synchronous voice-assistant coordinator.
 *
 * @details
 * Composes caller-created speech recognition, language-model, and speech-output
 * objects into deterministic single-round operations with optional hooks.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant
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

#ifndef XHAL_RPI5CAR_VOICE_ASSISTANT_H
#define XHAL_RPI5CAR_VOICE_ASSISTANT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModel.h"
#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarTextToSpeech.h"
#include "xHal_Rpi5CarVoiceAssistantTypes.h"

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
 * @class XWalkVoiceAssistant
 * @brief Coordinates one synchronous listen, model, and speech-response pipeline.
 *
 * @details
 * Stores non-owning pointers to caller-created speech-to-text, language-model,
 * and text-to-speech objects. It owns only configuration and running state. Wake
 * detection, triggers, keyboard input, image capture, threads, and scheduling
 * remain responsibilities of the application composition layer.
 */
class XWalkVoiceAssistant final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Non-owning pointer to the required speech-recognition object.
         *
         * @note
         * Never null; the pointed-to caller-owned object must outlive this coordinator.
         */
        XWalkSpeechToText* speechToTextPointer;

        /**
         * @brief Non-owning pointer to the required language-model object.
         *
         * @note
         * Never null; the pointed-to caller-owned object must outlive this coordinator.
         */
        XWalkLanguageModel* languageModelPointer;

        /**
         * @brief Non-owning pointer to the required speech-output object.
         *
         * @note
         * Never null; the pointed-to caller-owned object must outlive this coordinator.
         */
        XWalkTextToSpeech* textToSpeechPointer;

        /**
         * @brief Nullable non-owning context supplied to optional lifecycle callbacks.
         *
         * @note
         * Any non-null object must outlive this coordinator and every callback invocation.
         */
        contextpointer callbackContextPointer;

        /** @brief Optional callback table copied during construction. */
        XWalkVoiceAssistantCallbacks callbacks;

        /** @brief Owned startup text copied during construction. */
        XWalkVoiceAssistantConfiguration configuration;

        /** @brief `true` between successful `start()` and `stop()` operations. */
        boolean runningValue;

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Rejects an operation that requires a started assistant.
         *
         * @throws std::logic_error
         * If the assistant is not running.
         */
        void requireRunning() const;

        /**
         * @brief Invokes an optional event callback.
         *
         * @param[in] callback
         * Nullable callback; null produces no operation.
         */
        void invokeEvent(voiceassistanteventcallback callback) const;

        /**
         * @brief Invokes an optional text callback.
         *
         * @param[in] callback
         * Nullable callback; null produces no operation.
         *
         * @param[in] text
         * Text view valid for the synchronous callback duration.
         */
        void invokeText(voiceassistanttextcallback callback, stringview text) const;

        /**
         * @brief Applies the optional response parser.
         *
         * @param[in] response
         * Final language-model response.
         *
         * @return
         * Parsed owned response, or an unmodified copy when no parser exists.
         */
        string parseResponse(stringview response) const;

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a coordinator from caller-owned pipeline components.
         *
         * @param[in,out] speechToText
         * Speech-recognition object that must outlive this coordinator.
         *
         * @param[in,out] languageModel
         * Language-model object that must outlive this coordinator.
         *
         * @param[in,out] textToSpeech
         * Speech-output object that must outlive this coordinator.
         *
         * @param[in] assistantConfiguration
         * Startup instructions and optional welcome text copied by value.
         *
         * @param[in,out] callbackContext
         * Nullable non-owning context used by optional callbacks.
         *
         * @param[in] assistantCallbacks
         * Optional lifecycle and response-parser callbacks copied by value.
         *
         * @post
         * The language-model backend has received the configured instructions.
         */
        XWalkVoiceAssistant(XWalkSpeechToText& speechToText, XWalkLanguageModel& languageModel,
            XWalkTextToSpeech& textToSpeech,
            const XWalkVoiceAssistantConfiguration& assistantConfiguration = {},
            contextpointer callbackContext = nullptr,
            const XWalkVoiceAssistantCallbacks& assistantCallbacks = {});

        /**
         * @brief Stops a running assistant without owning its dependencies.
         *
         * @warning
         * Stop operations invoked during destruction must not throw.
         */
        ~XWalkVoiceAssistant();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables copying of non-owning pipeline bindings. */
        XWalkVoiceAssistant(const XWalkVoiceAssistant&) = delete;
        /** @brief Disables copy assignment of non-owning pipeline bindings. */
        XWalkVoiceAssistant& operator=(const XWalkVoiceAssistant&) = delete;
        /** @brief Disables moving because dependency identities are retained. */
        XWalkVoiceAssistant(XWalkVoiceAssistant&&) = delete;
        /** @brief Disables move assignment because dependency identities are retained. */
        XWalkVoiceAssistant& operator=(XWalkVoiceAssistant&&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Starts the assistant and optionally speaks its welcome text.
         *
         * @post
         * `isRunning()` returns `true` after successful completion.
         *
         * @note
         * Calling `start()` while running has no effect.
         */
        void start();

        /**
         * @brief Stops recognition and reports the assistant stop event.
         *
         * @post
         * `isRunning()` returns `false` before backend cancellation is requested.
         *
         * @note
         * Calling `stop()` while already stopped has no effect.
         */
        void stop();

        /**
         * @brief Reports whether the assistant has been started and not stopped.
         *
         * @return
         * `true` while the coordinator accepts round operations; otherwise `false`.
         */
        boolean isRunning() const;

        /**
         * @brief Acquires one final speech-recognition result.
         *
         * @param[in] timeoutMs
         * Non-zero recognition interval in milliseconds accepted by `XWalkSpeechToText`.
         *
         * @return
         * Owned recognized text, including an empty result for silence.
         *
         * @throws std::logic_error
         * If the assistant is not running.
         *
         * @throws std::runtime_error
         * If the speech-recognition backend is not ready.
         */
        string listen(uint32 timeoutMs = XHAL_RPI5CAR_SPEECH_TO_TEXT_DEFAULT_TIMEOUT_MS);

        /**
         * @brief Generates one final language-model response.
         *
         * @param[in] text
         * Input text forwarded without encoding changes.
         *
         * @param[in] imagePath
         * Optional previously captured image path; empty means text-only input.
         *
         * @return
         * Owned final language-model response.
         *
         * @throws std::logic_error
         * If the assistant is not running.
         */
        string think(stringview text, stringview imagePath = {});

        /**
         * @brief Synthesizes one non-empty response with lifecycle callbacks.
         *
         * @param[in] text
         * Response text; an empty view intentionally produces no speech operation.
         *
         * @throws std::logic_error
         * If the assistant is not running.
         */
        void say(stringview text);

        /**
         * @brief Processes caller-supplied text through model, parser, and speech output.
         *
         * @param[in] text
         * Trigger text, such as validated keyboard or application input.
         *
         * @param[in] imagePath
         * Optional previously captured image path; empty means text-only input.
         *
         * @return
         * Parsed owned response. An empty response is returned without speech output.
         *
         * @throws std::logic_error
         * If the assistant is not running.
         */
        string processText(stringview text, stringview imagePath = {});

        /**
         * @brief Executes one microphone-to-speech assistant round.
         *
         * @param[in] timeoutMs
         * Non-zero recognition interval in milliseconds accepted by `XWalkSpeechToText`.
         *
         * @param[in] imagePath
         * Optional previously captured image path; empty means text-only input.
         *
         * @return
         * Parsed owned response, or an empty string when no speech is recognized.
         *
         * @throws std::logic_error
         * If the assistant is not running.
         */
        string runRound(uint32 timeoutMs = XHAL_RPI5CAR_SPEECH_TO_TEXT_DEFAULT_TIMEOUT_MS,
            stringview imagePath = {});
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_VOICE_ASSISTANT_H */
