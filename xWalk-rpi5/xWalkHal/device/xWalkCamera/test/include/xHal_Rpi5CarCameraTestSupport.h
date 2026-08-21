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
#include "xHal_Rpi5CarCameraStreamOpenCv.h"

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

    /** @brief Records injected OpenCV provider operations without camera hardware. */
    struct OpenCvCameraStreamTestState
    {
            boolean opened{};           /**< Simulated OpenCV handle state. */
            boolean openResult{true};   /**< Result returned by the open operation. */
            boolean readResult{true};   /**< Result returned by the frame-read operation. */
            boolean encodeResult{true}; /**< Result returned by the JPEG encoder. */
            uint32 releaseCount{};      /**< Number of camera release operations. */
            uint32 setCount{};          /**< Number of camera property operations. */
            string source{};            /**< Most recently opened source or pipeline. */
            int apiPreference{};        /**< Most recently selected OpenCV capture API. */
            float64 frameWidth{};       /**< Requested V4L2 width, or zero for libcamera. */
            float64 frameHeight{};      /**< Requested V4L2 height, or zero for libcamera. */
            float64 readTimeoutMs{};    /**< Most recently requested read timeout. */
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

    /** @brief Returns whether the injected OpenCV camera is open. */
    boolean openCvIsOpened(contextpointer context, const cv::VideoCapture& camera) noexcept;

    /** @brief Records and simulates an OpenCV camera open operation. */
    boolean openCvOpen(contextpointer context, cv::VideoCapture& camera, stringview source, int apiPreference);

    /** @brief Records an OpenCV camera property operation. */
    boolean openCvSet(contextpointer context, cv::VideoCapture& camera, int property, float64 value);

    /** @brief Produces or rejects one simulated OpenCV frame. */
    boolean openCvRead(contextpointer context, cv::VideoCapture& camera, cv::Mat& frame);

    /** @brief Produces or rejects one simulated JPEG. */
    boolean openCvEncode(contextpointer context, const cv::Mat& frame, uint32 jpegQuality, bytevector& jpeg);

    /** @brief Records an OpenCV camera release operation. */
    void openCvRelease(contextpointer context, cv::VideoCapture& camera) noexcept;

    /** @brief Creates the complete injectable OpenCV operation table. */
    XWalkCameraStreamOpenCvOperations openCvOperations() noexcept;
} /* namespace xwalk::hal::test::camera */

#endif /* XHAL_RPI5CAR_CAMERA_TEST_SUPPORT_H */
