/******************************************************************************
 * @file        xHal_Rpi5CarCameraTypes.h
 * @brief       Declares xWalk camera configuration and callback types.
 *
 * @details
 * Defines backend-neutral capture settings for Raspberry Pi CSI cameras and
 * USB Video Class cameras.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_CAMERA_TYPES_H
#define XHAL_RPI5CAR_CAMERA_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal
{

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/**
 * @enum XWalkCameraConnection
 * @brief Selects the physical camera connection handled by a backend.
 */
enum class XWalkCameraConnection : uint8
{
    /** @brief Raspberry Pi Camera Serial Interface camera. */
    Csi = 0U,
    /** @brief USB Video Class camera exposed through Linux V4L2. */
    Usb
};

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @struct XWalkCameraConfiguration
 * @brief Stores bounded still-image capture settings.
 */
struct XWalkCameraConfiguration
{
    /** @brief Output width in pixels from 16 through 7680. */
    uint32 widthPixels{640U};
    /** @brief Output height in pixels from 16 through 4320. */
    uint32 heightPixels{480U};
    /** @brief Capture timeout in milliseconds from 1 through 300,000. */
    uint32 timeoutMs{5'000U};
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Captures one JPEG image through an injected backend.
 * @param[in,out] context Nullable backend context that outlives callback use.
 * @param[in] outputPath Non-empty destination file path.
 * @param[in] configuration Validated still-image settings.
 * @return `true` when a complete image was written; otherwise `false`.
 */
using cameracapturecallback = boolean (*)(contextpointer context,
    stringview outputPath, const XWalkCameraConfiguration& configuration);

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_CAMERA_TYPES_H */
