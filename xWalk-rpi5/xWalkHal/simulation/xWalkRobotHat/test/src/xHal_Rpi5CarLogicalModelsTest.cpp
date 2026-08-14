/******************************************************************************
 * @file        xHal_Rpi5CarLogicalModelsTest.cpp
 * @brief       Verifies deterministic logical behavioral model boundaries.
 * @project     xWalk Firmware
 * @module      xWalkRobotHatSimulationTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xHal_Rpi5CarRobotHatSimulationTestSupport.h"

#include <cassert>
#include <limits>

namespace xwalk::hal::test::robothat
{

void runLogicalModelTests()
{
    simulation::XWalkLogicalModelConfiguration configuration;
    configuration.accelerationPerTick = 5.0;
    configuration.decelerationPerTick = 10.0;
    configuration.invertRightDirection = true;
    configuration.steeringMinimum = -40.0;
    configuration.steeringMaximum = 40.0;
    configuration.servoCentre = 2.0;
    configuration.servoTravel = 30.0;
    configuration.initialBatteryVoltage = 7.4;
    configuration.batteryWarningVoltage = 7.25;
    configuration.batteryCriticalVoltage = 6.8;
    configuration.batteryReductionPerTick = 0.1;
    configuration.cameraDelayTicks = 4U;
    configuration.i2cFailureInterval = 3U;
    configuration.grayscaleSequence = {{{10U, 20U, 30U}}, {{40U, 50U, 60U}}};
    configuration.ultrasonicSequence = {100.0, 15.0};

    simulation::XWalkLogicalModelState state;
    assert(simulation::initializeLogicalModel(state, configuration) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert(state.armed && (state.steeringAngle == 2.0));
    assert(simulation::commandLogicalMotors(state, 150.0, 50.0) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert((state.commandedLeftSpeed == 100.0) &&
        (state.commandedRightSpeed == -50.0));
    assert(simulation::advanceLogicalModel(state, 2U) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert((state.simulatedLeftSpeed == 10.0) &&
        (state.simulatedRightSpeed == -10.0));
    assert(state.batteryWarning && !state.batteryCritical);

    assert(simulation::commandLogicalSteering(state, 90.0) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert(state.steeringAngle == 32.0);
    std::array<uint16, 3U> grayscale{};
    assert(simulation::nextLogicalGrayscale(state, grayscale) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert((grayscale[0U] == 10U) && (grayscale[2U] == 30U));
    float64 distance{};
    assert(simulation::nextLogicalUltrasonic(state, distance) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert(distance == 100.0);

    uint64 firstFrame{};
    uint64 secondFrame{};
    assert(simulation::nextLogicalCameraFrame(state, firstFrame) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert(simulation::nextLogicalCameraFrame(state, secondFrame) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert((firstFrame == 1U) && (secondFrame == 2U));
    assert(!simulation::logicalI2cOperationFails(state));
    assert(!simulation::logicalI2cOperationFails(state));
    assert(simulation::logicalI2cOperationFails(state));
    assert(!state.armed && (state.commandedLeftSpeed == 0.0) &&
        (state.commandedRightSpeed == 0.0));

    simulation::XWalkLogicalModelState frozen;
    configuration.freezeCamera = true;
    configuration.batteryReductionPerTick = 0.0;
    assert(simulation::initializeLogicalModel(frozen, configuration) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert(simulation::nextLogicalCameraFrame(frozen, firstFrame) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert(simulation::nextLogicalCameraFrame(frozen, secondFrame) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert(firstFrame == secondFrame);

    configuration.accelerationPerTick = 0.0;
    assert(simulation::initializeLogicalModel(state, configuration) ==
        simulation::XWalkLogicalModelStatus::InvalidConfiguration);
    assert(!state.initialized && !state.armed);
    configuration.accelerationPerTick = 1.0;
    configuration.ultrasonicSequence = {501.0};
    assert(simulation::validateLogicalModelConfiguration(configuration) ==
        simulation::XWalkLogicalModelStatus::InvalidConfiguration);

    configuration.ultrasonicSequence.clear();
    assert(simulation::initializeLogicalModel(state, configuration) ==
        simulation::XWalkLogicalModelStatus::Ok);
    assert(simulation::commandLogicalMotors(state,
        std::numeric_limits<float64>::quiet_NaN(), 1.0) ==
        simulation::XWalkLogicalModelStatus::InvalidConfiguration);
    assert(!state.armed && (state.simulatedLeftSpeed == 0.0));

    uint64 expectedSequence{1U};
    for (size index = 0U; index < state.eventCount; ++index)
    {
        assert(state.events[index].sequence == expectedSequence);
        ++expectedSequence;
    }
}

} /* namespace xwalk::hal::test::robothat */
