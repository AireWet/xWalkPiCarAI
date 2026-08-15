/******************************************************************************
 * @file        xHal_Rpi5CarServoSimulation.h
 * @brief       Declares the device-free xWalkServo simulation.
 * @project     xWalk Firmware
 * @module      xWalkServo Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_SERVO_SIMULATION_H
#define XHAL_RPI5CAR_SERVO_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Exercises Servo angle and pulse output through an in-memory bus. */
    int32 runServoSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_SERVO_SIMULATION_H */
