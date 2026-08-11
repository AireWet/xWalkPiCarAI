/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the standalone xWalkGpio operation simulation.
 *
 * @details
 * Boots tracing, creates the build-selected Linux device implementation, binds
 * the public GPIO API, and executes the simulation handler without test code.
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

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarGpioDeviceFactory.h"
#include "xHal_Rpi5CarGpioHandler.h"
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarGpioSimulationArguments.h"
#include "xHal_Rpi5CarGpioSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"

/** @brief Contains the standalone GPIO simulation composition helper. */
namespace {

/** @brief Binds the public GPIO object and dispatches the simulation handler.
 */
XWalkHal::int32 runSimulation(xwalk::hal::XWalkGpioLinux &linuxBackend) {
  const XWalkHal::XWalkGpioCallbacks callbacks =
      XHAL_GPIO_CALLBACKS(xwalk::hal::XWalkGpioLinux);
  xwalk::hal::XWalkGpio gpio(&linuxBackend, callbacks, "LED");
  xwalk::hal::sim::XWalkGpioHandler handler;
  return handler.run(gpio);
}

} /* namespace */

/**
 * @brief Runs xWalkGpio operations through the build-selected device backend.
 * @param[in] argumentCount Number of process arguments, including the binary
 * name.
 * @param[in] argumentValues Non-null process argument array with
 * `argumentCount` entries.
 * @return Zero on success, two for invalid arguments or trace selection, and
 * another non-zero status for operation failure.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount,
                     XWalkHal::charpointer argumentValues[]) {
  const XWalkHal::filesystempath traceConfigurationPath(
      XWALK_GPIO_SIMULATION_TRACE_CONFIG_PATH);
  const XWalkHal::filesystempath traceLogPath(
      XWALK_GPIO_SIMULATION_TRACE_LOG_PATH);
  xwalk::hal::XWalkTrace::configureGlobal(traceConfigurationPath, traceLogPath);

  const xwalk::hal::sim::XWalkGpioSimulationArguments arguments(argumentCount,
                                                                argumentValues);
  const XWalkHal::boolean argumentsValid = arguments.valid();
  if (argumentsValid == false) {
    XWALK_HAL_ERROR(XWALK_EXCEPTION, "Invalid xWalkGpio simulation arguments");
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      argumentValues[0]);
    return 2;
  }

  const XWalkHal::boolean helpRequested = arguments.helpRequested();
  if (helpRequested) {
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      argumentValues[0]);
    XWALK_HAL_WARNING(XWALK_LOGIC, "Run GPIO LED output and input operations "
                                   "through the selected backend");
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
  XWALK_HAL_TRACE_UID0(RPI .080, "xWalkGpio simulation started");

  xwalk::hal::owningpointer<xwalk::hal::XWalkGpioDevice> device =
      xwalk::hal::sim::createGpioDevice();
  xwalk::hal::XWalkGpioLinux linuxBackend(
      *device, XWALK_GPIO_SIMULATION_DEVICE_PATH, {}, {}, 27U);
  const XWalkHal::int32 simulationResult = runSimulation(linuxBackend);
  XWALK_HAL_TRACE_UID1(RPI .082,
                       "xWalkGpio simulation completed with status %d",
                       simulationResult);
  return simulationResult;
}
