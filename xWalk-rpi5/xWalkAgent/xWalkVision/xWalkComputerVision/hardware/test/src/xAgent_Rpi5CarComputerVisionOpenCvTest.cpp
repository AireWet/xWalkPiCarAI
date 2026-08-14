#include "xAgent_Rpi5CarComputerVisionOpenCv.h"
#include "xAgent_Rpi5CarComputerVisionOpenCvTestSupport.h"

#include <gtest/gtest.h>

namespace xwalk::agent::test::computer_vision_opencv
{
TEST(XWalkComputerVisionOpenCvHost, SelectsEverySupportedBackend)
{
    EXPECT_EQ(XWalkComputerVisionOpenCv::backendFromString("v4l2"),
        XWalkComputerVisionOpenCvBackend::V4l2);
    EXPECT_EQ(XWalkComputerVisionOpenCv::backendFromString("gstreamer"),
        XWalkComputerVisionOpenCvBackend::Gstreamer);
    EXPECT_EQ(XWalkComputerVisionOpenCv::backendFromString("video_file"),
        XWalkComputerVisionOpenCvBackend::VideoFile);
    EXPECT_EQ(XWalkComputerVisionOpenCv::backendFromString("image_sequence"),
        XWalkComputerVisionOpenCvBackend::ImageSequence);
    EXPECT_EQ(XWalkComputerVisionOpenCv::backendFromString("automatic"),
        XWalkComputerVisionOpenCvBackend::Automatic);
    EXPECT_THROW(static_cast<void>(XWalkComputerVisionOpenCv::backendFromString("unknown")),
        std::invalid_argument);
}

TEST(XWalkComputerVisionOpenCvHost, RejectsEveryConfigurationBoundary)
{
    XWalkComputerVisionOpenCvConfiguration configuration;
    configuration.cameraDevice = "/tmp/input.avi";
    configuration.photoDirectory = "/tmp";
    configuration.faceCascadePath.clear();
    const auto rejected = [&configuration]()
    {
        EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration),
            std::invalid_argument);
    };
    configuration.photoDirectory.clear();
    rejected();
    configuration.photoDirectory = "relative";
    rejected();
    configuration.photoDirectory = "/tmp";
    configuration.cameraDevice = "/tmp/input\r.avi";
    rejected();
    configuration.cameraDevice = "/tmp/input.avi";
    configuration.faceCascadePath = "relative.xml";
    rejected();
    configuration.faceCascadePath.clear();
    configuration.widthPixels = 15U;
    EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration), std::out_of_range);
    configuration.widthPixels = 7'681U;
    EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration), std::out_of_range);
    configuration.widthPixels = 640U;
    configuration.heightPixels = 15U;
    EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration), std::out_of_range);
    configuration.heightPixels = 4'321U;
    EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration), std::out_of_range);
    configuration.heightPixels = 480U;
    configuration.readTimeoutMilliseconds = 60'001U;
    EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration), std::out_of_range);
    configuration.readTimeoutMilliseconds = 1'000U;
    configuration.faceCascadePath = "/tmp/xwalk-missing-cascade.xml";
    EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration), std::runtime_error);
}

TEST(XWalkComputerVisionOpenCvHost, ExercisesCallbacksAndColorDetectors)
{
    RecordedVideoFixture fixture(true);
    XWalkComputerVisionOpenCvConfiguration configuration;
    configuration.cameraBackend = XWalkComputerVisionOpenCvBackend::VideoFile;
    configuration.cameraDevice = fixture.video.string();
    configuration.photoDirectory = fixture.directory.string();
    configuration.faceCascadePath.clear();
    configuration.widthPixels = 32U;
    configuration.heightPixels = 24U;
    XWalkComputerVisionOpenCv provider(configuration);
    const XWalkComputerVisionCallbacks callbackTable = provider.callbacks();
    EXPECT_THROW(static_cast<void>(callbackTable.start(nullptr)), std::invalid_argument);
    EXPECT_NO_THROW(callbackTable.stop(nullptr));
    ASSERT_TRUE(callbackTable.start(&provider));
    EXPECT_TRUE(callbackTable.start(&provider));
    const XWalkComputerVisionColor colors[]{XWalkComputerVisionColor::Red,
        XWalkComputerVisionColor::Orange, XWalkComputerVisionColor::Yellow,
        XWalkComputerVisionColor::Green, XWalkComputerVisionColor::Blue,
        XWalkComputerVisionColor::Purple};
    for (const XWalkComputerVisionColor color : colors)
    {
        callbackTable.setColor(&provider, color);
        EXPECT_GE(callbackTable.observe(&provider).color.count, 1U);
    }
    callbackTable.setColor(&provider, XWalkComputerVisionColor::Close);
    callbackTable.setQr(&provider, true);
    callbackTable.setQr(&provider, false);
    callbackTable.stop(&provider);
}

