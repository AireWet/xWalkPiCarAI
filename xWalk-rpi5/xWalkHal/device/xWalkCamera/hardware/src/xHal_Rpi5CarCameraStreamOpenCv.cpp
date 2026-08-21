/******************************************************************************
 * @file        xHal_Rpi5CarCameraStreamOpenCv.cpp
 * @brief       Implements OpenCV encoded camera streaming for Linux.
 *
 * @details
 * Opens either a validated V4L2 device or a fixed libcamera GStreamer pipeline
 * and converts each acquired frame into a bounded-quality JPEG byte vector.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera OpenCV Backend
 *
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCameraStreamOpenCv.h"

#include "xHal_Rpi5CarTrace.h"

#include <opencv2/imgcodecs.hpp>

#include <string>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /** @brief Reports whether the production OpenCV camera is open. */
    static boolean systemIsOpened(contextpointer context, const cv::VideoCapture& camera) noexcept
    {
        static_cast<void>(context);
        return static_cast<boolean>(camera.isOpened());
    }

    /** @brief Opens the production OpenCV camera source. */
    static boolean systemOpen(contextpointer context, cv::VideoCapture& camera, stringview source, int apiPreference)
    {
        static_cast<void>(context);
        return static_cast<boolean>(camera.open(string(source), apiPreference));
    }

    /** @brief Sets one production OpenCV camera property. */
    static boolean systemSet(contextpointer context, cv::VideoCapture& camera, int property, float64 value)
    {
        static_cast<void>(context);
        return static_cast<boolean>(camera.set(property, value));
    }

    /** @brief Reads one production OpenCV frame. */
    static boolean systemRead(contextpointer context, cv::VideoCapture& camera, cv::Mat& frame)
    {
        static_cast<void>(context);
        return static_cast<boolean>(camera.read(frame));
    }

    /** @brief JPEG-encodes one production OpenCV frame. */
    static boolean systemEncode(contextpointer context, const cv::Mat& frame, uint32 jpegQuality, bytevector& jpeg)
    {
        static_cast<void>(context);
        const std::vector<int> parameters{cv::IMWRITE_JPEG_QUALITY, static_cast<int>(jpegQuality)};
        return static_cast<boolean>(cv::imencode(".jpg", frame, jpeg, parameters));
    }

    /** @brief Releases the production OpenCV camera. */
    static void systemRelease(contextpointer context, cv::VideoCapture& camera) noexcept
    {
        static_cast<void>(context);
        camera.release();
    }

    /**
     * @brief Resolves a callback context to its owning backend.
     * @param[in,out] context Non-null non-owning pointer to a live backend.
     * @return Referenced backend represented by `context`.
     * @throws std::invalid_argument If `context` is null.
     */
    XWalkCameraStreamOpenCv& XWalkCameraStreamOpenCv::backend(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Camera-stream OpenCV context is null");
        }
        return *static_cast<XWalkCameraStreamOpenCv*>(context);
    }

    /** @brief Returns the production OpenCV operation table. */
    XWalkCameraStreamOpenCvOperations XWalkCameraStreamOpenCv::systemOperations() noexcept
    {
        return {&systemIsOpened, &systemOpen, &systemSet, &systemRead, &systemEncode, &systemRelease};
    }

    /** @brief Validates a complete OpenCV operation table. */
    void XWalkCameraStreamOpenCv::validateOperations(const XWalkCameraStreamOpenCvOperations& selectedOperations)
    {
        const boolean complete = (selectedOperations.isOpened != nullptr) && (selectedOperations.open != nullptr) &&
                                 (selectedOperations.set != nullptr) && (selectedOperations.read != nullptr) &&
                                 (selectedOperations.encode != nullptr) && (selectedOperations.release != nullptr);
        if (complete == false)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Camera-stream OpenCV operation table is incomplete");
        }
    }

    /** @brief Builds the fixed libcamera GStreamer pipeline for validated dimensions. */
    string XWalkCameraStreamOpenCv::libcameraPipeline(const XWalkCameraStreamConfiguration& configuration)
    {
        return "libcamerasrc ! video/x-raw,format=NV12,width=" + std::to_string(configuration.widthPixels) +
               ",height=" + std::to_string(configuration.heightPixels) +
               " ! videoconvert ! video/x-raw,format=BGR ! appsink drop=true max-buffers=1 sync=false";
    }

    /** @brief Creates an idle production OpenCV camera backend. */
    XWalkCameraStreamOpenCv::XWalkCameraStreamOpenCv() : XWalkCameraStreamOpenCv(nullptr, systemOperations())
    {
    }

    /** @brief Creates an idle backend with injectable operations. */
    XWalkCameraStreamOpenCv::XWalkCameraStreamOpenCv(contextpointer selectedOperationContext,
                                                     const XWalkCameraStreamOpenCvOperations& selectedOperations)
        : operationContext(selectedOperationContext), operations(selectedOperations)
    {
        validateOperations(operations);
    }

    /**
     * @brief Releases the owned OpenCV camera when it remains open.
     */
    XWalkCameraStreamOpenCv::~XWalkCameraStreamOpenCv() noexcept
    {
        stopCamera(this);
    }

    /**
     * @brief Opens and configures the selected OpenCV camera source.
     * @param[in,out] context Non-null non-owning pointer to a live backend.
     * @param[in] configuration Validated camera source, dimensions, and timeout.
     * @return True if the camera is already open or opens successfully; otherwise false.
     * @throws std::invalid_argument If `context` is null.
     */
    boolean XWalkCameraStreamOpenCv::startCamera(contextpointer context,
                                                 const XWalkCameraStreamConfiguration& configuration)
    {
        XWalkCameraStreamOpenCv& provider = backend(context);
        const boolean cameraOpen = provider.operations.isOpened(provider.operationContext, provider.camera);
        if (cameraOpen)
        {
            return true;
        }
        int apiPreference = cv::CAP_V4L2;
        string source(configuration.source);
        if (configuration.backend == "v4l2")
        {
            source = configuration.source;
        }
        else
        {
            apiPreference = cv::CAP_GSTREAMER;
            source = libcameraPipeline(configuration);
        }
        const boolean cameraOpened =
            provider.operations.open(provider.operationContext, provider.camera, source, apiPreference);
        if (cameraOpened == false)
        {
            provider.operations.release(provider.operationContext, provider.camera);
            XWALK_HAL_ERROR(XWALK_EXCEPTION,
                            "Failed to open camera-stream backend " + configuration.backend + " source " +
                                configuration.source);
            return false;
        }
        if (configuration.backend == "v4l2")
        {
            static_cast<void>(provider.operations.set(
                provider.operationContext, provider.camera, cv::CAP_PROP_FRAME_WIDTH, configuration.widthPixels));
            static_cast<void>(provider.operations.set(
                provider.operationContext, provider.camera, cv::CAP_PROP_FRAME_HEIGHT, configuration.heightPixels));
        }
        static_cast<void>(provider.operations.set(
            provider.operationContext, provider.camera, cv::CAP_PROP_READ_TIMEOUT_MSEC, configuration.readTimeoutMs));
        return true;
    }

    /**
     * @brief Releases the selected OpenCV camera when it is open.
     * @param[in,out] context Nullable non-owning pointer to a live backend.
     */
    void XWalkCameraStreamOpenCv::stopCamera(contextpointer context) noexcept
    {
        if (context != nullptr)
        {
            XWalkCameraStreamOpenCv& provider = *static_cast<XWalkCameraStreamOpenCv*>(context);
            const boolean cameraOpen = provider.operations.isOpened(provider.operationContext, provider.camera);
            if (cameraOpen)
            {
                provider.operations.release(provider.operationContext, provider.camera);
            }
        }
    }

    /**
     * @brief Reads and JPEG-encodes one frame from the selected camera.
     * @param[in,out] context Non-null non-owning pointer to a live backend.
     * @param[in] configuration Validated camera and JPEG encoding settings.
     * @param[out] jpeg Complete encoded JPEG bytes, or an empty vector on failure.
     * @return True when a non-empty frame is encoded successfully; otherwise false.
     * @throws std::invalid_argument If `context` is null.
     */
    boolean XWalkCameraStreamOpenCv::captureJpeg(contextpointer context,
                                                 const XWalkCameraStreamConfiguration& configuration,
                                                 bytevector& jpeg)
    {
        XWalkCameraStreamOpenCv& provider = backend(context);
        cv::Mat frame;
        const boolean frameRead = provider.operations.read(provider.operationContext, provider.camera, frame);
        const boolean frameAvailable = static_cast<boolean>(frameRead && !frame.empty());
        if (frameAvailable == false)
        {
            jpeg.clear();
            stopCamera(context);
            XWALK_HAL_ERROR(XWALK_EXCEPTION,
                            "Failed to read the first or next camera-stream frame from " + configuration.backend +
                                " source " + configuration.source);
            return false;
        }
        const boolean encoded =
            provider.operations.encode(provider.operationContext, frame, configuration.jpegQuality, jpeg);
        if ((encoded == false) || jpeg.empty())
        {
            jpeg.clear();
            stopCamera(context);
            XWALK_HAL_ERROR(XWALK_EXCEPTION, "Failed to JPEG-encode a camera-stream frame");
            return false;
        }
        return true;
    }

    /**
     * @brief Creates the callback table used by `XWalkCameraStream`.
     * @return Complete callback table whose context must point to this live backend.
     */
    XWalkCameraStreamCallbacks XWalkCameraStreamOpenCv::callbacks() const noexcept
    {
        return {&startCamera, &stopCamera, &captureJpeg};
    }

} /* namespace xwalk::hal */
