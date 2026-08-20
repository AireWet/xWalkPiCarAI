/******************************************************************************
 * @file        xAgent_Rpi5CarCameraCaptureTestSupport.cpp
 * @brief       Implements reusable camera-capture Agent host-test support.
 *
 * @details
 * Records fake capture requests entirely in caller-owned state.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarCameraCaptureTestSupport.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent::test::camera
 * @brief Contains reusable camera-capture Agent host-test support.
 */
namespace xwalk::agent::test::camera
{

    /**
     * @brief Records one fake camera request and returns configured state.
     * @param[in,out] context Non-null non-owning `CameraCaptureTestState` pointer.
     * @param[in] outputPath Destination copied into the fake state.
     * @param[in] configuration Validated settings observed without modification.
     * @return Configured fake capture result.
     */
    agent::boolean capture(agent::contextpointer context,
                           agent::stringview outputPath,
                           const hal::XWalkCameraConfiguration& configuration)
    {
        CameraCaptureTestState& state = *static_cast<CameraCaptureTestState*>(context);
        static_cast<void>(configuration);
        ++state.captureCount;
        state.outputPath = outputPath;
        return state.result;
    }

} /* namespace xwalk::agent::test::camera */
