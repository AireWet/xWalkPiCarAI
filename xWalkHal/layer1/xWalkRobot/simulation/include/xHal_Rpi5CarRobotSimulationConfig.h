/******************************************************************************
 * @file        xHal_Rpi5CarRobotSimulationConfig.h
 * @brief       Validates generated xWalkRobot simulation paths.
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ROBOT_SIMULATION_CONFIG_H
#define XHAL_RPI5CAR_ROBOT_SIMULATION_CONFIG_H
/** @brief Default generated trace inventory used outside a configured target. */
#ifndef XWALK_ROBOT_SIMULATION_TRACE_CONFIG_PATH
#define XWALK_ROBOT_SIMULATION_TRACE_CONFIG_PATH "xwalk-traces.xml"
#endif
/** @brief Default trace log path used outside a configured target. */
#ifndef XWALK_ROBOT_SIMULATION_TRACE_LOG_PATH
#define XWALK_ROBOT_SIMULATION_TRACE_LOG_PATH "log/xWalkRobotTrace.log"
#endif
/** @brief Default simulated calibration store used outside a configured target. */
#ifndef XWALK_ROBOT_SIMULATION_STORE_PATH
#define XWALK_ROBOT_SIMULATION_STORE_PATH "robot-simulation.config"
#endif
#endif /* XHAL_RPI5CAR_ROBOT_SIMULATION_CONFIG_H */
