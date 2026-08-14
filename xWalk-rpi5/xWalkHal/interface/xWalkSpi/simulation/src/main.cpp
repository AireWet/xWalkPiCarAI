/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the standalone xWalkSpi operation simulation.
 *
 * @details
 * Boots tracing, creates the build-selected Linux device implementation, binds
 * the public SPI API, and executes the simulation handler without test code.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Simulation
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
#include "xHal_Rpi5CarSpiDeviceFactory.h"
#include "xHal_Rpi5CarSpiHandler.h"
#include "xHal_Rpi5CarSpiLinux.h"
#include "xHal_Rpi5CarSpiSimulationArguments.h"
#include "xHal_Rpi5CarSpiSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"

/** @brief Contains the standalone SPI simulation composition helper. */
namespace {

/** @brief Binds the public SPI object and dispatches the simulation handler. */
XWalkHal::int32 runSimulation(xwalk::hal::XWalkSpiLinux &linuxBackend) {
  xwalk::hal::XWalkSpi spi(
      &linuxBackend, XHAL_SPI_TRANSFER_CALLBACK(xwalk::hal::XWalkSpiLinux));
  xwalk::hal::sim::XWalkSpiHandler handler;
  return handler.run(spi);
}

} /* namespace */

/**
 * @brief Runs xWalkSpi operations through the build-selected device backend.
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
      XWALK_SPI_SIMULATION_TRACE_CONFIG_PATH);
  const XWalkHal::filesystempath traceLogPath(
      XWALK_SPI_SIMULATION_TRACE_LOG_PATH);
  xwalk::hal::XWalkTrace::configureGlobal(traceConfigurationPath, traceLogPath);

  const xwalk::hal::sim::XWalkSpiSimulationArguments arguments(argumentCount,
                                                               argumentValues);
  const XWalkHal::boolean argumentsValid = arguments.valid();
  if (argumentsValid == false) {
    XWALK_HAL_ERROR(XWALK_EXCEPTION, "Invalid xWalkSpi simulation arguments");
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      argumentValues[0]);
    return 2;
  }

  const XWalkHal::boolean helpRequested = arguments.helpRequested();
  if (helpRequested) {
    XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]",
                      argumentValues[0]);
    XWALK_HAL_WARNING(
        XWALK_LOGIC, "Run the build-selected xWalkSpi full-duplex transaction");
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
  XWALK_HAL_TRACE_UID0(RPI .059, "xWalkSpi simulation started");

  xwalk::hal::owningpointer<xwalk::hal::XWalkSpiDevice> device =
      xwalk::hal::sim::createSpiDevice();
  xwalk::hal::XWalkSpiLinux linuxBackend(*device,
                                         XWALK_SPI_SIMULATION_DEVICE_PATH);
  const XWalkHal::int32 simulationResult = runSimulation(linuxBackend);
  XWALK_HAL_TRACE_UID1(RPI .061, "xWalkSpi simulation completed with status %d",
                       simulationResult);
  return simulationResult;
}
