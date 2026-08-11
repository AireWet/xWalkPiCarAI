/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the standalone xWalkConfig simulation.
 *
 * @details
 * Loads persistent trace state, validates an optional selector, and performs
 * bounded filesystem operations only below the configured build directory.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarConfigSimulation.h"
#include "xHal_Rpi5CarConfigSimulationArguments.h"
#include "xHal_Rpi5CarConfigSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the bounded xWalkConfig standalone simulation.
 * @param[in] argumentCount Number of process arguments including the binary
 * name.
 * @param[in] argumentValues Process arguments valid throughout execution.
 * @return Zero on success, one on persistence failure, or two for invalid
 * input.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount,
                     XWalkHal::charpointer argumentValues[]) {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH);
  const xwalk::hal::sim::XWalkConfigSimulationArguments arguments(
      argumentCount, argumentValues);
  if (arguments.valid() == false) {
    XWALK_HAL_ERROR(XWALK_EXCEPTION,
                    "Invalid xWalkConfig simulation arguments");
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      argumentValues[0]);
    return 2;
  }
  if (arguments.helpRequested()) {
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      argumentValues[0]);
    XWALK_HAL_WARNING(XWALK_LOGIC, "Trace selectors persist in XML and load "
                                   "automatically on the next run");
    return 0;
  }
  if (arguments.applyTraceUpdate() == false) {
    XWALK_HAL_ERROR(
        XWALK_EXCEPTION,
        "The requested trace identifier is not present in the trace inventory");
    return 2;
  }
  XWALK_HAL_TRACE_UID0(RPI .110, "xWalkConfig simulation started");
  const XWalkHal::int32 result =
      xwalk::hal::sim::runConfigSimulation(XWALK_CONFIG_SIMULATION_DATA_PATH);
  XWALK_HAL_TRACE_UID1(
      RPI .111, "xWalkConfig simulation completed with status %d", result);
  return result;
}
