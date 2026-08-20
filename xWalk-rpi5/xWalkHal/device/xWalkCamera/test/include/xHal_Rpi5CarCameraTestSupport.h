/******************************************************************************
 * @file        xHal_Rpi5CarCameraTestSupport.h
 * @brief       Declares reusable xWalkCamera host-test support.
 *
 * @details
 * Provides named in-memory capture state and callbacks without accessing a
 * camera device, process, or filesystem.
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
#ifndef XHAL_RPI5CAR_CAMERA_TEST_SUPPORT_H
#define XHAL_RPI5CAR_CAMERA_TEST_SUPPORT_H

#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarCameraStream.h"

/** @brief Contains reusable xWalkCamera host-test support. */
namespace xwalk::hal::test::camera
{
    /** @brief Records one in-memory camera capture request. */
    struct CameraTestState
    {
            uint32 captureCount{};                    /**< Number of capture callback invocations. */
            string outputPath{};                      /**< Most recently requested destination path. */
            XWalkCameraConfiguration configuration{}; /**< Most recently requested settings. */
            boolean result{true};                     /**< Result returned to the camera interface. */
    };

    /**
     * @struct CameraStreamTestState
     * @brief Records deterministic encoded-camera lifecycle and capture state.
     */
    struct CameraStreamTestState
    {
            /** @brief True while the fake encoded-camera backend is active. */
            boolean started{};
            /** @brief Selects whether fake camera startup succeeds. */
            boolean startResult{true};
            /** @brief Selects whether fake JPEG capture succeeds. */
            boolean captureResult{true};
            /** @brief Number of successful fake JPEG acquisitions. */
            uint32 captureCount{};
            /** @brief Most recently observed streaming configuration. */
            XWalkCameraStreamConfiguration configuration{};
    };

    /**
     * @brief Records one camera request without accessing a physical device.
     * @param[in,out] context Non-null, non-owning pointer to `CameraTestState`.
     * @param[in] outputPath Non-empty destination supplied by the camera.
     * @param[in] configuration Validated capture settings copied into the state.
     * @return The configured in-memory result.
     */
    boolean captureImage(contextpointer context, stringview outputPath, const XWalkCameraConfiguration& configuration);

    /**
     * @brief Starts the in-memory encoded-camera backend.
     * @param[in,out] context Non-null non-owning pointer to `CameraStreamTestState`.
     * @param[in] configuration Validated streaming settings copied into the state.
     * @return Configured fake startup result.
     */
    boolean startStream(contextpointer context, const XWalkCameraStreamConfiguration& configuration);

    /**
     * @brief Stops the in-memory encoded-camera backend.
     * @param[in,out] context Non-null non-owning pointer to `CameraStreamTestState`.
     */
    void stopStream(contextpointer context) noexcept;

    /**
     * @brief Produces one deterministic marker-bearing JPEG-like frame.
     * @param[in,out] context Non-null non-owning pointer to `CameraStreamTestState`.
     * @param[in] configuration Validated streaming settings copied into the state.
     * @param[out] jpeg Fake encoded frame bytes, or an empty vector on failure.
     * @return Configured fake capture result.
     */
    boolean
    captureStream(contextpointer context, const XWalkCameraStreamConfiguration& configuration, bytevector& jpeg);

    /**
     * @brief Creates the complete in-memory encoded-camera callback table.
     * @return Callback table requiring a `CameraStreamTestState` context.
     */
    XWalkCameraStreamCallbacks streamCallbacks() noexcept;
} /* namespace xwalk::hal::test::camera */

#endif /* XHAL_RPI5CAR_CAMERA_TEST_SUPPORT_H */
