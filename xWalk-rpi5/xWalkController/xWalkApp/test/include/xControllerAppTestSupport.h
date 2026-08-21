/******************************************************************************
 * @file        xControllerAppTestSupport.h
 * @brief       Declares reusable device-free Controller application test support.
 *
 * @details
 * Provides deterministic camera state and callbacks for camera-only lifecycle
 * tests without accessing Raspberry Pi hardware.
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

#ifndef XCONTROLLER_APP_TEST_SUPPORT_H
#define XCONTROLLER_APP_TEST_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCameraStream.h"
#include "xController.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl::test::app
 * @brief Contains reusable device-free Controller application test support.
 */
namespace xwalk::ctrl::test::app
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct CameraOnlyStreamingTestState
     * @brief Records the lifecycle of one deterministic camera-only stream.
     */
    struct CameraOnlyStreamingTestState
    {
            /** @brief True between successful start and stop callbacks. */
            ::ctrl::boolean cameraStarted{};
            /** @brief Number of stop callbacks received by the fake camera. */
            ::ctrl::uint32 stopCount{};
    };

    /**
     * @class CameraOnlyControllerProbe
     * @brief Exposes the protected cancellation check for a camera-only Controller.
     *
     * @details
     * Inherits production construction and retains no additional state. Test code
     * owns every dependency and never deletes this probe through a base pointer.
     */
    class CameraOnlyControllerProbe final : public XWalkController
    {
        public:
            /**************************************************************************
             * Public constructors
             **************************************************************************/

            using XWalkController::XWalkController;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Evaluates production cancellation without an attached vehicle.
             * @return Production continuation result for the configured callback.
             */
            ::ctrl::boolean operationMayContinueForTest();
    };

    /******************************************************************************
     * Function declarations
     ******************************************************************************/

    /**
     * @brief Marks the deterministic camera in `context` as started.
     * @param[in,out] context Non-null pointer to test-owned camera state.
     * @param[in] configuration Validated camera configuration; unused by the fake.
     * @return Always `true` after recording successful startup.
     */
    ::ctrl::boolean startCameraOnlyStream(::ctrl::contextpointer context,
                                          const hal::XWalkCameraStreamConfiguration& configuration);

    /**
     * @brief Marks the deterministic camera in `context` as stopped.
     * @param[in,out] context Non-null pointer to test-owned camera state.
     */
    void stopCameraOnlyStream(::ctrl::contextpointer context) noexcept;

    /**
     * @brief Produces one minimal deterministic JPEG frame.
     * @param[in,out] context Non-null pointer to test-owned camera state.
     * @param[in] configuration Validated camera configuration; unused by the fake.
     * @param[out] jpeg Complete minimal JPEG marker sequence.
     * @return Always `true` after assigning the frame.
     */
    ::ctrl::boolean captureCameraOnlyFrame(::ctrl::contextpointer context,
                                           const hal::XWalkCameraStreamConfiguration& configuration,
                                           ::ctrl::bytevector& jpeg);

    /**
     * @brief Returns a deterministic monotonic millisecond value.
     * @param[in] context Optional test context; unused.
     * @return One millisecond.
     */
    ::ctrl::uint64 cameraOnlyStreamingClock(::ctrl::contextpointer context) noexcept;

    /**
     * @brief Creates the complete deterministic camera callback table.
     * @return Callback table requiring a `CameraOnlyStreamingTestState` context.
     */
    hal::XWalkCameraStreamCallbacks cameraOnlyStreamingCallbacks() noexcept;

} /* namespace xwalk::ctrl::test::app */

#endif /* XCONTROLLER_APP_TEST_SUPPORT_H */
