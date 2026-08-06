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

#include "xHal_Rpi5CarExceptions.h"

namespace xwalk::hal
{

/** @brief Constructs a camera around one caller-owned backend. */
XWalkCamera::XWalkCamera(contextpointer context, cameracapturecallback captureOperation,
    const XWalkCameraConfiguration& configuration):
    backendContext(context), captureCallback(captureOperation), configurationValue(configuration)
{
    validate(captureCallback, configurationValue);
}

/** @brief Releases no caller-owned backend resources. */
XWalkCamera::~XWalkCamera() = default;

/** @brief Captures one JPEG image at a caller-selected destination. */
string XWalkCamera::capture(stringview outputPath)
{
    const hal::boolean outputPathNRInvalid =
        static_cast<hal::boolean>(
            outputPath.empty() || (outputPath.find('\n') != stringview::npos) ||
        (outputPath.find('\r') != stringview::npos));
    if (outputPathNRInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Camera output path must be non-empty and single-line");
    }
    const hal::boolean imageCaptured =
        captureCallback(backendContext, outputPath, configurationValue);
    if (imageCaptured == false)
    {
        XHAL_THROW_RUNTIME_ERROR("Camera backend failed to capture an image");
    }
    return string(outputPath);
}

/** @brief Converts a deployment connection name to its typed value. */
XWalkCameraConnection XWalkCamera::connectionFromString(stringview connection)
{
    if (connection == "csi")
    {
        return XWalkCameraConnection::Csi;
    }
    if (connection == "usb")
    {
        return XWalkCameraConnection::Usb;
    }
    XHAL_THROW_INVALID_ARGUMENT("Camera connection must be csi or usb");
}

/** @brief Validates the callback and bounded capture settings. */
void XWalkCamera::validate(cameracapturecallback capture,
    const XWalkCameraConfiguration& configuration)
{
    if (capture == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Camera capture callback must not be null");
    }
    if ((configuration.widthPixels < 16U) || (configuration.widthPixels > 7'680U) ||
        (configuration.heightPixels < 16U) || (configuration.heightPixels > 4'320U) ||
        (configuration.timeoutMs == 0U) || (configuration.timeoutMs > 300'000U))
    {
        XHAL_THROW_OUT_OF_RANGE("Camera capture settings are outside supported bounds");
    }
}

} /* namespace xwalk::hal */
