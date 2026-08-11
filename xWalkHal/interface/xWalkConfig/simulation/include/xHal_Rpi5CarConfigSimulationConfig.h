/******************************************************************************
 * @file        xHal_Rpi5CarConfigSimulationConfig.h
 * @brief       Defines xWalkConfig simulation paths.
 *
 * @details
 * Supplies build-overridable persistent trace, log, and test-owned data paths.
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

#ifndef XHAL_RPI5CAR_CONFIG_SIMULATION_CONFIG_H
#define XHAL_RPI5CAR_CONFIG_SIMULATION_CONFIG_H

/******************************************************************************
 * Constants
 ******************************************************************************/

#ifndef XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH
#define XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH "xwalk-traces.xml"
#endif

#ifndef XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH
#define XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH "log/xWalkConfigTrace.log"
#endif

#ifndef XWALK_CONFIG_SIMULATION_DATA_PATH
#define XWALK_CONFIG_SIMULATION_DATA_PATH "simulation-data"
#endif

#endif /* XHAL_RPI5CAR_CONFIG_SIMULATION_CONFIG_H */
