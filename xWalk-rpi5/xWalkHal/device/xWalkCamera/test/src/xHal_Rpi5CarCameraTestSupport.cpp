/******************************************************************************
 * @file        xHal_Rpi5CarCameraTestSupport.cpp
 * @brief       Implements reusable xWalkCamera host-test support.
 *
 * @details
 * Records validated capture requests entirely in memory.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera Host Test
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
#include "xHal_Rpi5CarCameraTestSupport.h"

/** @brief Contains reusable xWalkCamera host-test support. */
namespace xwalk::hal::test::camera
{
    /**
     * @brief Records one camera request without accessing a physical device.
     * @param[in,out] context Non-null, non-owning pointer to `CameraTestState`.
     * @param[in] outputPath Non-empty destination supplied by the camera.
     * @param[in] configuration Validated capture settings copied into the state.
     * @return The configured in-memory result.
     */
    boolean captureImage(contextpointer context, stringview outputPath, const XWalkCameraConfiguration& configuration)
    {
        CameraTestState& state = *static_cast<CameraTestState*>(context);
        ++state.captureCount;
        state.outputPath = outputPath;
        state.configuration = configuration;
        return state.result;
    }
} /* namespace xwalk::hal::test::camera */
