/******************************************************************************
 * @file        xHal_Rpi5CarCameraStream.cpp
 * @brief       Implements backend-neutral encoded camera streaming.
 *
 * @details
 * Validates the callback boundary and forwards bounded camera lifecycle and
 * JPEG-frame requests to one caller-owned provider.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCameraStream.h"

#include "xHal_Rpi5CarTrace.h"

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
     * @brief Validates the callback table and bounded streaming profile.
     * @param[in] providerCallbacks Complete backend callback table to validate.
     * @param[in] settings Camera source, frame, encoding, and timeout settings to validate.
     * @throws std::invalid_argument If a callback, backend, or source is invalid.
     * @throws std::out_of_range If a numeric setting is outside its supported range.
     */
    void XWalkCameraStream::validate(const XWalkCameraStreamCallbacks& providerCallbacks,
                                     const XWalkCameraStreamConfiguration& settings)
    {
        const boolean callbacksIncomplete =
            static_cast<boolean>((providerCallbacks.start == nullptr) || (providerCallbacks.stop == nullptr) ||
                                 (providerCallbacks.capture == nullptr));
        if (callbacksIncomplete)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Camera-stream callbacks must be complete");
        }
        const boolean backendSupported = static_cast<boolean>(
            (settings.backend == "v4l2") || (settings.backend == "gstreamer") || (settings.backend == "automatic"));
        const boolean sourceInvalid =
            static_cast<boolean>(settings.source.empty() || (settings.source.find('\n') != string::npos) ||
                                 (settings.source.find('\r') != string::npos));
        if ((backendSupported == false) || sourceInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Camera-stream source is invalid");
        }
        const boolean dimensionsInvalid =
            static_cast<boolean>((settings.widthPixels < 16U) || (settings.widthPixels > 7'680U) ||
                                 (settings.heightPixels < 16U) || (settings.heightPixels > 4'320U));
        const boolean encodingInvalid =
            static_cast<boolean>((settings.jpegQuality == 0U) || (settings.jpegQuality > 100U) ||
                                 (settings.readTimeoutMs == 0U) || (settings.readTimeoutMs > 60'000U));
        if (dimensionsInvalid || encodingInvalid)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Camera-stream setting is outside its supported range");
        }
    }

    /**
     * @brief Binds one caller-owned encoded-camera backend.
     * @param[in,out] context Nullable backend context that must outlive this object.
     * @param[in] providerCallbacks Complete synchronous callback table to copy.
     * @param[in] settings Camera source, frame, encoding, and timeout settings to copy.
     * @throws std::invalid_argument If a callback, backend, or source is invalid.
     * @throws std::out_of_range If a numeric setting is outside its supported range.
     */
    XWalkCameraStream::XWalkCameraStream(contextpointer context,
                                         const XWalkCameraStreamCallbacks& providerCallbacks,
                                         const XWalkCameraStreamConfiguration& settings)
        : backendContext(context), callbacks(providerCallbacks), configuration(settings)
    {
        validate(callbacks, configuration);
    }

    /**
     * @brief Stops the backend when it remains active.
     */
    XWalkCameraStream::~XWalkCameraStream() noexcept
    {
        stop();
    }

    /**
     * @brief Starts the configured camera backend idempotently.
     * @return True when the backend is active; otherwise false.
     */
    boolean XWalkCameraStream::start()
    {
        if (startedValue == false)
        {
            startedValue = callbacks.start(backendContext, configuration);
        }
        return startedValue;
    }

    /**
     * @brief Captures one encoded JPEG frame from an active backend.
     * @param[out] jpeg Complete encoded JPEG bytes, or an empty vector on failure.
     * @return True when a frame is encoded successfully; otherwise false.
     */
    boolean XWalkCameraStream::capture(bytevector& jpeg)
    {
        if (startedValue == false)
        {
            jpeg.clear();
            return false;
        }
        return callbacks.capture(backendContext, configuration, jpeg);
    }

    /**
     * @brief Stops the configured camera backend idempotently.
     */
    void XWalkCameraStream::stop() noexcept
    {
        if (startedValue)
        {
            callbacks.stop(backendContext);
            startedValue = false;
        }
    }

    /**
     * @brief Reports whether camera startup completed.
     * @return True only while the backend is active.
     */
    boolean XWalkCameraStream::started() const noexcept
    {
        return startedValue;
    }

} /* namespace xwalk::hal */
