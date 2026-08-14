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

/** @brief Contains reusable xWalkCamera host-test support. */
namespace xwalk::hal::test::camera
{
/** @brief Records one in-memory camera capture request. */
struct CameraTestState
{
    uint32 captureCount{}; /**< Number of capture callback invocations. */
    string outputPath{}; /**< Most recently requested destination path. */
    XWalkCameraConfiguration configuration{}; /**< Most recently requested settings. */
    boolean result{true}; /**< Result returned to the camera interface. */
};

/**
 * @brief Records one camera request without accessing a physical device.
 * @param[in,out] context Non-null, non-owning pointer to `CameraTestState`.
 * @param[in] outputPath Non-empty destination supplied by the camera.
 * @param[in] configuration Validated capture settings copied into the state.
 * @return The configured in-memory result.
 */
boolean captureImage(contextpointer context, stringview outputPath,
    const XWalkCameraConfiguration& configuration);
} /* namespace xwalk::hal::test::camera */

#endif /* XHAL_RPI5CAR_CAMERA_TEST_SUPPORT_H */
