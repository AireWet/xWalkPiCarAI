/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerSimulation.h
 * @brief       Declares the device-free xWalkBuzzer simulation.
 * @project     xWalk Firmware
 * @module      xWalkBuzzer Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_BUZZER_SIMULATION_H
#define XHAL_RPI5CAR_BUZZER_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises active and passive buzzers without physical output. */
int32 runBuzzerSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_BUZZER_SIMULATION_H */
