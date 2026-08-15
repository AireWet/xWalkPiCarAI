/******************************************************************************
 * @file        xHal_Rpi5CarLedSimulation.h
 * @brief       Declares the device-free xWalkLed simulation.
 * @project     xWalk Firmware
 * @module      xWalkLed Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LED_SIMULATION_H
#define XHAL_RPI5CAR_LED_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Exercises single-color and RGB LEDs without opening hardware. */
    int32 runLedSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_LED_SIMULATION_H */
