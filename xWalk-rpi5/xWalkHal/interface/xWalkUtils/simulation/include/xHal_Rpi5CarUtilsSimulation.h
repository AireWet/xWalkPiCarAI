/******************************************************************************
 * @file        xHal_Rpi5CarUtilsSimulation.h
 * @brief       Declares the side-effect-free xWalkUtils simulation.
 * @details     Exercises representative public utility operations through the host stub.
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_UTILS_SIMULATION_H
#define XHAL_RPI5CAR_UTILS_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Runs representative Utils operations through the in-memory host stub. */
    int32 runUtilsSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_UTILS_SIMULATION_H */
