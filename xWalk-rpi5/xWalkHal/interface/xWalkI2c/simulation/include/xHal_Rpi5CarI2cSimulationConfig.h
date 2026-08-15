/******************************************************************************
 * @file        xHal_Rpi5CarI2cSimulationConfig.h
 * @brief       Defines xWalkI2c simulation trace paths.
 *
 * @details
 * Provides guarded non-CMake defaults for the generated trace inventory and
 * build-local I2C simulation log selected by supported CMake targets.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-09
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_I2C_SIMULATION_CONFIG_H
#define XHAL_RPI5CAR_I2C_SIMULATION_CONFIG_H

/******************************************************************************
 * Constants
 ******************************************************************************/

/** @brief Default generated trace inventory used outside a configured CMake target. */
#ifndef XWALK_I2C_SIMULATION_TRACE_CONFIG_PATH
    #define XWALK_I2C_SIMULATION_TRACE_CONFIG_PATH "xwalk-traces.xml"
#endif

/** @brief Default I2C test trace log used outside a configured CMake target. */
#ifndef XWALK_I2C_SIMULATION_TRACE_LOG_PATH
    #define XWALK_I2C_SIMULATION_TRACE_LOG_PATH "log/xWalkI2cTrace.log"
#endif

/** @brief Default physical Linux device used by the hardware selection. */
#ifndef XWALK_I2C_SIMULATION_DEVICE_PATH
    #define XWALK_I2C_SIMULATION_DEVICE_PATH "/dev/i2c-1"
#endif

#endif /* XHAL_RPI5CAR_I2C_SIMULATION_CONFIG_H */
