/******************************************************************************
 * @file        xHal_Rpi5CarCameraHostStub.cpp
 * @brief       Implements the device-free xWalkCamera host stub.
 * @project     xWalk Firmware
 * @module      xWalkCamera Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarCameraHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    /** @brief Records one successful camera request without device or file access.
     */
    boolean XWalkCameraHostStub::capture(contextpointer context,
                                         stringview outputPath,
                                         const XWalkCameraConfiguration& configuration)
    {
        XWalkCameraHostStub& self = *static_cast<XWalkCameraHostStub*>(context);
        ++self.captureCountValue;
        self.outputPathValue = outputPath;
        self.configurationValue = configuration;
        XWALK_HAL_TRACE_UID0(RPI .218, "Camera host stub received a capture request");
        return true;
    }

    /** @brief Returns the number of in-memory capture requests. */
    uint32 XWalkCameraHostStub::captureCount() const noexcept
    {
        return captureCountValue;
    }
    /** @brief Returns the most recent in-memory destination path. */
    stringview XWalkCameraHostStub::outputPath() const noexcept
    {
        return outputPathValue;
    }
    /** @brief Returns the most recent in-memory capture settings. */
    const XWalkCameraConfiguration& XWalkCameraHostStub::configuration() const noexcept
    {
        return configurationValue;
    }
} /* namespace xwalk::hal::sim */
