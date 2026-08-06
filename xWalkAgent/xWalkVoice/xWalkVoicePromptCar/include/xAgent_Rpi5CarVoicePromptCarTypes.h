/******************************************************************************
 * @file        xAgent_Rpi5CarVoicePromptCarTypes.h
 * @brief       Declares spoken movement-example callbacks and configuration.
 * @project     xWalk Firmware
 * @module      xWalkVoicePromptCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_PROMPT_CAR_TYPES_H
#define XAGENT_RPI5CAR_VOICE_PROMPT_CAR_TYPES_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::agent
{

using voicepromptcaroutputcallback = void (*)(agent::contextpointer,
    agent::stringview);
using voicepromptcarcontinuecallback = agent::boolean (*)(agent::contextpointer);
using voicepromptcardelaycallback = void (*)(agent::contextpointer, agent::uint32);

/** @brief Stores the complete synchronous application callback boundary. */
struct XWalkVoicePromptCarCallbacks
{
    voicepromptcaroutputcallback output{nullptr};
    voicepromptcarcontinuecallback shouldContinue{nullptr};
    voicepromptcardelaycallback delay{nullptr};
};

/** @brief Stores source-compatible movement values for example 14. */
struct XWalkVoicePromptCarConfiguration
{
    agent::float64 speedPercent{30.0};
    agent::float64 steeringAngle{20.0};
    agent::uint32 driveDurationMs{2'000U};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_PROMPT_CAR_TYPES_H */
