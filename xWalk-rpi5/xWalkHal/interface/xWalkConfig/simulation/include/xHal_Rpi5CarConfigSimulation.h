/******************************************************************************
 * @file        xHal_Rpi5CarConfigSimulation.h
 * @brief       Declares the device-safe xWalkConfig simulation.
 *
 * @details
 * Exercises both Config APIs beneath one caller-provided writable directory.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_CONFIG_SIMULATION_H
#define XHAL_RPI5CAR_CONFIG_SIMULATION_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::sim
{

    /**
     * @brief Runs representative section and flat-store persistence operations.
     * @param[in] dataDirectory Writable simulation-owned directory.
     * @return Zero when values persist and reload successfully; otherwise one.
     */
    int32 runConfigSimulation(const filesystempath& dataDirectory);

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_CONFIG_SIMULATION_H */
