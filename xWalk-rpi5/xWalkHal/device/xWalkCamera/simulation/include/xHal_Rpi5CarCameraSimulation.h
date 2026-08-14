/******************************************************************************
 * @file        xHal_Rpi5CarCameraSimulation.h
 * @brief       Declares the device-free xWalkCamera simulation.
 * @project     xWalk Firmware
 * @module      xWalkCamera Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_CAMERA_SIMULATION_H
#define XHAL_RPI5CAR_CAMERA_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises one camera capture through an in-memory callback backend. */
int32 runCameraSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_CAMERA_SIMULATION_H */
