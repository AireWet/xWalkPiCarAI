/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxSimulationTest.cpp
 * @brief       Verifies PiCar-X lifecycle and safety through the Robot HAT simulator.
 * @project     xWalk Firmware
 * @module      xWalkPicarxTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarxTestSupport.h"

#include "xHal_Rpi5CarFileFunctions.h"

#include <cassert>
#include <stdexcept>

namespace
{
using xwalk::agent::test::picarx::SimulationRig;
using xwalk::agent::test::picarx::successfulRegisterWrites;

void testSafeCommissioningGate(xwalk::agent::stringview path)
{
    SimulationRig rig(path, false);
    assert(rig.backend.events().empty());
    assert(rig.vehicle->initialize());
    assert(!rig.vehicle->initialize());
    assert(successfulRegisterWrites(rig, 0x22U) == 0U);
    assert(successfulRegisterWrites(rig, 0x20U) == 0U);
    assert(successfulRegisterWrites(rig, 0x21U) == 0U);
    assert(rig.motors.isArmed());
    rig.vehicle->emergencyStop();
    assert(!rig.motors.isArmed());
}

void testExplicitPersistedPositions(xwalk::agent::stringview path)
{
    SimulationRig rig(path, true);
    assert(rig.vehicle->initialize());
    assert(successfulRegisterWrites(rig, 0x22U) == 1U);
    assert(successfulRegisterWrites(rig, 0x20U) == 1U);
    assert(successfulRegisterWrites(rig, 0x21U) == 1U);
    assert(rig.motors.isArmed());
}

void testInitializationFailureStaysDisarmed(xwalk::agent::stringview path)
{
    SimulationRig rig(path, true);
    rig.backend.failNext(xwalk::hal::simulation::XWalkRobotHatOperation::I2cWrite, 0x22U);
    bool failed{};
    try
    {
        static_cast<void>(rig.vehicle->initialize());
    }
    catch (const std::exception&)
    {
        failed = true;
    }
    assert(failed);
    assert(!rig.vehicle->isInitialized());
    assert(!rig.motors.isArmed());
    assert(rig.vehicle->emergencyStopRequested());
}

void testInitializationStageFailureStaysDisarmed(xwalk::agent::stringview path,
    xwalk::agent::uint8 failingRegister)
{
    SimulationRig rig(path, true);
    rig.backend.failNext(xwalk::hal::simulation::XWalkRobotHatOperation::I2cWrite,
        failingRegister);
    bool failed{};
    try
    {
        static_cast<void>(rig.vehicle->initialize());
    }
    catch (const std::exception&)
    {
        failed = true;
    }
    assert(failed);
    assert(!rig.vehicle->isInitialized());
    assert(!rig.motors.isArmed());
    assert(rig.vehicle->emergencyStopRequested());
    assert(rig.motorOne.speed() == 0.0);
    assert(rig.motorTwo.speed() == 0.0);
}

void testCommandsBeforeInitializationAreRejected(xwalk::agent::stringview path)
{
    SimulationRig rig(path, false);
    const xwalk::agent::size eventsBeforeCommand = rig.backend.events().size();
    bool motorRejected{};
    bool servoRejected{};
    try
    {
        rig.vehicle->forward(10.0);
    }
    catch (const std::exception&)
    {
        motorRejected = true;
    }
    try
    {
        rig.vehicle->setDirectionServoAngle(10.0);
    }
    catch (const std::exception&)
    {
        servoRejected = true;
    }
    assert(motorRejected);
    assert(servoRejected);
    assert(rig.backend.events().size() == eventsBeforeCommand);
    assert(!rig.motors.isArmed());
}

void testMovementFaultDisarmsAndStopsBothChannels(xwalk::agent::stringview path,
    xwalk::agent::uint8 failingRegister)
{
    SimulationRig rig(path, false);
    static_cast<void>(rig.vehicle->initialize());
    rig.backend.failNext(xwalk::hal::simulation::XWalkRobotHatOperation::I2cWrite,
        failingRegister);
    bool failed{};
    try
    {
        rig.vehicle->forward(10.0);
    }
    catch (const std::runtime_error&)
    {
        failed = true;
    }
    assert(failed);
    assert(!rig.motors.isArmed());
    assert(rig.motorOne.speed() == 0.0);
    assert(rig.motorTwo.speed() == 0.0);
}

void testShutdownIsNonActuatingAndIdempotent(xwalk::agent::stringview path)
{
    SimulationRig rig(path, true);
    rig.vehicle->close();
    rig.vehicle->close();
    assert(!rig.vehicle->isInitialized());
    assert(rig.vehicle->emergencyStopRequested());
    assert(successfulRegisterWrites(rig, 0x22U) == 0U);
    assert(successfulRegisterWrites(rig, 0x20U) == 0U);
    assert(successfulRegisterWrites(rig, 0x21U) == 0U);
}

void testEmergencyRecoveryDoesNotResumeMovement(xwalk::agent::stringview path)
{
    SimulationRig rig(path, false);
    assert(rig.vehicle->initialize());
    rig.vehicle->forward(10.0);
    assert(rig.motorOne.speed() != 0.0);
    assert(rig.motorTwo.speed() != 0.0);
    assert(rig.vehicle->emergencyStop());
    assert(!rig.motors.isArmed());
    assert(rig.motorOne.speed() == 0.0);
    assert(rig.motorTwo.speed() == 0.0);

    rig.vehicle->clearEmergencyStop();
    assert(rig.motors.isArmed());
    assert(!rig.vehicle->emergencyStopRequested());
    assert(rig.motorOne.speed() == 0.0);
    assert(rig.motorTwo.speed() == 0.0);
    rig.vehicle->forward(10.0);
    assert(rig.motorOne.speed() != 0.0);
    assert(rig.motorTwo.speed() != 0.0);
}

void testShutdownDuringMovementStopsAndDisarms(xwalk::agent::stringview path)
{
    SimulationRig rig(path, false);
    assert(rig.vehicle->initialize());
    rig.vehicle->forward(10.0);
    rig.vehicle->close();
    assert(!rig.vehicle->isInitialized());
    assert(!rig.motors.isArmed());
    assert(rig.vehicle->emergencyStopRequested());
    assert(rig.motorOne.speed() == 0.0);
    assert(rig.motorTwo.speed() == 0.0);
    rig.vehicle->close();
}

} /* namespace */

