/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the standalone xWalkI2c operation simulation.
 *
 * @details
 * Boots tracing, creates the build-selected Linux device implementation, binds
 * the public I2C API, and executes the simulation handler without test code.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-09
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

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarI2cDeviceFactory.h"
#include "xHal_Rpi5CarI2cHandler.h"
#include "xHal_Rpi5CarI2cLinux.h"
#include "xHal_Rpi5CarI2cSimulationArguments.h"
#include "xHal_Rpi5CarI2cSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

    /** @brief Binds the public I2C object and dispatches the simulation handler. */
    XWalkHal::int32 runSimulation(xwalk::hal::XWalkI2cLinux& linuxBackend)
    {
        xwalk::hal::XWalkI2c i2c{&linuxBackend,
                                 XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
                                 XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
                                 XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux),
                                 XHAL_I2C_READ_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
                                 XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux)};
        xwalk::hal::sim::XWalkI2cHandler handler;
        return handler.run(i2c);
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs xWalkI2c operations through the build-selected device backend.
 * @param[in] argumentCount Number of process arguments, including the binary
 * name.
 * @param[in] argumentValues Non-null process argument array with
 * `argumentCount` entries.
 * @return Zero when the complete operation sequence succeeds, two for invalid
 * arguments or trace selection, and another non-zero status for operation
 * failure.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer argumentValues[])
{
    const xwalk::hal::sim::XWalkI2cSimulationArguments arguments(argumentCount, argumentValues);
    const XWalkHal::boolean argumentsValid = arguments.valid();
    if (argumentsValid == false)
    {
        std::fprintf(stderr,
                     "Usage: %s [--help | --trace <selector>]\n"
                     "Try '%s --help' for complete trace-control help.\n",
                     argumentValues[0],
                     argumentValues[0]);
        return 2;
    }

    const XWalkHal::boolean helpRequested = arguments.helpRequested();
    if (helpRequested)
    {
        std::fprintf(stdout,
                     "Usage: %s [--help | --trace <selector>]\n\n"
                     "Run the build-selected xWalkI2c operation sequence.\n\n"
                     "Options:\n"
                     "  -h, --help                 Show this help without opening I2C "
                     "hardware.\n"
                     "  --trace RPI.<digits>.enable\n"
                     "                             Enable one scanner-known trace ID.\n"
                     "  --trace RPI.<digits>.disable\n"
                     "                             Disable one scanner-known trace ID.\n"
                     "  --trace RPI.enable         Enable the complete RPI module.\n"
                     "  --trace RPI.disable        Disable the complete RPI module.\n"
                     "  --trace all.enable         Enable every scanner-known trace ID.\n"
                     "  --trace all.disable        Disable every scanner-known trace ID.\n"
                     "  --trace FILE.json          Apply grouped trace settings.\n\n"
                     "Trace IDs use TAG.<digits> and must be unique across the compiled "
                     "project.\n"
                     "A duplicate trace ID fails metadata generation and compilation.\n"
                     "New traces are disabled; saved XML states load automatically.\n",
                     argumentValues[0]);
        return 0;
    }

    const XWalkHal::filesystempath traceConfigurationPath(XWALK_I2C_SIMULATION_TRACE_CONFIG_PATH);
    const XWalkHal::filesystempath traceLogPath(XWALK_I2C_SIMULATION_TRACE_LOG_PATH);
    xwalk::hal::XWalkTrace::configureGlobal(traceConfigurationPath, traceLogPath);
    const XWalkHal::boolean traceUpdateApplied = arguments.applyTraceUpdate();
    if (traceUpdateApplied == false)
    {
        std::fprintf(stderr,
                     "The requested trace identifier is not present in the "
                     "trace inventory.\n");
        return 2;
    }
    XWALK_HAL_TRACE_UID0(RPI .043, "xWalkI2c trace boot completed");
    XWALK_HAL_TRACE_UID0(RPI .031, "xWalkI2c simulation started");

    xwalk::hal::owningpointer<xwalk::hal::XWalkI2cDevice> device = xwalk::hal::sim::createI2cDevice();
    xwalk::hal::XWalkI2cLinux linuxBackend(*device, XWALK_I2C_SIMULATION_DEVICE_PATH, 1U);

    const XWalkHal::int32 simulationResult = runSimulation(linuxBackend);
    XWALK_HAL_TRACE_UID1(RPI .032, "xWalkI2c simulation completed with status %d", simulationResult);
    return simulationResult;
}
