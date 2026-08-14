/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerSimulation.h
 * @brief       Declares the device-free xWalkLineTracker simulation.
 * @project     xWalk Firmware
 * @module      xWalkLineTracker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LINE_TRACKER_SIMULATION_H
#define XHAL_RPI5CAR_LINE_TRACKER_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises grayscale and line position through an in-memory I2C bus. */
int32 runLineTrackerSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_LINE_TRACKER_SIMULATION_H */
