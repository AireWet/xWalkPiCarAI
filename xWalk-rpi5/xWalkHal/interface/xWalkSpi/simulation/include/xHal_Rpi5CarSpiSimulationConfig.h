/******************************************************************************
 * @file        xHal_Rpi5CarSpiSimulationConfig.h
 * @brief       Defines xWalkSpi simulation trace paths.
 *
 * @details
 * Provides guarded non-CMake defaults for the generated trace inventory,
 * build-local log, and selected Linux SPI device.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Simulation
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

#ifndef XHAL_RPI5CAR_SPI_SIMULATION_CONFIG_H
#define XHAL_RPI5CAR_SPI_SIMULATION_CONFIG_H

/** @brief Default generated trace inventory used outside a configured CMake target. */
#ifndef XWALK_SPI_SIMULATION_TRACE_CONFIG_PATH
    #define XWALK_SPI_SIMULATION_TRACE_CONFIG_PATH "xwalk-traces.xml"
#endif

/** @brief Default SPI trace log used outside a configured CMake target. */
#ifndef XWALK_SPI_SIMULATION_TRACE_LOG_PATH
    #define XWALK_SPI_SIMULATION_TRACE_LOG_PATH "log/xWalkSpiTrace.log"
#endif

/** @brief Default physical Linux device used by the hardware selection. */
#ifndef XWALK_SPI_SIMULATION_DEVICE_PATH
    #define XWALK_SPI_SIMULATION_DEVICE_PATH "/dev/spidev0.0"
#endif

#endif /* XHAL_RPI5CAR_SPI_SIMULATION_CONFIG_H */
