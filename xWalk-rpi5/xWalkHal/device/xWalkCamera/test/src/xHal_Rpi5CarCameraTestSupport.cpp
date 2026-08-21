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

    /** @brief Returns whether the injected OpenCV camera is open. */
    boolean openCvIsOpened(contextpointer context, const cv::VideoCapture& camera) noexcept
    {
        static_cast<void>(camera);
        return static_cast<OpenCvCameraStreamTestState*>(context)->opened;
    }

    /** @brief Records and simulates an OpenCV camera open operation. */
    boolean openCvOpen(contextpointer context, cv::VideoCapture& camera, stringview source, int apiPreference)
    {
        static_cast<void>(camera);
        OpenCvCameraStreamTestState& state = *static_cast<OpenCvCameraStreamTestState*>(context);
        state.source = source;
        state.apiPreference = apiPreference;
        state.opened = state.openResult;
        return state.openResult;
    }

    /** @brief Records an OpenCV camera property operation. */
    boolean openCvSet(contextpointer context, cv::VideoCapture& camera, int property, float64 value)
    {
        static_cast<void>(camera);
        OpenCvCameraStreamTestState& state = *static_cast<OpenCvCameraStreamTestState*>(context);
        ++state.setCount;
        if (property == cv::CAP_PROP_FRAME_WIDTH)
        {
            state.frameWidth = value;
        }
        else if (property == cv::CAP_PROP_FRAME_HEIGHT)
        {
            state.frameHeight = value;
        }
        else if (property == cv::CAP_PROP_READ_TIMEOUT_MSEC)
        {
            state.readTimeoutMs = value;
        }
        return true;
    }

    /** @brief Produces or rejects one simulated OpenCV frame. */
    boolean openCvRead(contextpointer context, cv::VideoCapture& camera, cv::Mat& frame)
    {
        static_cast<void>(camera);
        const OpenCvCameraStreamTestState& state = *static_cast<OpenCvCameraStreamTestState*>(context);
        if (state.readResult)
        {
            frame = cv::Mat::zeros(1, 1, CV_8UC3);
        }
        return state.readResult;
    }

    /** @brief Produces or rejects one simulated JPEG. */
    boolean openCvEncode(contextpointer context, const cv::Mat& frame, uint32 jpegQuality, bytevector& jpeg)
    {
        static_cast<void>(frame);
        static_cast<void>(jpegQuality);
        const OpenCvCameraStreamTestState& state = *static_cast<OpenCvCameraStreamTestState*>(context);
        if (state.encodeResult)
        {
            jpeg = {0xFFU, 0xD8U, 0x4FU, 0xFFU, 0xD9U};
        }
        return state.encodeResult;
    }

    /** @brief Records an OpenCV camera release operation. */
    void openCvRelease(contextpointer context, cv::VideoCapture& camera) noexcept
    {
        static_cast<void>(camera);
        OpenCvCameraStreamTestState& state = *static_cast<OpenCvCameraStreamTestState*>(context);
        state.opened = false;
        ++state.releaseCount;
    }

    /** @brief Creates the complete injectable OpenCV operation table. */
    XWalkCameraStreamOpenCvOperations openCvOperations() noexcept
    {
        return {&openCvIsOpened, &openCvOpen, &openCvSet, &openCvRead, &openCvEncode, &openCvRelease};
    }
} /* namespace xwalk::hal::test::camera */
