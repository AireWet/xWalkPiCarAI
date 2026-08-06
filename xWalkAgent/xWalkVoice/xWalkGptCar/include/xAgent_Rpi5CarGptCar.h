/******************************************************************************
 * @file        xAgent_Rpi5CarGptCar.h
 * @brief       Declares the upstream GPT PiCar-X assistant profile.
 *
 * @details
 * Adapts the gpt_examples assistant prompt, JSON response contract, optional
 * camera input, and keyboard selection onto the shared voice-car coordinator.
 *
 * @project     xWalk Firmware
 * @module      xWalkGptCar
 *
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_GPT_CAR_H
#define XAGENT_RPI5CAR_GPT_CAR_H

#include "xAgent_Rpi5CarVoiceActiveCar.h"

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/**
 * @class XWalkGptCar
 * @brief Supplies the gpt_examples profile and delegates its foreground loop.
 *
 * @details
 * Stores a non-owning pointer to a caller-created shared voice-car coordinator.
 * The referenced coordinator and all of its dependencies must outlive this object.
 */
class XWalkGptCar final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning shared coordinator pointer that is never null. */
        XWalkVoiceActiveCar* voiceCarObject{nullptr};

    public:
        /**************************************************************************
         * Public constants
         **************************************************************************/

        /** @brief Source-compatible OpenAI model used by the C++ provider. */
        static constexpr agent::cstring MODEL_NAME = "gpt-4o";
        /** @brief OpenAI-compatible endpoint used by the C++ composition. */
        static constexpr agent::cstring MODEL_ENDPOINT =
            "https://api.openai.com/v1/chat/completions";
        /** @brief Environment variable that exclusively supplies the credential. */
        static constexpr agent::cstring API_KEY_ENVIRONMENT = "OPENAI_API_KEY";
        /** @brief Source OpenAI text-to-speech voice retained as profile metadata. */
        static constexpr agent::cstring SPEECH_VOICE = "echo";

        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Binds the shared voice-car coordinator used for execution.
         * @param[in] voiceCar Coordinator that must outlive this profile adapter.
         */
        explicit XWalkGptCar(XWalkVoiceActiveCar& voiceCar) noexcept;

        /** @brief Destroys the adapter without stopping or releasing its dependency. */
        ~XWalkGptCar() = default;

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        XWalkGptCar(const XWalkGptCar&) = delete;
        XWalkGptCar(XWalkGptCar&&) = delete;
        XWalkGptCar& operator=(const XWalkGptCar&) = delete;
        XWalkGptCar& operator=(XWalkGptCar&&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /** @brief Returns the upstream assistant instructions without credentials. */
        static hal::XWalkVoiceAssistantConfiguration assistantConfiguration();

        /** @brief Returns JSON, image-enabled, voice-input source defaults. */
        static XWalkVoiceActiveCarConfiguration carConfiguration();

        /** @brief Selects keyboard input and optional image analysis for the next run. */
        void configure(agent::boolean keyboardInput, agent::boolean withImage) noexcept;

        /** @brief Runs assistant rounds until application cancellation. */
        agent::int32 run();

        /** @brief Stops assistant, action, LED, and vehicle activity. */
        void stop();
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_GPT_CAR_H */
