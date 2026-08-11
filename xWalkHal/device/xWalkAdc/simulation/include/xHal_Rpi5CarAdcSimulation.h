/******************************************************************************
 * @file        xHal_Rpi5CarAdcSimulation.h
 * @brief       Declares the device-free xWalkAdc simulation.
 * @project     xWalk Firmware
 * @module      xWalkAdc Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ADC_SIMULATION_H
#define XHAL_RPI5CAR_ADC_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises ADC selection, acquisition, and voltage conversion in memory. */
int32 runAdcSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ADC_SIMULATION_H */
