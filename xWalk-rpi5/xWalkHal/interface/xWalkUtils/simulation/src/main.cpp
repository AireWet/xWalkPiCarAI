/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the standalone side-effect-free xWalkUtils simulation.
 * @details     Loads persistent tracing and dispatches the in-memory host
 *composition.
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarUtilsSimulation.h"
#include "xHal_Rpi5CarUtilsSimulationArguments.h"
#include "xHal_Rpi5CarUtilsSimulationConfig.h"
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer argumentValues[])
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_UTILS_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_UTILS_SIMULATION_TRACE_LOG_PATH);
    const xwalk::hal::sim::XWalkUtilsSimulationArguments arguments(argumentCount, argumentValues);
    const XWalkHal::boolean argumentsValid = arguments.valid();
    if (argumentsValid == false)
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "Invalid xWalkUtils simulation arguments");
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", argumentValues[0]);
        return 2;
    }
    const XWalkHal::boolean helpRequested = arguments.helpRequested();
    if (helpRequested)
    {
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", argumentValues[0]);
        XWALK_HAL_WARNING(XWALK_LOGIC, "Saved XML trace states load automatically on the next run");
        return 0;
    }
    const XWalkHal::boolean traceUpdateApplied = arguments.applyTraceUpdate();
    if (traceUpdateApplied == false)
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "The requested trace identifier is not present in the trace inventory");
        return 2;
    }
    XWALK_HAL_TRACE_UID0(RPI .137, "xWalkUtils simulation started");
    const XWalkHal::int32 result = xwalk::hal::sim::runUtilsSimulation();
    XWALK_HAL_TRACE_UID1(RPI .138, "xWalkUtils simulation completed with status %d", result);
    return result;
}
