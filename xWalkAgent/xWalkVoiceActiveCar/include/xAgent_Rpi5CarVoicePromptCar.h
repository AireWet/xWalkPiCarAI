/******************************************************************************
 * @file        xAgent_Rpi5CarVoicePromptCar.h
 * @brief       Declares the spoken PiCar-X movement demonstration.
 *
 * @details
 * Coordinates caller-owned vehicle and speech-synthesis services to reproduce
 * the upstream four-movement spoken demonstration.
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

#ifndef XAGENT_RPI5CAR_VOICE_PROMPT_CAR_H
#define XAGENT_RPI5CAR_VOICE_PROMPT_CAR_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarVoiceActiveCarTypes.h"
#include "xHal_Rpi5CarTextToSpeech.h"

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
 * @class XWalkVoicePromptCar
 * @brief Ports the spoken movement sequence from `14.voice_promt_car.py`.
 *
 * @details
 * Stores non-owning pointers to caller-owned PiCar-X and text-to-speech
 * objects. Calls are synchronous and require external serialization.
 */
class XWalkVoicePromptCar final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning vehicle coordinator pointer. */
        XWalkPicarx* picarxObject{nullptr};
        /** @brief Non-owning speech-synthesis coordinator pointer. */
        hal::XWalkTextToSpeech* textToSpeechObject{nullptr};
        /** @brief Nullable context forwarded to application callbacks. */
        hal::contextpointer callbackContext{nullptr};
        /** @brief Complete synchronous application callback table. */
        XWalkVoiceActiveCarCallbacks callbacks{};
        /** @brief Owned and validated movement configuration. */
        XWalkVoicePromptCarConfiguration configuration{};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Speaks and executes one forward or backward movement.
         * @param[in] prompt Text spoken before movement.
         * @param[in] forward `true` for forward movement; `false` for backward.
         */
        void drive(hal::stringview prompt, hal::boolean forward);
        /**
         * @brief Speaks and executes one steered forward movement.
         * @param[in] prompt Text spoken before movement.
         * @param[in] angle Signed steering angle in degrees.
         */
        void turn(hal::stringview prompt, hal::float64 angle);
        /**
         * @brief Validates required callbacks and bounded configuration values.
         * @param[in] backendCallbacks Callback table requiring output, continuation, and delay.
         * @param[in] carConfiguration Movement configuration to validate.
         * @throws std::invalid_argument If a required callback is null.
         * @throws std::out_of_range If a numeric value is outside its documented range.
         */
        static void validate(const XWalkVoiceActiveCarCallbacks& backendCallbacks,
            const XWalkVoicePromptCarConfiguration& carConfiguration);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Binds caller-owned vehicle, speech, and callback services.
         * @param[in,out] picarx Vehicle coordinator that must outlive this object.
         * @param[in,out] textToSpeech Speech coordinator that must outlive this object.
         * @param[in,out] context Nullable callback context that must outlive callback use.
         * @param[in] backendCallbacks Complete synchronous application callbacks.
         * @param[in] carConfiguration Owned timing, speed, and steering settings.
         */
        XWalkVoicePromptCar(XWalkPicarx& picarx,
            hal::XWalkTextToSpeech& textToSpeech, hal::contextpointer context,
            const XWalkVoiceActiveCarCallbacks& backendCallbacks,
            const XWalkVoicePromptCarConfiguration& carConfiguration = {});
        /** @brief Releases no caller-owned vehicle, speech, or callback service. */
        ~XWalkVoicePromptCar() = default;

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables moving because dependency identity is retained. */
        XWalkVoicePromptCar(XWalkVoicePromptCar&&) = delete;
        /** @brief Disables copying of non-owning service bindings. */
        XWalkVoicePromptCar(const XWalkVoicePromptCar&) = delete;
        /** @brief Disables move assignment because dependency identity is retained. */
        XWalkVoicePromptCar& operator=(XWalkVoicePromptCar&&) = delete;
        /** @brief Disables copy assignment of non-owning service bindings. */
        XWalkVoicePromptCar& operator=(const XWalkVoicePromptCar&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Runs the four spoken movement demonstrations once.
         * @return Zero after completion or cancellation and safe vehicle shutdown.
         */
        hal::int32 run();
        /** @brief Stops the vehicle and centres steering. */
        void stop();
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_PROMPT_CAR_H */
