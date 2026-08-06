/******************************************************************************
 * @file        xAgent_Rpi5CarVoicePromptCar.h
 * @brief       Declares the spoken PiCar-X movement demonstration.
 * @project     xWalk Firmware
 * @module      xWalkVoicePromptCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_PROMPT_CAR_H
#define XAGENT_RPI5CAR_VOICE_PROMPT_CAR_H

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarVoicePromptCarTypes.h"
#include "xHal_Rpi5CarTextToSpeech.h"

namespace xwalk::agent
{

/** @brief Ports the spoken movement sequence from `14.voice_promt_car.py`. */
class XWalkVoicePromptCar final
{
private:
    XWalkPicarx* picarxObject{nullptr};
    hal::XWalkTextToSpeech* textToSpeechObject{nullptr};
    agent::contextpointer callbackContext{nullptr};
    XWalkVoicePromptCarCallbacks callbacks{};
    XWalkVoicePromptCarConfiguration configuration{};

protected:
    void drive(agent::stringview prompt, agent::boolean forward);
    void turn(agent::stringview prompt, agent::float64 angle);
    static void validate(const XWalkVoicePromptCarCallbacks& backendCallbacks,
        const XWalkVoicePromptCarConfiguration& carConfiguration);

public:
    XWalkVoicePromptCar(XWalkPicarx& picarx,
        hal::XWalkTextToSpeech& textToSpeech, agent::contextpointer context,
        const XWalkVoicePromptCarCallbacks& backendCallbacks,
        const XWalkVoicePromptCarConfiguration& carConfiguration = {});
    ~XWalkVoicePromptCar() = default;

    XWalkVoicePromptCar(XWalkVoicePromptCar&&) = delete;
    XWalkVoicePromptCar(const XWalkVoicePromptCar&) = delete;
    XWalkVoicePromptCar& operator=(XWalkVoicePromptCar&&) = delete;
    XWalkVoicePromptCar& operator=(const XWalkVoicePromptCar&) = delete;

    agent::int32 run();
    void stop();
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_PROMPT_CAR_H */
