/******************************************************************************
 * @file        xHal_Rpi5CarCameraStream.h
 * @brief       Declares backend-neutral encoded camera streaming.
 *
 * @details
 * Validates one caller-owned streaming backend and exposes synchronous camera
 * lifecycle and JPEG-frame acquisition without owning platform resources.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera
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

#ifndef XHAL_RPI5CAR_CAMERA_STREAM_H
#define XHAL_RPI5CAR_CAMERA_STREAM_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

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
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkCameraStreamConfiguration
     * @brief Stores one bounded encoded-camera streaming profile.
     */
    struct XWalkCameraStreamConfiguration
    {
            /** @brief OpenCV backend name: `v4l2`, `gstreamer`, or `automatic`. */
            string backend{"v4l2"};
            /** @brief Non-empty camera device path or GStreamer pipeline. */
            string source{"/dev/video0"};
            /** @brief Requested frame width from 16 through 7,680 pixels. */
            uint32 widthPixels{640U};
            /** @brief Requested frame height from 16 through 4,320 pixels. */
            uint32 heightPixels{480U};
            /** @brief JPEG encoding quality from 1 through 100 percent. */
            uint32 jpegQuality{80U};
            /** @brief Maximum frame-read wait from 1 through 60,000 milliseconds. */
            uint32 readTimeoutMs{1'000U};
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Opens one caller-owned encoded-camera backend.
     * @param[in,out] context Nullable non-owning backend context that outlives callback use.
     * @param[in] configuration Validated camera source, frame, encoding, and timeout settings.
     * @return True when the camera is ready for capture; otherwise false.
     */
    using camerastreamstartcallback = boolean (*)(contextpointer context,
                                                  const XWalkCameraStreamConfiguration& configuration);

    /**
     * @brief Closes one caller-owned encoded-camera backend without throwing.
     * @param[in,out] context Nullable non-owning backend context that outlives callback use.
     */
    using camerastreamstopcallback = void (*)(contextpointer context) noexcept;

    /**
     * @brief Acquires one complete JPEG frame from a caller-owned backend.
     * @param[in,out] context Nullable non-owning backend context that outlives callback use.
     * @param[in] configuration Validated camera source, frame, encoding, and timeout settings.
     * @param[out] jpeg Complete encoded JPEG bytes, or an empty vector on failure.
     * @return True when one frame is encoded successfully; otherwise false.
     */
    using camerastreamcapturecallback = boolean (*)(contextpointer context,
                                                    const XWalkCameraStreamConfiguration& configuration,
                                                    bytevector& jpeg);

    /**
     * @struct XWalkCameraStreamCallbacks
     * @brief Groups the complete backend boundary for encoded camera streaming.
     */
    struct XWalkCameraStreamCallbacks
    {
            /** @brief Non-null synchronous camera-start operation. */
            camerastreamstartcallback start{nullptr};
            /** @brief Non-null non-throwing camera-stop operation. */
            camerastreamstopcallback stop{nullptr};
            /** @brief Non-null synchronous JPEG-frame acquisition operation. */
            camerastreamcapturecallback capture{nullptr};
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkCameraStream
     * @brief Provides validated encoded camera lifecycle without owning hardware.
     *
     * @details
     * Stores a nullable non-owning callback context, copies its callback table
     * and configuration, and serially forwards start, capture, and stop calls.
     * The caller-owned backend must outlive this object.
     */
    class XWalkCameraStream final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Nullable non-owning backend context that must outlive this object. */
            contextpointer backendContext{nullptr};
            /** @brief Complete validated backend callbacks copied at construction. */
            XWalkCameraStreamCallbacks callbacks{};
            /** @brief Validated camera and encoding settings copied at construction. */
            XWalkCameraStreamConfiguration configuration{};
            /** @brief True after successful startup and before the matching stop. */
            boolean startedValue{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Validates the callback table and bounded streaming profile.
             * @param[in] providerCallbacks Complete backend callback table to validate.
             * @param[in] settings Camera source, frame, encoding, and timeout settings to validate.
             * @throws std::invalid_argument If a callback, backend, or source is invalid.
             * @throws std::out_of_range If a numeric setting is outside its supported range.
             */
            static void validate(const XWalkCameraStreamCallbacks& providerCallbacks,
                                 const XWalkCameraStreamConfiguration& settings);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Binds one caller-owned encoded-camera backend.
             * @param[in,out] context Nullable backend context that must outlive this object.
             * @param[in] providerCallbacks Complete synchronous callback table to copy.
             * @param[in] settings Camera source, frame, encoding, and timeout settings to copy.
             * @throws std::invalid_argument If a callback, backend, or source is invalid.
             * @throws std::out_of_range If a numeric setting is outside its supported range.
             */
            XWalkCameraStream(contextpointer context,
                              const XWalkCameraStreamCallbacks& providerCallbacks,
                              const XWalkCameraStreamConfiguration& settings = {});

            /** @brief Stops the backend when it remains active. */
            ~XWalkCameraStream() noexcept;

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            XWalkCameraStream(const XWalkCameraStream&) = delete;
            XWalkCameraStream(XWalkCameraStream&&) = delete;
            XWalkCameraStream& operator=(const XWalkCameraStream&) = delete;
            XWalkCameraStream& operator=(XWalkCameraStream&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Starts the configured camera backend idempotently.
             * @return True when the backend is active; otherwise false.
             */
            boolean start();

            /**
             * @brief Captures one encoded JPEG frame from an active backend.
             * @param[out] jpeg Complete encoded JPEG bytes, or an empty vector on failure.
             * @return True when a frame is encoded successfully; otherwise false.
             */
            boolean capture(bytevector& jpeg);

            /** @brief Stops the configured camera backend idempotently. */
            void stop() noexcept;

            /**
             * @brief Reports whether camera startup completed.
             * @return True only while the backend is active.
             */
            boolean started() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_CAMERA_STREAM_H */
