/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the standalone device-free xWalkGPT simulation.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarGptSimulation.h"
#include "xHal_Rpi5CarGptSimulationArguments.h"
#include "xHal_Rpi5CarGptSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"
XWalkHal::int32 main(XWalkHal::int32 count, XWalkHal::charpointer values[])
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_GPT_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_GPT_SIMULATION_TRACE_LOG_PATH);
    const xwalk::hal::sim::XWalkGptSimulationArguments arguments(count, values);
    if (!arguments.valid())
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "Invalid xWalkGPT simulation arguments");
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", values[0]);
        return 2;
    }
    if (arguments.helpRequested())
    {
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", values[0]);
        XWALK_HAL_WARNING(XWALK_LOGIC, "Trace selectors persist in XML and load on the next run");
        return 0;
    }
    if (!arguments.applyTraceUpdate())
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "The requested trace identifier is not present in the trace inventory");
        return 2;
    }
    XWALK_HAL_TRACE_UID0(RPI .364, "xWalkGPT simulation started");
    const XWalkHal::int32 result = xwalk::hal::sim::runGptSimulation();
    XWALK_HAL_TRACE_UID1(RPI .365, "xWalkGPT simulation completed with status %d", result);
    return result;
}
