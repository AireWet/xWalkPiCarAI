/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the standalone device-free xWalkBoardControl simulation.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBoardControlSimulation.h"
#include "xHal_Rpi5CarBoardControlSimulationArguments.h"
#include "xHal_Rpi5CarBoardControlSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"
XWalkHal::int32 main(XWalkHal::int32 count, XWalkHal::charpointer values[])
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH);
    const xwalk::hal::sim::XWalkBoardControlSimulationArguments arguments(count, values);
    const XWalkHal::boolean argumentsValid = arguments.valid();
    if (argumentsValid == false)
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "Invalid xWalkBoardControl simulation arguments");
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", values[0]);
        return 2;
    }
    const XWalkHal::boolean helpRequested = arguments.helpRequested();
    if (helpRequested)
    {
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", values[0]);
        XWALK_HAL_WARNING(XWALK_LOGIC, "Trace selectors persist in XML and load on the next run");
        return 0;
    }
    const XWalkHal::boolean traceUpdateApplied = arguments.applyTraceUpdate();
    if (traceUpdateApplied == false)
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "The requested trace identifier is not present in the trace inventory");
        return 2;
    }
    XWALK_HAL_TRACE_UID0(RPI .331, "xWalkBoardControl simulation started");
    const XWalkHal::int32 result = xwalk::hal::sim::runBoardControlSimulation();
    XWALK_HAL_TRACE_UID1(RPI .332, "xWalkBoardControl simulation completed with status %d", result);
    return result;
}
