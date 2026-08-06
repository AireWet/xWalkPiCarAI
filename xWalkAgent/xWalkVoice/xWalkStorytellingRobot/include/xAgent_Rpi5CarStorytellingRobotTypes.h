/******************************************************************************
 * @file        xAgent_Rpi5CarStorytellingRobotTypes.h
 * @brief       Declares storytelling callbacks and source-compatible values.
 * @project     xWalk Firmware
 * @module      xWalkStorytellingRobot
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_STORYTELLING_ROBOT_TYPES_H
#define XAGENT_RPI5CAR_STORYTELLING_ROBOT_TYPES_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::agent
{

using storytellingrobotdelaycallback = void (*)(agent::contextpointer,
    agent::uint32);
using storytellingrobotcontinuecallback = agent::boolean (*)(
    agent::contextpointer);

/** @brief Stores the complete synchronous application callback boundary. */
struct XWalkStorytellingRobotCallbacks
{
    storytellingrobotdelaycallback delay{nullptr};
    storytellingrobotcontinuecallback shouldContinue{nullptr};
};

/** @brief Stores the exact example 15 motion, timing, and narration values. */
struct XWalkStorytellingRobotConfiguration
{
    agent::float64 speedPercent{30.0};
    agent::uint32 outwardLegDurationMs{3'000U};
    agent::uint32 homeLegDurationMs{6'000U};
    agent::string greeting{"Hello! I'm PiCar-X speaking with Piper."};
    agent::string firstJoke{
        "Why can't your nose be twelve inches long? Because then it would be a foot!"};
    agent::string secondJoke{
        "Why did the cow go to outer space? To see the moooon!"};
    agent::string farewell{
        "That's all for today. Goodbye, let's go home and sleep."};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_STORYTELLING_ROBOT_TYPES_H */
