/******************************************************************************
 * @file        xHal_Rpi5CarMotorSimulation.h
 * @brief       Declares the device-free xWalkMotor simulation.
 * @project     xWalk Firmware
 * @module      xWalkMotor Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_MOTOR_SIMULATION_H
#define XHAL_RPI5CAR_MOTOR_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Exercises one motor without opening I2C or GPIO hardware. */
    int32 runMotorSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_MOTOR_SIMULATION_H */
