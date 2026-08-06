/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceControlledCar.h
 * @brief       Declares the wake-word voice-controlled PiCar-X coordinator.
 * @project     xWalk Firmware
 * @module      xWalkVoiceControlledCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_CONTROLLED_CAR_H
#define XAGENT_RPI5CAR_VOICE_CONTROLLED_CAR_H

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarVoiceControlledCarTypes.h"
#include "xHal_Rpi5CarSpeechToText.h"

namespace xwalk::agent
{

/** @brief Ports the wake, command, and sleep loop from example 16. */
class XWalkVoiceControlledCar final
{
    private:
        XWalkPicarx* picarxObject{nullptr}; /**< Non-owning vehicle pointer. */
        hal::XWalkSpeechToText* speechToTextObject{nullptr}; /**< Non-owning STT pointer. */
        agent::contextpointer callbackContext{nullptr}; /**< Callback context. */
        XWalkVoiceControlledCarCallbacks callbacks{}; /**< Application callbacks. */
        XWalkVoiceControlledCarConfiguration configuration{}; /**< Owned settings. */

    protected:
        void execute(XWalkVoiceControlledCarCommand command);
        void waitForMovement();
        static agent::string normalize(agent::stringview text);
        static void validate(const XWalkVoiceControlledCarCallbacks& backendCallbacks,
            const XWalkVoiceControlledCarConfiguration& carConfiguration);

    public:
        XWalkVoiceControlledCar(XWalkPicarx& picarx,
            hal::XWalkSpeechToText& speechToText, agent::contextpointer context,
            const XWalkVoiceControlledCarCallbacks& backendCallbacks,
            const XWalkVoiceControlledCarConfiguration& carConfiguration = {});
        ~XWalkVoiceControlledCar() = default;

        XWalkVoiceControlledCar(XWalkVoiceControlledCar&&) = delete;
        XWalkVoiceControlledCar(const XWalkVoiceControlledCar&) = delete;
        XWalkVoiceControlledCar& operator=(XWalkVoiceControlledCar&&) = delete;
        XWalkVoiceControlledCar& operator=(const XWalkVoiceControlledCar&) = delete;

        agent::int32 run();
        void stop();
        static XWalkVoiceControlledCarCommand classifyCommand(
            agent::stringview transcript, agent::stringview sleepWord = "sleep");
        static agent::boolean containsWakeWord(agent::stringview transcript,
            agent::stringview wakeWord = "hey robot");
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_CONTROLLED_CAR_H */
