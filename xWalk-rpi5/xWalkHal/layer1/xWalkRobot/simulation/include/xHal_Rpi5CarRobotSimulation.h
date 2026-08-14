/******************************************************************************
 * @file        xHal_Rpi5CarRobotSimulation.h
 * @brief       Declares the device-free xWalkRobot simulation.
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ROBOT_SIMULATION_H
#define XHAL_RPI5CAR_ROBOT_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises Robot with one simulated servo and a build-local store. */
int32 runRobotSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ROBOT_SIMULATION_H */
