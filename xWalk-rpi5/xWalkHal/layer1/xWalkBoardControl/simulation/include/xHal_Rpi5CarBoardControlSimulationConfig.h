/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlSimulationConfig.h
 * @brief       Validates generated xWalkBoardControl simulation paths.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_BOARD_CONTROL_SIMULATION_CONFIG_H
#define XHAL_RPI5CAR_BOARD_CONTROL_SIMULATION_CONFIG_H
/** @brief Default generated trace inventory used outside a configured target. */
#ifndef XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH
    #define XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH "xwalk-traces.xml"
#endif
/** @brief Default trace log path used outside a configured target. */
#ifndef XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH
    #define XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH "log/xWalkBoardControlTrace.log"
#endif
/** @brief Default synthetic device-tree root used outside a configured target. */
#ifndef XWALK_BOARD_CONTROL_SIMULATION_DEVICE_TREE_PATH
    #define XWALK_BOARD_CONTROL_SIMULATION_DEVICE_TREE_PATH "device-tree"
#endif
#endif /* XHAL_RPI5CAR_BOARD_CONTROL_SIMULATION_CONFIG_H */
