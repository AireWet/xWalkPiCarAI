/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345Simulation.h
 * @brief       Declares the device-free xWalkAdxl345 simulation.
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ADXL345_SIMULATION_H
#define XHAL_RPI5CAR_ADXL345_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises ADXL345 configuration and three-axis conversion in memory. */
int32 runAdxl345Simulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ADXL345_SIMULATION_H */
