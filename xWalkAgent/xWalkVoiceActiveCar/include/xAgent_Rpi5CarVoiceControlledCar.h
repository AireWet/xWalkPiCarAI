/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceControlledCar.h
 * @brief       Declares the wake-word voice-controlled PiCar-X coordinator.
 *
 * @details
 * Coordinates caller-owned vehicle and speech-recognition services while an
 * application supplies synchronous output, cancellation, and delay callbacks.
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCar
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_CONTROLLED_CAR_H
#define XAGENT_RPI5CAR_VOICE_CONTROLLED_CAR_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarVoiceActiveCarTypes.h"
#include "xHal_Rpi5CarSpeechToText.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains application coordinators for xWalk firmware. */
namespace xwalk::agent
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkVoiceControlledCar
 * @brief Ports the wake, command, and sleep loop from `16.voice_controlled_car.py`.
 *
 * @details
 * Stores non-owning pointers to caller-owned PiCar-X and speech-recognition
 * objects. Calls are synchronous and require external serialization.
 */
class XWalkVoiceControlledCar final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning vehicle coordinator pointer. */
        XWalkPicarx* picarxObject{nullptr};
        /** @brief Non-owning speech-recognition coordinator pointer. */
        hal::XWalkSpeechToText* speechToTextObject{nullptr};
        /** @brief Nullable context forwarded to application callbacks. */
        hal::contextpointer callbackContext{nullptr};
        /** @brief Complete synchronous application callback table. */
        XWalkVoiceActiveCarCallbacks callbacks{};
        /** @brief Owned and validated recognition and movement configuration. */
        XWalkVoiceControlledCarConfiguration configuration{};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Executes one recognized movement or sleep request.
         * @param[in] command Supported command to apply synchronously.
         */
        void execute(XWalkVoiceControlledCarCommand command);
        /**
         * @brief Returns a lowercase ASCII copy for phrase matching.
         * @param[in] text Transcript or phrase to normalize.
         * @return Owned lowercase copy of `text`.
         */
        static hal::string lowercase(hal::stringview text);
        /**
         * @brief Validates required callbacks and bounded configuration values.
         * @param[in] backendCallbacks Callback table requiring output, continuation, and delay.
         * @param[in] carConfiguration Recognition and movement configuration to validate.
         * @throws std::invalid_argument If a callback or phrase is missing.
         * @throws std::out_of_range If a numeric value is outside its documented range.
         */
        static void validate(const XWalkVoiceActiveCarCallbacks& backendCallbacks,
            const XWalkVoiceControlledCarConfiguration& carConfiguration);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Binds caller-owned vehicle, recognition, and callback services.
         * @param[in,out] picarx Vehicle coordinator that must outlive this object.
         * @param[in,out] speechToText Recognition coordinator that must outlive this object.
         * @param[in,out] context Nullable callback context that must outlive callback use.
         * @param[in] backendCallbacks Complete synchronous application callbacks.
         * @param[in] carConfiguration Owned wake, sleep, timing, speed, and steering settings.
         */
        XWalkVoiceControlledCar(XWalkPicarx& picarx,
            hal::XWalkSpeechToText& speechToText, hal::contextpointer context,
            const XWalkVoiceActiveCarCallbacks& backendCallbacks,
            const XWalkVoiceControlledCarConfiguration& carConfiguration = {});
        /** @brief Releases no caller-owned vehicle, recognition, or callback service. */
        ~XWalkVoiceControlledCar() = default;

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables moving because dependency identity is retained. */
        XWalkVoiceControlledCar(XWalkVoiceControlledCar&&) = delete;
        /** @brief Disables copying of non-owning service bindings. */
        XWalkVoiceControlledCar(const XWalkVoiceControlledCar&) = delete;
        /** @brief Disables move assignment because dependency identity is retained. */
        XWalkVoiceControlledCar& operator=(XWalkVoiceControlledCar&&) = delete;
        /** @brief Disables copy assignment of non-owning service bindings. */
        XWalkVoiceControlledCar& operator=(const XWalkVoiceControlledCar&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Runs wake-word and movement recognition until cancellation.
         * @return Zero after cancellation and safe vehicle shutdown.
         */
        hal::int32 run();
        /** @brief Requests recognition shutdown and leaves the vehicle safe. */
        void stop();
        /**
         * @brief Classifies one case-insensitive transcript by supported keywords.
         * @param[in] transcript Recognized text to inspect.
         * @param[in] sleepWord Non-empty session-closing phrase.
         * @return The first supported command by defined priority, or `Unknown`.
         * @throws std::invalid_argument If `sleepWord` is empty.
         */
        static XWalkVoiceControlledCarCommand classifyCommand(hal::stringview transcript,
            hal::stringview sleepWord = "sleep");
        /**
         * @brief Reports whether a transcript contains the configured wake phrase.
         * @param[in] transcript Recognized text to inspect.
         * @param[in] wakeWord Non-empty wake phrase to find.
         * @return `true` when the phrase occurs case-insensitively; otherwise `false`.
         * @throws std::invalid_argument If `wakeWord` is empty.
         */
        static hal::boolean containsWakeWord(hal::stringview transcript,
            hal::stringview wakeWord = "hey robot");
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_CONTROLLED_CAR_H */
