/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingOpenCvHostTest.cpp
 * @brief       Verifies OpenCV video sources without physical camera access.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording OpenCV Host Test
 * @author      Joxy John
 * @date        2026-08-11
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoRecordingOpenCv.h"
#include "xAgent_Rpi5CarVideoRecordingOpenCvTestSupport.h"

#include <gtest/gtest.h>

#include <limits>

namespace xwalk::agent::test::video_recording_opencv
{

/** @brief Verifies every supported configuration selector. */
TEST(XWalkVideoRecordingOpenCvHost, SelectsEverySupportedBackend)
{
    EXPECT_EQ(XWalkVideoRecordingOpenCv::backendFromString("v4l2"),
        XWalkVideoRecordingOpenCvBackend::V4l2);
    EXPECT_EQ(XWalkVideoRecordingOpenCv::backendFromString("gstreamer"),
        XWalkVideoRecordingOpenCvBackend::Gstreamer);
    EXPECT_EQ(XWalkVideoRecordingOpenCv::backendFromString("video_file"),
        XWalkVideoRecordingOpenCvBackend::VideoFile);
    EXPECT_EQ(XWalkVideoRecordingOpenCv::backendFromString("image_sequence"),
        XWalkVideoRecordingOpenCvBackend::ImageSequence);
    EXPECT_EQ(XWalkVideoRecordingOpenCv::backendFromString("automatic"),
        XWalkVideoRecordingOpenCvBackend::Automatic);
    EXPECT_THROW(static_cast<void>(XWalkVideoRecordingOpenCv::backendFromString("unknown")),
        std::invalid_argument);
}

/** @brief Verifies path and timeout rejection before any source is opened. */
TEST(XWalkVideoRecordingOpenCvHost, ValidatesConfigurationBeforeOpening)
{
    XWalkVideoRecordingOpenCvConfiguration configuration;
    configuration.videoDirectory = "/tmp";
    EXPECT_THROW(XWalkVideoRecordingOpenCv missingSource(configuration), std::invalid_argument);
    configuration.cameraDevice = "relative.avi";
    configuration.cameraBackend = XWalkVideoRecordingOpenCvBackend::VideoFile;
    EXPECT_THROW(XWalkVideoRecordingOpenCv relativeSource(configuration), std::invalid_argument);
    configuration.cameraDevice = "/tmp/missing.avi";
    configuration.readTimeoutMilliseconds = 0U;
    EXPECT_THROW(XWalkVideoRecordingOpenCv invalidTimeout(configuration), std::out_of_range);
}

/** @brief Rejects every bounded recording configuration family. */
TEST(XWalkVideoRecordingOpenCvHost, RejectsEveryConfigurationBoundary)
{
    XWalkVideoRecordingOpenCvConfiguration configuration;
    configuration.cameraBackend = XWalkVideoRecordingOpenCvBackend::VideoFile;
    configuration.cameraDevice = "/tmp/input.avi";
    configuration.videoDirectory = "/tmp";
    configuration.cameraDevice = "/tmp/input\n.avi";
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::invalid_argument);
    configuration.cameraDevice = "/tmp/input.avi";
    configuration.videoDirectory = "relative";
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::invalid_argument);
    configuration.videoDirectory = "/tmp";
    configuration.widthPixels = 15U;
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::out_of_range);
    configuration.widthPixels = 7'681U;
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::out_of_range);
    configuration.widthPixels = 640U;
    configuration.heightPixels = 15U;
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::out_of_range);
    configuration.heightPixels = 4'321U;
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::out_of_range);
    configuration.heightPixels = 480U;
    configuration.framesPerSecond = 0.0;
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::out_of_range);
    configuration.framesPerSecond = 121.0;
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::out_of_range);
    configuration.framesPerSecond = std::numeric_limits<agent::float64>::infinity();
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::out_of_range);
    configuration.framesPerSecond = 20.0;
    configuration.readTimeoutMilliseconds = 60'001U;
    EXPECT_THROW(XWalkVideoRecordingOpenCv provider(configuration), std::out_of_range);
}

/** @brief Exercises callback context and inactive-recorder failure paths. */
TEST(XWalkVideoRecordingOpenCvHost, RejectsInvalidCallbackState)
{
    XWalkVideoRecordingOpenCvConfiguration configuration;
    configuration.cameraBackend = XWalkVideoRecordingOpenCvBackend::VideoFile;
    configuration.cameraDevice = "/tmp/xwalk-recording-source-does-not-exist.avi";
    configuration.videoDirectory = "/tmp";
    XWalkVideoRecordingOpenCv provider(configuration);
    const XWalkVideoRecordingCallbacks callbacks = provider.callbacks();
    EXPECT_THROW(static_cast<void>(callbacks.startCamera(nullptr)), std::invalid_argument);
    EXPECT_NO_THROW(callbacks.stopCamera(nullptr));
    EXPECT_THROW(static_cast<void>(callbacks.beginRecording(&provider, {})),
        std::runtime_error);
    EXPECT_THROW(callbacks.pauseRecording(&provider), std::logic_error);
    EXPECT_THROW(callbacks.continueRecording(&provider), std::logic_error);
    EXPECT_FALSE(callbacks.timestamp(&provider).empty());
}

/** @brief Verifies finite recorded input and repeated shutdown without a device node. */
TEST(XWalkVideoRecordingOpenCvHost, OpensRecordedInputAndStopsIdempotently)
{
    RecordedVideoFixture fixture;
    XWalkVideoRecordingOpenCvConfiguration configuration;
    configuration.cameraBackend = XWalkVideoRecordingOpenCvBackend::VideoFile;
    configuration.cameraDevice = fixture.video.string();
    configuration.videoDirectory = fixture.directory.string();
    configuration.widthPixels = 32U;
    configuration.heightPixels = 24U;
    XWalkVideoRecordingOpenCv provider(configuration);
    const XWalkVideoRecordingCallbacks callbacks = provider.callbacks();
    ASSERT_TRUE(callbacks.startCamera(&provider));
    callbacks.stopCamera(&provider);
    callbacks.stopCamera(&provider);
}

/** @brief Verifies a missing recorded source reports open failure safely. */
TEST(XWalkVideoRecordingOpenCvHost, ReportsMissingSourceWithoutHardware)
{
    XWalkVideoRecordingOpenCvConfiguration configuration;
    configuration.cameraBackend = XWalkVideoRecordingOpenCvBackend::VideoFile;
    configuration.cameraDevice = "/tmp/xwalk-recording-source-does-not-exist.avi";
    configuration.videoDirectory = "/tmp";
    XWalkVideoRecordingOpenCv provider(configuration);
    EXPECT_FALSE(provider.callbacks().startCamera(&provider));
}

} /* namespace xwalk::agent::test::video_recording_opencv */
