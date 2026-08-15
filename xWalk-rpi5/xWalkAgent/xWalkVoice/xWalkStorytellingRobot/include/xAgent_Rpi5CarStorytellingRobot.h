/******************************************************************************
 * @file        xAgent_Rpi5CarStorytellingRobot.h
 * @brief       Declares the Piper storytelling movement coordinator.
 * @project     xWalk Firmware
 * @module      xWalkStorytellingRobot
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_STORYTELLING_ROBOT_H
#define XAGENT_RPI5CAR_STORYTELLING_ROBOT_H

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarStorytellingRobotTypes.h"
#include "xHal_Rpi5CarTextToSpeech.h"

namespace xwalk::agent
{

    /** @brief Ports the narrative movement sequence from example 15. */
    class XWalkStorytellingRobot final
    {
        private:
            XWalkPicarx* picarxObject{nullptr};
            hal::XWalkTextToSpeech* textToSpeechObject{nullptr};
            agent::contextpointer callbackContext{nullptr};
            XWalkStorytellingRobotCallbacks callbacks{};
            XWalkStorytellingRobotConfiguration configuration{};

        protected:
            static void validate(const XWalkStorytellingRobotCallbacks& backendCallbacks,
                                 const XWalkStorytellingRobotConfiguration& robotConfiguration);
            agent::boolean wait(agent::uint32 durationMs) const;
            agent::boolean driveForwardAndNarrate(agent::stringview narration);

        public:
            XWalkStorytellingRobot(XWalkPicarx& picarx,
                                   hal::XWalkTextToSpeech& textToSpeech,
                                   agent::contextpointer context,
                                   const XWalkStorytellingRobotCallbacks& backendCallbacks,
                                   const XWalkStorytellingRobotConfiguration& robotConfiguration = {});
            ~XWalkStorytellingRobot() = default;

            XWalkStorytellingRobot(const XWalkStorytellingRobot&) = delete;
            XWalkStorytellingRobot(XWalkStorytellingRobot&&) = delete;
            XWalkStorytellingRobot& operator=(const XWalkStorytellingRobot&) = delete;
            XWalkStorytellingRobot& operator=(XWalkStorytellingRobot&&) = delete;

            agent::int32 run();
            void stop();
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_STORYTELLING_ROBOT_H */
