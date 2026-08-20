/******************************************************************************
 * @file        xHal_Rpi5CarCameraStreamOpenCv.h
 * @brief       Declares OpenCV encoded camera streaming for Linux.
 *
 * @details
 * Owns one OpenCV camera and exposes its lifecycle and JPEG encoding through
 * the backend-neutral xWalk camera-stream callback boundary.
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

#ifndef XHAL_RPI5CAR_CAMERA_STREAM_OPENCV_H
#define XHAL_RPI5CAR_CAMERA_STREAM_OPENCV_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCameraStream.h"

#include <opencv2/videoio.hpp>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkCameraStreamOpenCv
     * @brief Owns one OpenCV camera used for synchronous JPEG streaming.
     *
     * @details
     * Callback contexts are non-owning pointers to this object. The backend
     * must outlive the `XWalkCameraStream` that uses the returned callback table.
     */
    class XWalkCameraStreamOpenCv final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief OpenCV camera handle owned and released by this backend. */
            cv::VideoCapture camera{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Resolves a callback context to its owning backend.
             * @param[in,out] context Non-null non-owning pointer to a live backend.
             * @return Referenced backend represented by `context`.
             * @throws std::invalid_argument If `context` is null.
             */
            static XWalkCameraStreamOpenCv& backend(contextpointer context);

            /**
             * @brief Opens and configures the selected OpenCV camera source.
             * @param[in,out] context Non-null non-owning pointer to a live backend.
             * @param[in] configuration Validated camera source, dimensions, and timeout.
             * @return True if the camera is already open or opens successfully; otherwise false.
             * @throws std::invalid_argument If `context` is null.
             */
            static boolean startCamera(contextpointer context, const XWalkCameraStreamConfiguration& configuration);

            /**
             * @brief Releases the selected OpenCV camera when it is open.
             * @param[in,out] context Nullable non-owning pointer to a live backend.
             */
            static void stopCamera(contextpointer context) noexcept;

            /**
             * @brief Reads and JPEG-encodes one frame from the selected camera.
             * @param[in,out] context Non-null non-owning pointer to a live backend.
             * @param[in] configuration Validated camera and JPEG encoding settings.
             * @param[out] jpeg Complete encoded JPEG bytes, or an empty vector on failure.
             * @return True when a non-empty frame is encoded successfully; otherwise false.
             * @throws std::invalid_argument If `context` is null.
             */
            static boolean
            captureJpeg(contextpointer context, const XWalkCameraStreamConfiguration& configuration, bytevector& jpeg);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /** @brief Creates an idle OpenCV camera backend without opening hardware. */
            XWalkCameraStreamOpenCv() = default;

            /** @brief Releases the owned OpenCV camera when it remains open. */
            ~XWalkCameraStreamOpenCv() noexcept;

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            XWalkCameraStreamOpenCv(const XWalkCameraStreamOpenCv&) = delete;
            XWalkCameraStreamOpenCv(XWalkCameraStreamOpenCv&&) = delete;
            XWalkCameraStreamOpenCv& operator=(const XWalkCameraStreamOpenCv&) = delete;
            XWalkCameraStreamOpenCv& operator=(XWalkCameraStreamOpenCv&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Creates the callback table used by `XWalkCameraStream`.
             * @return Complete callback table whose context must point to this live backend.
             */
            XWalkCameraStreamCallbacks callbacks() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_CAMERA_STREAM_OPENCV_H */
