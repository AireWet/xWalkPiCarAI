/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceControlledCarTypes.h
 * @brief       Declares wake-word voice-control callbacks and configuration.
 * @project     xWalk Firmware
 * @module      xWalkVoiceControlledCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_CONTROLLED_CAR_TYPES_H
#define XAGENT_RPI5CAR_VOICE_CONTROLLED_CAR_TYPES_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::agent
{

    using voicecontrolledcaroutputcallback = void (*)(agent::contextpointer, agent::stringview);
    using voicecontrolledcarcontinuecallback = agent::boolean (*)(agent::contextpointer);
    using voicecontrolledcardelaycallback = void (*)(agent::contextpointer, agent::uint32);

    /** @brief Stores the complete synchronous application callback boundary. */
    struct XWalkVoiceControlledCarCallbacks
    {
            voicecontrolledcaroutputcallback output{nullptr};
            voicecontrolledcarcontinuecallback shouldContinue{nullptr};
            voicecontrolledcardelaycallback delay{nullptr};
    };

    /** @brief Identifies one supported spoken command from example 16. */
    enum class XWalkVoiceControlledCarCommand : agent::uint8
    {
        Unknown = 0U,
        Forward,
        Backward,
        Left,
        Right,
        Sleep
    };

    /** @brief Stores source-compatible recognition and movement values. */
    struct XWalkVoiceControlledCarConfiguration
    {
            agent::string wakeWord{"hey robot"};
            agent::string sleepWord{"sleep"};
            agent::float64 speedPercent{30.0};
            agent::float64 steeringAngle{25.0};
            agent::uint32 driveDurationMs{1'000U};
            agent::uint32 listenTimeoutMs{5'000U};
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_CONTROLLED_CAR_TYPES_H */