int main(int argumentCount, char* arguments[])
{
    if (argumentCount != 2)
    {
        return 1;
    }
    const xwalk::agent::filesystempath base(arguments[1]);
    for (xwalk::agent::uint32 index = 0U; index < 17U; ++index)
    {
        xwalk::agent::filesystempath path = base;
        path += "." + std::to_string(index);
        static_cast<void>(xwalk::hal::removeFilesystemEntry(path));
        static_cast<void>(xwalk::hal::removeFilesystemEntry(path.string() + ".tmp"));
        if (index == 0U)
        {
            testSafeCommissioningGate(path.string());
        }
        else if (index == 1U)
        {
            testExplicitPersistedPositions(path.string());
        }
        else if (index == 2U)
        {
            testInitializationFailureStaysDisarmed(path.string());
        }
        else if ((index >= 3U) && (index <= 8U))
        {
            const xwalk::agent::fixedarray<xwalk::agent::uint8, 6U> registers{
                0x20U, 0x21U, 0x2CU, 0x2DU, 0x2EU, 0x2FU};
            testInitializationStageFailureStaysDisarmed(path.string(),
                registers[index - 3U]);
        }
        else if (index == 9U)
        {
            testCommandsBeforeInitializationAreRejected(path.string());
        }
        else if ((index >= 10U) && (index <= 13U))
        {
            testMovementFaultDisarmsAndStopsBothChannels(path.string(),
                static_cast<xwalk::agent::uint8>(0x2CU + (index - 10U)));
        }
        else if (index == 14U)
        {
            testEmergencyRecoveryDoesNotResumeMovement(path.string());
        }
        else if (index == 15U)
        {
            testShutdownDuringMovementStopsAndDisarms(path.string());
        }
        else
        {
            testShutdownIsNonActuatingAndIdempotent(path.string());
        }
        static_cast<void>(xwalk::hal::removeFilesystemEntry(path));
        static_cast<void>(xwalk::hal::removeFilesystemEntry(path.string() + ".tmp"));
    }
    return 0;
}
