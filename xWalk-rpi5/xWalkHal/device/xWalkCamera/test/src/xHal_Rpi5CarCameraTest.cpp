/******************************************************************************
 * @file        xHal_Rpi5CarCameraTest.cpp
 * @brief       Verifies backend-neutral camera behavior in memory.
 *
 * @details
 * Checks capture forwarding, validation, connection parsing, failure
 * propagation, and persistent trace selection without camera access.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera Host Test
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
#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarCameraSimulationArguments.h"
#include "xHal_Rpi5CarCameraSimulationConfig.h"
#include "xHal_Rpi5CarCameraTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"

/** @brief Contains camera host-test scenarios private to this translation unit.
 */
namespace
{
    using xwalk::hal::test::camera::CameraStreamTestState;
    using xwalk::hal::test::camera::CameraTestState;
    using xwalk::hal::test::camera::captureImage;
    using xwalk::hal::test::camera::streamCallbacks;

    /** @brief Verifies successful capture forwarding and connection parsing. */
    void testCapture()
    {
        CameraTestState state;
        xwalk::hal::XWalkCamera camera(&state, &captureImage);
        const XWalkHal::string result = camera.capture("image.jpg");
        assert(result == "image.jpg");
        const XWalkHal::boolean optionalCapture = camera.tryCapture("optional-image.jpg");
        xwalk::hal::test::requireTestCondition(optionalCapture);
        assert(state.captureCount == 2U);
        assert(state.outputPath == "optional-image.jpg");
        assert(state.configuration.widthPixels == 640U);
        assert(state.configuration.heightPixels == 480U);
        assert(state.configuration.timeoutMs == 5'000U);
        assert(xwalk::hal::XWalkCamera::connectionFromString("csi") == xwalk::hal::XWalkCameraConnection::Csi);
        assert(xwalk::hal::XWalkCamera::connectionFromString("usb") == xwalk::hal::XWalkCameraConnection::Usb);
        assert(xwalk::hal::validCameraSourceString("csi"));
        assert(xwalk::hal::validCameraSourceString("usb"));
        assert(xwalk::hal::validCameraSourceString("/dev/video12"));
        assert(!xwalk::hal::validCameraSourceString("https://untrusted.invalid/live"));
        assert(!xwalk::hal::validCameraSourceString("/dev/video0\n"));
    }

    /** @brief Verifies callback, configuration, path, and backend-failure
     * validation. */
    void testValidation()
    {
        CameraTestState state;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                const xwalk::hal::XWalkCamera camera(&state, nullptr);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::XWalkCameraConfiguration configuration;
                configuration.widthPixels = 15U;
                const xwalk::hal::XWalkCamera camera(&state, &captureImage, configuration);
            });
        xwalk::hal::XWalkCamera camera(&state, &captureImage);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(camera.capture("invalid\npath.jpg"));
            });
        state.result = false;
        const XWalkHal::boolean optionalCapture = camera.tryCapture("optional-failed.jpg");
        xwalk::hal::test::requireTestCondition(optionalCapture == false);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(camera.capture("failed.jpg"));
            });
        xwalk::hal::test::expectFailure(
            []()
            {
                static_cast<void>(xwalk::hal::XWalkCamera::connectionFromString("dsi"));
            });
    }

    /** @brief Verifies encoded-camera lifecycle, capture, and validation. */
    void testStream()
    {
        CameraStreamTestState state;
        xwalk::hal::XWalkCameraStream stream(&state, streamCallbacks());
        XWalkHal::bytevector jpeg;
        const XWalkHal::boolean captureBeforeStart = stream.capture(jpeg);
        xwalk::hal::test::requireTestCondition(captureBeforeStart == false);
        xwalk::hal::test::requireTestCondition(jpeg.empty());
        const XWalkHal::boolean streamStarted = stream.start();
        xwalk::hal::test::requireTestCondition(streamStarted);
        xwalk::hal::test::requireTestCondition(stream.started());
        const XWalkHal::boolean frameCaptured = stream.capture(jpeg);
        xwalk::hal::test::requireTestCondition(frameCaptured);
        xwalk::hal::test::requireTestCondition(state.captureCount == 1U);
        xwalk::hal::test::requireTestCondition(jpeg.size() == 5U);
        stream.stop();
        xwalk::hal::test::requireTestCondition(stream.started() == false);
        xwalk::hal::test::requireTestCondition(state.started == false);

        xwalk::hal::test::expectFailure(
            [&state]()
            {
                const xwalk::hal::XWalkCameraStream streamValue(&state, {});
            });
        xwalk::hal::test::expectFailure(
            [&state]()
            {
                xwalk::hal::XWalkCameraStreamConfiguration configuration;
                configuration.jpegQuality = 0U;
                const xwalk::hal::XWalkCameraStream streamValue(&state, streamCallbacks(), configuration);
            });
    }

    /** @brief Verifies persistent Camera trace-selector parsing and application. */
    void testTraceSelection()
    {
        char executable[] = "xWalkCameraTest";
        char option[] = "--trace";
        char enableSelector[] = "RPI.219.enable";
        char disableSelector[] = "RPI.219.disable";
        char malformedSelector[] = "RPI.invalid.enable";
        XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
        xwalk::hal::sim::XWalkCameraSimulationArguments enable(3, enableArguments);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());
        XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
        xwalk::hal::sim::XWalkCameraSimulationArguments disable(3, disableArguments);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());
        XWalkHal::charpointer malformedArguments[]{executable, option, malformedSelector};
        const xwalk::hal::sim::XWalkCameraSimulationArguments malformed(3, malformedArguments);
        assert(malformed.valid() == false);
    }
} /* namespace */

/**
 * @brief Runs every xWalkCamera host-test scenario.
 * @return Zero when all assertions pass.
 */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_CAMERA_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_CAMERA_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .222, "xWalkCamera host tests started");
    testCapture();
    testValidation();
    testStream();
    testTraceSelection();
    XWALK_HAL_TRACE_UID0(RPI .223, "xWalkCamera host tests completed");
    return 0;
}
