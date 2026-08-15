/******************************************************************************
 * @file        xHal_Rpi5CarCameraSimulation.cpp
 * @brief       Implements the device-free xWalkCamera simulation.
 * @project     xWalk Firmware
 * @module      xWalkCamera Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarCameraSimulation.h"
#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarCameraHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    /** @brief Exercises validated camera capture without device, process, or file
     * access. */
    int32 runCameraSimulation()
    {
        XWalkCameraHostStub backend;
        const XWalkCameraConfiguration configuration{1'280U, 720U, 2'000U};
        XWalkCamera camera(&backend, &XWalkCameraHostStub::capture, configuration);
        const string outputPath = camera.capture("simulation.jpg");
        const boolean valid =
            (outputPath == "simulation.jpg") && (backend.captureCount() == 1U) &&
            (backend.outputPath() == "simulation.jpg") && (backend.configuration().widthPixels == 1'280U) &&
            (backend.configuration().heightPixels == 720U) && (backend.configuration().timeoutMs == 2'000U);
        XWALK_HAL_TRACE_UID0(RPI .219, "xWalkCamera host simulation completed");
        return valid ? 0 : 1;
    }
} /* namespace xwalk::hal::sim */
