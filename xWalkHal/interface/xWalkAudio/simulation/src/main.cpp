/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the standalone xWalkAudio operation simulation.
 *
 * @details
 * Configures persistent tracing, validates command-line trace selectors, and
 * starts the build-selected device-free or physical Audio composition.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio Host Simulation
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAudioSimulation.h"
#include "xHal_Rpi5CarAudioSimulationArguments.h"
#include "xHal_Rpi5CarAudioSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

XWalkHal::int32 main(XWalkHal::int32 argumentCount,
                     XWalkHal::charpointer argumentValues[]) {
  const XWalkHal::filesystempath traceConfigurationPath(
      XWALK_AUDIO_SIMULATION_TRACE_CONFIG_PATH);
  const XWalkHal::filesystempath traceLogPath(
      XWALK_AUDIO_SIMULATION_TRACE_LOG_PATH);
  xwalk::hal::XWalkTrace::configureGlobal(traceConfigurationPath, traceLogPath);

  const xwalk::hal::sim::XWalkAudioSimulationArguments arguments(
      argumentCount, argumentValues);
  const XWalkHal::boolean argumentsValid = arguments.valid();
  if (argumentsValid == false) {
    XWALK_HAL_ERROR(XWALK_EXCEPTION, "Invalid xWalkAudio simulation arguments");
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      argumentValues[0]);
    return 2;
  }
  const XWalkHal::boolean helpRequested = arguments.helpRequested();
  if (helpRequested) {
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      argumentValues[0]);
    XWALK_HAL_WARNING(
        XWALK_LOGIC,
        "Run one bounded silent Audio playback through the selected backend");
    XWALK_HAL_WARNING(XWALK_LOGIC,
                      "Trace selectors: RPI.<digits>.enable, "
                      "RPI.<digits>.disable, RPI.enable, "
                      "RPI.disable, all.enable, all.disable, or FILE.json");
    XWALK_HAL_WARNING(
        XWALK_LOGIC,
        "New traces are disabled; saved XML states load automatically");
    return 0;
  }
  const XWalkHal::boolean traceUpdateApplied = arguments.applyTraceUpdate();
  if (traceUpdateApplied == false) {
    XWALK_HAL_ERROR(
        XWALK_EXCEPTION,
        "The requested trace identifier is not present in the trace inventory");
    return 2;
  }
  XWALK_HAL_TRACE_UID0(RPI .096, "xWalkAudio simulation started");
  const XWalkHal::int32 simulationResult =
      xwalk::hal::sim::runAudioSimulation();
  XWALK_HAL_TRACE_UID1(RPI .097,
                       "xWalkAudio simulation completed with status %d",
                       simulationResult);
  return simulationResult;
}
