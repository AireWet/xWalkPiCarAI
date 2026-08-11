/******************************************************************************
 * @file        xHal_Rpi5CarRobotHatSimulationTestSupport.cpp
 * @brief       Implements reusable Robot HAT simulation test helpers.
 * @project     xWalk Firmware
 * @module      xWalkRobotHatSimulationTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xHal_Rpi5CarRobotHatSimulationTestSupport.h"

namespace xwalk::hal::test::robothat
{

size countEvents(const simulation::XWalkRobotHatSimulation& simulation,
    simulation::XWalkRobotHatOperation operation)
{
    size count{};
    for (const simulation::XWalkRobotHatEvent& event : simulation.events())
    {
        if (event.operation == operation)
        {
            ++count;
        }
    }
    return count;
}

boolean hasDeterministicOrdering(const simulation::XWalkRobotHatSimulation& simulation)
{
    uint64 expected{1U};
    for (const simulation::XWalkRobotHatEvent& event : simulation.events())
    {
        if (event.sequence != expected)
        {
            return false;
        }
        ++expected;
    }
    return true;
}

} /* namespace xwalk::hal::test::robothat */
