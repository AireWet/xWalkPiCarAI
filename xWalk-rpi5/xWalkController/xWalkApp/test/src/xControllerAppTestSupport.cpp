/******************************************************************************
 * @file        xControllerAppTestSupport.cpp
 * @brief       Implements reusable device-free Controller application test support.
 *
 * @details
 * Supplies deterministic camera lifecycle callbacks and loopback port
 * selection for Controller application tests.
 *
 * @project     xWalk Firmware
 * @module      xWalkApp GoogleTest Support
 *
 * @author      Joxy John
 * @date        2026-08-21
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

#include "xControllerAppTestSupport.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl::test::app
 * @brief Contains reusable device-free Controller application test support.
 */
namespace xwalk::ctrl::test::app
{

    /**
     * @brief Evaluates production cancellation without an attached vehicle.
     * @return Production continuation result for the configured callback.
     */
    ::ctrl::boolean CameraOnlyControllerProbe::operationMayContinueForTest()
    {
        return operationMayContinue();
    }

    /**
     * @brief Marks the deterministic camera in `context` as started.
     * @param[in,out] context Non-null pointer to test-owned camera state.
     * @param[in] configuration Validated camera configuration; unused by the fake.
     * @return Always `true` after recording successful startup.
     */
    ::ctrl::boolean startCameraOnlyStream(::ctrl::contextpointer context,
                                          const hal::XWalkCameraStreamConfiguration& configuration)
    {
        static_cast<void>(configuration);
        CameraOnlyStreamingTestState& state = *static_cast<CameraOnlyStreamingTestState*>(context);
        state.cameraStarted = true;
        return true;
    }

    /**
     * @brief Marks the deterministic camera in `context` as stopped.
     * @param[in,out] context Non-null pointer to test-owned camera state.
     */
    void stopCameraOnlyStream(::ctrl::contextpointer context) noexcept
    {
        CameraOnlyStreamingTestState& state = *static_cast<CameraOnlyStreamingTestState*>(context);
        state.cameraStarted = false;
        ++state.stopCount;
    }

    /**
     * @brief Produces one minimal deterministic JPEG frame.
     * @param[in,out] context Non-null pointer to test-owned camera state.
     * @param[in] configuration Validated camera configuration; unused by the fake.
     * @param[out] jpeg Complete minimal JPEG marker sequence.
     * @return Always `true` after assigning the frame.
     */
    ::ctrl::boolean captureCameraOnlyFrame(::ctrl::contextpointer context,
                                           const hal::XWalkCameraStreamConfiguration& configuration,
                                           ::ctrl::bytevector& jpeg)
    {
        static_cast<void>(context);
        static_cast<void>(configuration);
        jpeg = {0xFFU, 0xD8U, 0xFFU, 0xD9U};
        return true;
    }

    /**
     * @brief Returns a deterministic monotonic millisecond value.
     * @param[in] context Optional test context; unused.
     * @return One millisecond.
     */
    ::ctrl::uint64 cameraOnlyStreamingClock(::ctrl::contextpointer context) noexcept
    {
        static_cast<void>(context);
        return 1U;
    }

    /**
     * @brief Creates the complete deterministic camera callback table.
     * @return Callback table requiring a `CameraOnlyStreamingTestState` context.
     */
    hal::XWalkCameraStreamCallbacks cameraOnlyStreamingCallbacks() noexcept
    {
        return {&startCameraOnlyStream, &stopCameraOnlyStream, &captureCameraOnlyFrame};
    }

} /* namespace xwalk::ctrl::test::app */
