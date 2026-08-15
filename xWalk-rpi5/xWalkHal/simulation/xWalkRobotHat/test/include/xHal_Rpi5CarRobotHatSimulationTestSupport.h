/******************************************************************************
 * @file        xHal_Rpi5CarRobotHatSimulationTestSupport.h
 * @brief       Declares reusable Robot HAT simulation test helpers.
 * @project     xWalk Firmware
 * @module      xWalkRobotHatSimulationTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_ROBOT_HAT_SIMULATION_TEST_SUPPORT_H
#define XHAL_RPI5CAR_ROBOT_HAT_SIMULATION_TEST_SUPPORT_H

#include "xHal_Rpi5CarRobotHatSimulation.h"
#include "xHal_Rpi5CarLogicalModels.h"

namespace xwalk::hal::test::robothat
{

    /** @brief Counts recorded events having the selected operation. */
    size countEvents(const simulation::XWalkRobotHatSimulation& simulation,
                     simulation::XWalkRobotHatOperation operation);

    /** @brief Returns true only when logical event timestamps are contiguous and ordered. */
    boolean hasDeterministicOrdering(const simulation::XWalkRobotHatSimulation& simulation);

    /** @brief Runs deterministic logical-model boundary and scenario assertions. */
    void runLogicalModelTests();

} /* namespace xwalk::hal::test::robothat */

#endif /* XHAL_RPI5CAR_ROBOT_HAT_SIMULATION_TEST_SUPPORT_H */
