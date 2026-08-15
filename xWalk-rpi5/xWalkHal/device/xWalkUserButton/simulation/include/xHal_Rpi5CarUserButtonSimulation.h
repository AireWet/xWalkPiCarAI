/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonSimulation.h
 * @brief       Declares the device-free xWalkUserButton simulation.
 * @project     xWalk Firmware
 * @module      xWalkUserButton Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_USER_BUTTON_SIMULATION_H
#define XHAL_RPI5CAR_USER_BUTTON_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Exercises one short active-low press through in-memory GPIO. */
    int32 runUserButtonSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_USER_BUTTON_SIMULATION_H */
