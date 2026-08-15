/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxTestSupport.h
 * @brief       Declares reusable simulator-backed PiCar-X test composition.
 * @project     xWalk Firmware
 * @module      xWalkPicarxTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_PICARX_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_PICARX_TEST_SUPPORT_H

#include "xAgent_Rpi5CarPicarx.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarRobotHatSimulation.h"

#include <memory>

namespace xwalk::agent::test::picarx
{

    /** @brief Owns a complete V5 dual-PWM PiCar-X composition over the device-free simulator. */
    class SimulationRig final
    {
        public:
            hal::simulation::XWalkRobotHatSimulation backend{};
            hal::XWalkI2c i2c;
            hal::XWalkPwmTimerState timerState{};
            hal::XWalkPwm motorOneForward;
            hal::XWalkPwm motorOneReverse;
            hal::XWalkPwm motorTwoForward;
            hal::XWalkPwm motorTwoReverse;
            hal::XWalkPwm directionPwm;
            hal::XWalkPwm panPwm;
            hal::XWalkPwm tiltPwm;
            hal::XWalkMotor motorOne;
            hal::XWalkMotor motorTwo;
            hal::XWalkMotors motors;
            hal::XWalkServo directionServo;
            hal::XWalkServo panServo;
            hal::XWalkServo tiltServo;
            hal::XWalkAdc adcZero;
            hal::XWalkAdc adcOne;
            hal::XWalkAdc adcTwo;
            hal::XWalkGrayscaleModule grayscale;
            hal::XWalkGpio trigger;
            hal::XWalkGpio echo;
            hal::XWalkUltrasonic ultrasonic;
            hal::XWalkConfigStore config;
            std::unique_ptr<XWalkPicarx> vehicle{};

            /** @brief Constructs a fully owned test composition without initializing actuators. */
            SimulationRig(agent::stringview configPath, agent::boolean applyPersistedPositions);
            ~SimulationRig() = default;
            SimulationRig(const SimulationRig&) = delete;
            SimulationRig& operator=(const SimulationRig&) = delete;
            SimulationRig(SimulationRig&&) = delete;
            SimulationRig& operator=(SimulationRig&&) = delete;
    };

    /** @brief Counts successful I2C writes to one register. */
    agent::size successfulRegisterWrites(const SimulationRig& rig, agent::uint8 reg);

} /* namespace xwalk::agent::test::picarx */

#endif /* XAGENT_RPI5CAR_PICARX_TEST_SUPPORT_H */
