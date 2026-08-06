/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCar.h
 * @brief       Declares the sensor-aware voice-active PiCar-X coordinator.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCar
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_H
#define XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_H

#include "xAgent_Rpi5CarSelfDrive.h"
#include "xAgent_Rpi5CarVoiceActiveCarTypes.h"
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarVoiceAssistant.h"

namespace xwalk::agent
{

class XWalkVoiceActiveCar
{
    public:
        /** @brief Source robot name from `voice_active_car.py`. */
        static constexpr agent::cstring NAME = "Rolly";
        /** @brief Source speech-recognition language. */
        static constexpr agent::cstring SPEECH_LANGUAGE = "en-us";
        /** @brief Source keyboard-input feature selection. */
        static constexpr agent::boolean KEYBOARD_ENABLED = true;
        /** @brief Source OpenAI language-model name. */
        static constexpr agent::cstring MODEL_NAME = "gpt-4o-mini";
        /** @brief Source-compatible OpenAI Chat Completions endpoint. */
        static constexpr agent::cstring MODEL_ENDPOINT =
            "https://api.openai.com/v1/chat/completions";
        /** @brief Environment variable that exclusively supplies the credential. */
        static constexpr agent::cstring API_KEY_ENVIRONMENT = "OPENAI_API_KEY";
        /** @brief Source case-insensitive wake phrase. */
        static constexpr agent::cstring WAKE_WORD = "hey rolly";
        /** @brief Source response spoken after wake detection. */
        static constexpr agent::cstring ANSWER_ON_WAKE = "Hi there";

    private:
        XWalkPicarx* picarxObject{nullptr};
        XWalkSelfDrive* selfDriveObject{nullptr};
        hal::XWalkVoiceAssistant* assistantObject{nullptr};
        hal::XWalkLed* ledObject{nullptr};
        agent::contextpointer callbackContext{nullptr};
        XWalkVoiceActiveCarCallbacks callbacks{};
        XWalkVoiceActiveCarConfiguration configuration{};
        /** @brief True after wake recognition until one ordinary model round completes. */
        agent::boolean wakeDetectedValue{};

    protected:
        static void validate(const XWalkVoiceActiveCarCallbacks& backendCallbacks,
            const XWalkVoiceActiveCarConfiguration& carConfiguration);
        void blink(agent::uint32 count, agent::uint32 toggleDelayMs,
            agent::uint32 pauseMs);
        void dispatchActions(const agent::stringvector& actions);
        XWalkVoiceActiveCarResponse parseConfiguredResponse(
            agent::stringview response) const;
        agent::string sensorPrompt();
        /**
         * @brief Checks one recognition result for the configured wake phrase.
         * @param[in] text Recognized text retained only for this call.
         * @return `true` when `text` contains the wake phrase without case sensitivity.
         */
        agent::boolean isWakePhrase(agent::stringview text) const;

    public:
        XWalkVoiceActiveCar(XWalkPicarx& picarx, XWalkSelfDrive& selfDrive,
            hal::XWalkVoiceAssistant& assistant, hal::XWalkLed& led,
            agent::contextpointer context,
            const XWalkVoiceActiveCarCallbacks& backendCallbacks,
            const XWalkVoiceActiveCarConfiguration& carConfiguration = {});
        ~XWalkVoiceActiveCar() = default;

        XWalkVoiceActiveCar(XWalkVoiceActiveCar&&) = delete;
        XWalkVoiceActiveCar(const XWalkVoiceActiveCar&) = delete;
        XWalkVoiceActiveCar& operator=(XWalkVoiceActiveCar&&) = delete;
        XWalkVoiceActiveCar& operator=(const XWalkVoiceActiveCar&) = delete;

        /**
         * @brief Returns the complete Rolly instructions and welcome message.
         * @return Owned source-compatible assistant configuration.
         */
        static hal::XWalkVoiceAssistantConfiguration assistantConfiguration();

        /**
         * @brief Returns source sensing, image, recognition, and wake settings.
         * @return Ten-centimetre, image-enabled, English Rolly configuration.
         */
        static XWalkVoiceActiveCarConfiguration carConfiguration();

        /**
         * @brief Runs sensor-aware voice rounds until cancellation.
         * @return Zero after normal cancellation; one after action-worker failure.
         * @note Wake-enabled profiles require their phrase before each ordinary model round.
         */
        agent::int32 run();
        /** @brief Stops voice, action, vehicle, and LED activity. */
        void stop();
        /** @brief Parses `RESPONSE_TEXT\nACTIONS: ...` model output. */
        static XWalkVoiceActiveCarResponse parseResponse(agent::stringview response);

        /**
         * @brief Parses the JSON object returned by the upstream GPT-car assistant.
         * @param[in] response Model response containing `actions` and `answer` values.
         * @return Owned answer and action names; unrecognized text is retained as the answer.
         */
        static XWalkVoiceActiveCarResponse parseJsonResponse(agent::stringview response);

        /**
         * @brief Selects voice or keyboard input before starting the foreground loop.
         * @param[in] inputMode Input source used by later rounds.
         */
        void setInputMode(XWalkVoiceActiveCarInputMode inputMode) noexcept;

        /**
         * @brief Enables or disables image attachment before starting the foreground loop.
         * @param[in] enabled `true` to request a captured still image for ordinary prompts.
         */
        void setImageEnabled(agent::boolean enabled) noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_H */
