/******************************************************************************
 * @file        main.cpp
 * @brief       Runs the standalone device-free xWalkServo simulation.
 * @project     xWalk Firmware
 * @module      xWalkServo Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarServoSimulation.h"
#include "xHal_Rpi5CarServoSimulationArguments.h"
#include "xHal_Rpi5CarServoSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer argumentValues[])
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_SERVO_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_SERVO_SIMULATION_TRACE_LOG_PATH);
    const xwalk::hal::sim::XWalkServoSimulationArguments arguments(argumentCount, argumentValues);
    if (arguments.valid() == false)
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "Invalid xWalkServo simulation arguments");
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", argumentValues[0]);
        return 2;
    }
    if (arguments.helpRequested())
    {
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", argumentValues[0]);
        XWALK_HAL_WARNING(XWALK_LOGIC, "Trace selectors persist in XML and load on the next run");
        return 0;
    }
    if (arguments.applyTraceUpdate() == false)
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "The requested trace identifier is not present in the trace inventory");
        return 2;
    }
    XWALK_HAL_TRACE_UID0(RPI .188, "xWalkServo simulation started");
    const XWalkHal::int32 result = xwalk::hal::sim::runServoSimulation();
    XWALK_HAL_TRACE_UID1(RPI .189, "xWalkServo simulation completed with status %d", result);
    return result;
}
