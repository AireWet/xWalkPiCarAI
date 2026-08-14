/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicSimulation.h
 * @brief       Declares the device-free xWalkUltrasonic simulation.
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ULTRASONIC_SIMULATION_H
#define XHAL_RPI5CAR_ULTRASONIC_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises trigger, echo timing, and conversion through in-memory GPIO. */
int32 runUltrasonicSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ULTRASONIC_SIMULATION_H */
