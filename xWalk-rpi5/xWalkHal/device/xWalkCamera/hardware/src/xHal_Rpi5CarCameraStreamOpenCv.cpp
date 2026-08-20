/******************************************************************************
 * @file        xHal_Rpi5CarCameraStreamOpenCv.cpp
 * @brief       Implements OpenCV encoded camera streaming for Linux.
 *
 * @details
 * Opens a configured V4L2, GStreamer, or automatic OpenCV source and converts
 * each acquired frame into a bounded-quality JPEG byte vector.
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

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

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
        const boolean cameraOpen = static_cast<boolean>(provider.camera.isOpened());
        if (cameraOpen)
        {
            return true;
        }
        int apiPreference = cv::CAP_ANY;
        if (configuration.backend == "v4l2")
        {
            apiPreference = cv::CAP_V4L2;
        }
        else if (configuration.backend == "gstreamer")
        {
            apiPreference = cv::CAP_GSTREAMER;
        }
        const boolean cameraOpened = provider.camera.open(configuration.source, apiPreference);
        if (cameraOpened == false)
        {
            return false;
        }
        static_cast<void>(provider.camera.set(cv::CAP_PROP_FRAME_WIDTH, configuration.widthPixels));
        static_cast<void>(provider.camera.set(cv::CAP_PROP_FRAME_HEIGHT, configuration.heightPixels));
        static_cast<void>(provider.camera.set(cv::CAP_PROP_READ_TIMEOUT_MSEC, configuration.readTimeoutMs));
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
            const boolean cameraOpen = static_cast<boolean>(provider.camera.isOpened());
            if (cameraOpen)
            {
                provider.camera.release();
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
        const boolean frameRead = provider.camera.read(frame);
        const boolean frameAvailable = static_cast<boolean>(frameRead && !frame.empty());
        if (frameAvailable == false)
        {
            jpeg.clear();
            return false;
        }
        const std::vector<int> parameters{cv::IMWRITE_JPEG_QUALITY, static_cast<int>(configuration.jpegQuality)};
        return static_cast<boolean>(cv::imencode(".jpg", frame, jpeg, parameters));
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
