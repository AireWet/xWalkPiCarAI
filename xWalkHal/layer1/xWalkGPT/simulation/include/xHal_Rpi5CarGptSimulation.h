/******************************************************************************
 * @file        xHal_Rpi5CarGptSimulation.h
 * @brief       Declares the device-free xWalkGPT simulation.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_GPT_SIMULATION_H
#define XHAL_RPI5CAR_GPT_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises speech coordination without microphone or speaker devices. */
int32 runGptSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_GPT_SIMULATION_H */
