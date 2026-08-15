/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlSimulation.h
 * @brief       Declares the device-free xWalkBoardControl simulation.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_BOARD_CONTROL_SIMULATION_H
#define XHAL_RPI5CAR_BOARD_CONTROL_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Exercises BoardControl without opening physical devices. */
    int32 runBoardControlSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_BOARD_CONTROL_SIMULATION_H */
