/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the network-free language-model simulation.
 * @details     Loads persistent tracing and dispatches the in-memory backend.
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarLanguageModelSimulation.h"
#include "xHal_Rpi5CarLanguageModelSimulationArguments.h"
#include "xHal_Rpi5CarLanguageModelSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"
XWalkHal::int32 main(XWalkHal::int32 count, XWalkHal::charpointer values[]) {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_LOG_PATH);
  const xwalk::hal::sim::XWalkLanguageModelSimulationArguments arguments(
      count, values);
  if (arguments.valid() == false) {
    XWALK_HAL_ERROR(XWALK_EXCEPTION,
                    "Invalid xWalkLanguageModel simulation arguments");
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      values[0]);
    return 2;
  }
  if (arguments.helpRequested()) {
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      values[0]);
    XWALK_HAL_WARNING(
        XWALK_LOGIC,
        "Saved XML trace states load automatically on the next run");
    return 0;
  }
  if (arguments.applyTraceUpdate() == false) {
    XWALK_HAL_ERROR(
        XWALK_EXCEPTION,
        "The requested trace identifier is not present in the trace inventory");
    return 2;
  }
  XWALK_HAL_TRACE_UID0(RPI .151, "xWalkLanguageModel simulation started");
  const XWalkHal::int32 result = xwalk::hal::sim::runLanguageModelSimulation();
  XWALK_HAL_TRACE_UID1(RPI .152,
                       "xWalkLanguageModel simulation completed with status %d",
                       result);
  return result;
}
