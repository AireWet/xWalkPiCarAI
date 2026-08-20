/******************************************************************************
 * @file        xAgent_Rpi5CarCameraCaptureTestSupport.h
 * @brief       Declares reusable camera-capture Agent host-test support.
 *
 * @details
 * Provides caller-owned fake capture state and one backend callback without
 * accessing a camera device or filesystem.
 *
 * @project     xWalk Firmware
 * @module      xWalkCameraCapture Host Test
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

#ifndef XAGENT_RPI5CAR_CAMERA_CAPTURE_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_CAMERA_CAPTURE_TEST_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCamera.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent::test::camera
 * @brief Contains reusable camera-capture Agent host-test support.
 */
namespace xwalk::agent::test::camera
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct CameraCaptureTestState
     * @brief Records deterministic camera callback behavior and observations.
     */
    struct CameraCaptureTestState
    {
            /** @brief Result returned by the fake camera backend. */
            agent::boolean result{true};
            /** @brief Number of fake camera callback invocations. */
            agent::uint32 captureCount{};
            /** @brief Most recently requested output path. */
            agent::string outputPath{};
    };

    /******************************************************************************
     * Function declarations
     ******************************************************************************/

    /**
     * @brief Records one fake camera request and returns configured state.
     * @param[in,out] context Non-null non-owning `CameraCaptureTestState` pointer.
     * @param[in] outputPath Destination copied into the fake state.
     * @param[in] configuration Validated settings observed without modification.
     * @return Configured fake capture result.
     */
    agent::boolean capture(agent::contextpointer context,
                           agent::stringview outputPath,
                           const hal::XWalkCameraConfiguration& configuration);

} /* namespace xwalk::agent::test::camera */

#endif /* XAGENT_RPI5CAR_CAMERA_CAPTURE_TEST_SUPPORT_H */
