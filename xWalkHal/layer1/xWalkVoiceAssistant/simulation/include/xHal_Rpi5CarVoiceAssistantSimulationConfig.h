/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantSimulationConfig.h
 * @brief       Defines source-visible VoiceAssistant simulation paths.
 *
 * @details
 * Supplies editor and direct-source parsing defaults while allowing configured
 * CMake targets to provide authoritative build-local paths.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant Host Simulation
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

#ifndef XHAL_RPI5CAR_VOICE_ASSISTANT_SIMULATION_CONFIG_H
#define XHAL_RPI5CAR_VOICE_ASSISTANT_SIMULATION_CONFIG_H

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Default generated trace inventory used outside a configured target. */
#ifndef XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_CONFIG_PATH
#define XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_CONFIG_PATH "xwalk-traces.xml"
#endif

/** @brief Default trace log path used outside a configured target. */
#ifndef XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_LOG_PATH
#define XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_LOG_PATH "log/xWalkVoiceAssistantTrace.log"
#endif

#endif /* XHAL_RPI5CAR_VOICE_ASSISTANT_SIMULATION_CONFIG_H */
