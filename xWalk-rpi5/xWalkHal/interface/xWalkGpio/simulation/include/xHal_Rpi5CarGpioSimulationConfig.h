/******************************************************************************
 * @file        xHal_Rpi5CarGpioSimulationConfig.h
 * @brief       Defines xWalkGpio simulation trace and device paths.
 *
 * @details
 * Provides guarded non-CMake defaults for the generated trace inventory,
 * build-local log, and selected Linux GPIO device.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Simulation
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

#ifndef XHAL_RPI5CAR_GPIO_SIMULATION_CONFIG_H
#define XHAL_RPI5CAR_GPIO_SIMULATION_CONFIG_H

/** @brief Default generated trace inventory used outside a configured CMake target. */
#ifndef XWALK_GPIO_SIMULATION_TRACE_CONFIG_PATH
    #define XWALK_GPIO_SIMULATION_TRACE_CONFIG_PATH "xwalk-traces.xml"
#endif

/** @brief Default GPIO trace log used outside a configured CMake target. */
#ifndef XWALK_GPIO_SIMULATION_TRACE_LOG_PATH
    #define XWALK_GPIO_SIMULATION_TRACE_LOG_PATH "log/xWalkGpioTrace.log"
#endif

/** @brief Default physical Linux device used by the hardware selection. */
#ifndef XWALK_GPIO_SIMULATION_DEVICE_PATH
    #define XWALK_GPIO_SIMULATION_DEVICE_PATH "/dev/gpiochip0"
#endif

#endif /* XHAL_RPI5CAR_GPIO_SIMULATION_CONFIG_H */
