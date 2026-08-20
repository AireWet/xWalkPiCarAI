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

    /**
     * @brief Starts the in-memory encoded-camera backend.
     * @param[in,out] context Non-null non-owning pointer to `CameraStreamTestState`.
     * @param[in] configuration Validated streaming settings copied into the state.
     * @return Configured fake startup result.
     */
    boolean startStream(contextpointer context, const XWalkCameraStreamConfiguration& configuration)
    {
        CameraStreamTestState& state = *static_cast<CameraStreamTestState*>(context);
        state.configuration = configuration;
        state.started = state.startResult;
        return state.startResult;
    }

    /**
     * @brief Stops the in-memory encoded-camera backend.
     * @param[in,out] context Non-null non-owning pointer to `CameraStreamTestState`.
     */
    void stopStream(contextpointer context) noexcept
    {
        CameraStreamTestState& state = *static_cast<CameraStreamTestState*>(context);
        state.started = false;
    }

    /**
     * @brief Produces one deterministic marker-bearing JPEG-like frame.
     * @param[in,out] context Non-null non-owning pointer to `CameraStreamTestState`.
     * @param[in] configuration Validated streaming settings copied into the state.
     * @param[out] jpeg Fake encoded frame bytes, or an empty vector on failure.
     * @return Configured fake capture result.
     */
    boolean captureStream(contextpointer context, const XWalkCameraStreamConfiguration& configuration, bytevector& jpeg)
    {
        CameraStreamTestState& state = *static_cast<CameraStreamTestState*>(context);
        state.configuration = configuration;
        if (state.captureResult == false)
        {
            jpeg.clear();
            return false;
        }
        ++state.captureCount;
        jpeg = {0xFFU, 0xD8U, 0x45U, 0xFFU, 0xD9U};
        return true;
    }

    /**
     * @brief Creates the complete in-memory encoded-camera callback table.
     * @return Callback table requiring a `CameraStreamTestState` context.
     */
    XWalkCameraStreamCallbacks streamCallbacks() noexcept
    {
        return {&startStream, &stopStream, &captureStream};
    }
} /* namespace xwalk::hal::test::camera */