TEST(XWalkComputerVisionOpenCvHost, CapturesRecordedFrameToIsolatedDirectory)
{
    RecordedVideoFixture fixture;
    XWalkComputerVisionOpenCvConfiguration configuration;
    configuration.cameraBackend = XWalkComputerVisionOpenCvBackend::VideoFile;
    configuration.cameraDevice = fixture.video.string();
    configuration.photoDirectory = (fixture.directory / "photos").string();
    configuration.faceCascadePath.clear();
    configuration.widthPixels = 32U;
    configuration.heightPixels = 24U;
    XWalkComputerVisionOpenCv provider(configuration);
    const XWalkComputerVisionCallbacks callbackTable = provider.callbacks();
    ASSERT_TRUE(callbackTable.start(&provider));
    const agent::filesystempath photo(callbackTable.capture(&provider));
    EXPECT_TRUE(std::filesystem::is_regular_file(photo));
    callbackTable.stop(&provider);
}

TEST(XWalkComputerVisionOpenCvHost, ValidatesSourcesBeforeOpening)
{
    XWalkComputerVisionOpenCvConfiguration configuration;
    configuration.photoDirectory = "/tmp";
    configuration.faceCascadePath.clear();
    EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration), std::invalid_argument);
    configuration.cameraDevice = "relative-device";
    EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration), std::invalid_argument);
    configuration.cameraBackend = XWalkComputerVisionOpenCvBackend::Gstreamer;
    configuration.cameraDevice = "pipeline\ninvalid";
    EXPECT_THROW(XWalkComputerVisionOpenCv provider(configuration), std::invalid_argument);
}

TEST(XWalkComputerVisionOpenCvHost, ReadsRecordedFrameAndReportsEndOfFile)
{
    RecordedVideoFixture fixture;
    XWalkComputerVisionOpenCvConfiguration configuration;
    configuration.cameraBackend = XWalkComputerVisionOpenCvBackend::VideoFile;
    configuration.cameraDevice = fixture.video.string();
    configuration.photoDirectory = fixture.directory.string();
    configuration.faceCascadePath.clear();
    configuration.widthPixels = 32U;
    configuration.heightPixels = 24U;
    XWalkComputerVisionOpenCv provider(configuration);
    const XWalkComputerVisionCallbacks callbacks = provider.callbacks();
    ASSERT_TRUE(callbacks.start(&provider));
    EXPECT_NO_THROW(static_cast<void>(callbacks.observe(&provider)));
    EXPECT_THROW(static_cast<void>(callbacks.observe(&provider)), std::out_of_range);
    EXPECT_THROW(callbacks.setFace(&provider, true), std::logic_error);
    callbacks.stop(&provider);
    callbacks.stop(&provider);
}

TEST(XWalkComputerVisionOpenCvHost, ReportsOpenFailureWithoutHardware)
{
    XWalkComputerVisionOpenCvConfiguration configuration;
    configuration.cameraBackend = XWalkComputerVisionOpenCvBackend::VideoFile;
    configuration.cameraDevice = "/tmp/xwalk-file-that-does-not-exist.avi";
    configuration.photoDirectory = "/tmp";
    configuration.faceCascadePath.clear();
    configuration.readTimeoutMilliseconds = 0U;
    EXPECT_THROW(XWalkComputerVisionOpenCv invalidTimeout(configuration), std::out_of_range);
    configuration.readTimeoutMilliseconds = 1'000U;
    XWalkComputerVisionOpenCv provider(configuration);
    EXPECT_FALSE(provider.callbacks().start(&provider));
}
} /* namespace xwalk::agent::test::computer_vision_opencv */
