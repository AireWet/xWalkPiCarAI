/******************************************************************************
 * @file        xHal_Rpi5CarRobotSimulation.cpp
 * @brief       Implements the device-free xWalkRobot simulation.
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarRobotSimulation.h"
#include "xHal_Rpi5CarRobotHostStub.h"
#include "xHal_Rpi5CarRobotSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    int32 runRobotSimulation()
    {
        const filesystempath storePath(XWALK_ROBOT_SIMULATION_STORE_PATH);
        filesystempath temporaryPath = storePath;
        temporaryPath += ".tmp";
        static_cast<void>(removeFilesystemEntry(storePath));
        static_cast<void>(removeFilesystemEntry(temporaryPath));
        XWalkRobotHostStub backend;
        XWalkI2c i2c(
            &backend, &XWalkRobotHostStub::probe, &XWalkRobotHostStub::writeRegister, &XWalkRobotHostStub::read);
        XWalkPwmTimerState timerState;
        XWalkPwm pwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
        XWalkServo servo(pwm);
        XWalkConfigStore store(storePath.string());
        XWalkRobot robot(store, "simulation", 0U);
        robot.addServo(servo);
        robot.initialize();
        robot.setOffsets({2.0});
        robot.setOriginPositions({1.0});
        robot.setDirections({1.0});
        robot.setCalibrationPositions({3.0});
        robot.calibration();
        robot.reset({0.0});
        robot.setAction("nod", {{1.0}, {0.0}});
        robot.doAction("nod", 1U, 100.0);
        const boolean valid = robot.initialized() && (robot.servoCount() == 1U) &&
                              (robot.offsets() == float64vector({2.0})) &&
                              (robot.servoPositions() == float64vector({0.0})) && (backend.writeCount() > 0U);
        static_cast<void>(removeFilesystemEntry(storePath));
        static_cast<void>(removeFilesystemEntry(temporaryPath));
        static_cast<void>(removeFilesystemEntry(storePath.parent_path()));
        XWALK_HAL_TRACE_UID0(RPI .352, "xWalkRobot host simulation completed");
        return valid ? 0 : 1;
    }
} /* namespace xwalk::hal::sim */
