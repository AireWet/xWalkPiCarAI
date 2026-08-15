/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxTestSupport.cpp
 * @brief       Implements reusable simulator-backed PiCar-X test composition.
 * @project     xWalk Firmware
 * @module      xWalkPicarxTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarxTestSupport.h"

namespace xwalk::agent::test::picarx
{

    SimulationRig::SimulationRig(agent::stringview configPath, agent::boolean applyPersistedPositions)
        : i2c(&backend,
              &hal::simulation::XWalkRobotHatSimulation::probe,
              &hal::simulation::XWalkRobotHatSimulation::writeRegister,
              &hal::simulation::XWalkRobotHatSimulation::read,
              &hal::simulation::XWalkRobotHatSimulation::readRegister,
              &hal::simulation::XWalkRobotHatSimulation::tryWriteRegister),
          motorOneForward(i2c, "P12", 0x14U, timerState), motorOneReverse(i2c, "P13", 0x14U, timerState),
          motorTwoForward(i2c, "P14", 0x14U, timerState), motorTwoReverse(i2c, "P15", 0x14U, timerState),
          directionPwm(i2c, "P2", 0x14U, timerState), panPwm(i2c, "P0", 0x14U, timerState),
          tiltPwm(i2c, "P1", 0x14U, timerState), motorOne(motorOneForward, motorOneReverse),
          motorTwo(motorTwoForward, motorTwoReverse), motors(motorOne,
                                                             motorTwo,
                                                             []()
                                                             {
                                                                 hal::XWalkMotorsConfiguration value;
                                                                 value.watchdogWorkerEnabled = false;
                                                                 return value;
                                                             }()),
          directionServo(directionPwm), panServo(panPwm), tiltServo(tiltPwm), adcZero(i2c, "A0", 0x14U),
          adcOne(i2c, "A1", 0x14U), adcTwo(i2c, "A2", 0x14U), grayscale(adcZero, adcOne, adcTwo),
          trigger(&backend, hal::simulation::XWalkRobotHatSimulation::gpioCallbacks(), "D2"),
          echo(&backend, hal::simulation::XWalkRobotHatSimulation::gpioCallbacks(), "D3"),
          ultrasonic(trigger, echo, 0U), config(configPath)
    {
        config.set("picarx_dir_servo", "5");
        config.set("picarx_cam_pan_servo", "6");
        config.set("picarx_cam_tilt_servo", "7");
        config.set("picarx_apply_persisted_servo_positions", applyPersistedPositions ? "true" : "false");
        vehicle =
            std::make_unique<XWalkPicarx>(motors, directionServo, panServo, tiltServo, grayscale, ultrasonic, config);
        backend.clearEvents();
    }

    agent::size successfulRegisterWrites(const SimulationRig& rig, agent::uint8 reg)
    {
        agent::size count{};
        for (const hal::simulation::XWalkRobotHatEvent& event : rig.backend.events())
        {
            if ((event.operation == hal::simulation::XWalkRobotHatOperation::I2cWrite) && (event.target == reg) &&
                event.succeeded)
            {
                ++count;
            }
        }
        return count;
    }

} /* namespace xwalk::agent::test::picarx */
