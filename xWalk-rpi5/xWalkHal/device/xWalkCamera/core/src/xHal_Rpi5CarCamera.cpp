/******************************************************************************
 * @file        xHal_Rpi5CarCamera.cpp
 * @brief       Implements backend-neutral still-image capture.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarCamera.h"

#include "xHal_Rpi5CarTrace.h"

namespace xwalk::hal
{

    boolean validCameraSourceString(stringview source) noexcept
    {
        constexpr size MAXIMUM_SOURCE_BYTES{256U};
        if (source.empty() || (source.size() > MAXIMUM_SOURCE_BYTES) || (source.find('\0') != stringview::npos) ||
            (source.find('\r') != stringview::npos) || (source.find('\n') != stringview::npos))
        {
            return false;
        }
        if ((source == "csi") || (source == "usb"))
        {
            return true;
        }
        constexpr stringview DEVICE_PREFIX{"/dev/video"};
        if ((source.size() <= DEVICE_PREFIX.size()) || (source.substr(0U, DEVICE_PREFIX.size()) != DEVICE_PREFIX))
        {
            return false;
        }
        for (const char value : source.substr(DEVICE_PREFIX.size()))
        {
            if ((value < '0') || (value > '9'))
            {
                return false;
            }
        }
        return true;
    }

    /** @brief Constructs a camera around one caller-owned backend. */
    XWalkCamera::XWalkCamera(contextpointer context,
                             cameracapturecallback captureOperation,
                             const XWalkCameraConfiguration& configuration)
        : backendContext(context), captureCallback(captureOperation), configurationValue(configuration)
    {
        validate(captureCallback, configurationValue);
        XWALK_HAL_TRACE_UID3(RPI .212,
                             "Camera constructed for %u by %u pixels with timeout %u ms",
                             configurationValue.widthPixels,
                             configurationValue.heightPixels,
                             configurationValue.timeoutMs);
    }

    /** @brief Releases no caller-owned backend resources. */
    XWalkCamera::~XWalkCamera() = default;

    /** @brief Captures one JPEG image at a caller-selected destination. */
    string XWalkCamera::capture(stringview outputPath)
    {
        const hal::boolean imageCaptured = tryCapture(outputPath);
        if (imageCaptured == false)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Camera backend failed to capture an image");
        }
        return string(outputPath);
    }

    /**
     * @brief Attempts one JPEG capture for a caller that permits no image.
     * @param[in] outputPath Non-empty single-line destination path.
     * @return `true` after successful capture; otherwise `false` when the backend
     * cannot produce an image.
     * @throws std::invalid_argument If the output path is empty or contains a line
     * terminator.
     */
    boolean XWalkCamera::tryCapture(stringview outputPath)
    {
        const hal::boolean outputPathNRInvalid =
            static_cast<hal::boolean>(outputPath.empty() || (outputPath.find('\n') != stringview::npos) ||
                                      (outputPath.find('\r') != stringview::npos));
        if (outputPathNRInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Camera output path must be non-empty and single-line");
        }
        const hal::boolean imageCaptured = captureCallback(backendContext, outputPath, configurationValue);
        if (imageCaptured == false)
        {
            return false;
        }
        XWALK_HAL_TRACE_UID2(RPI .213,
                             "Camera capture completed at %u by %u pixels",
                             configurationValue.widthPixels,
                             configurationValue.heightPixels);
        return true;
    }

    /** @brief Converts a deployment connection name to its typed value. */
    XWalkCameraConnection XWalkCamera::connectionFromString(stringview connection)
    {
        if (connection == "csi")
        {
            XWALK_HAL_TRACE_UID0(RPI .214, "Camera CSI connection selected");
            return XWalkCameraConnection::Csi;
        }
        if (connection == "usb")
        {
            XWALK_HAL_TRACE_UID0(RPI .215, "Camera USB connection selected");
            return XWalkCameraConnection::Usb;
        }
        XWALK_HAL_ERROR(XWALK_INVAL, "Camera connection must be csi or usb");
    }

    /** @brief Validates the callback and bounded capture settings. */
    void XWalkCamera::validate(cameracapturecallback capture, const XWalkCameraConfiguration& configuration)
    {
        if (capture == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Camera capture callback must not be null");
        }
        if ((configuration.widthPixels < 16U) || (configuration.widthPixels > 7'680U) ||
            (configuration.heightPixels < 16U) || (configuration.heightPixels > 4'320U) ||
            (configuration.timeoutMs == 0U) || (configuration.timeoutMs > 300'000U))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Camera capture settings are outside supported bounds");
        }
    }

} /* namespace xwalk::hal */
