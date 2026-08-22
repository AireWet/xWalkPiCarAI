#include "xAgent_Rpi5CarComputerVisionOpenCvTestSupport.h"
#include "xHal_Rpi5CarTrace.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <chrono>
#include <filesystem>

namespace xwalk::agent::test::computer_vision_opencv
{
    RecordedVideoFixture::RecordedVideoFixture(agent::boolean colorSequence)
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        directory = agent::filesystempath("/tmp") / (agent::string("xwalk-opencv-") + std::to_string(stamp));
        std::filesystem::create_directories(directory);
        video = directory / "one-frame.avi";
        cv::VideoWriter writer(video.string(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10.0, cv::Size(32, 24));
        const agent::boolean writerOpen = writer.isOpened();
        if (!writerOpen)
        {
            XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, "Recorded-video test fixture could not open its writer");
        }
        if (colorSequence)
        {
            const cv::Scalar colors[]{{0.0, 0.0, 255.0},
                                      {0.0, 128.0, 255.0},
                                      {0.0, 255.0, 255.0},
                                      {0.0, 255.0, 0.0},
                                      {255.0, 0.0, 0.0},
                                      {255.0, 0.0, 255.0}};
            for (const cv::Scalar& color : colors)
            {
                writer.write(cv::Mat(24, 32, CV_8UC3, color));
            }
        }
        else
        {
            writer.write(cv::Mat(24, 32, CV_8UC3, cv::Scalar(10.0, 20.0, 30.0)));
        }
        writer.release();
    }

    RecordedVideoFixture::~RecordedVideoFixture() noexcept
    {
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }
} /* namespace xwalk::agent::test::computer_vision_opencv */
