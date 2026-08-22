/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingOpenCvTestSupport.cpp
 * @brief       Implements deterministic recorded-video test support.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording OpenCV Host Test
 * @author      Joxy John
 * @date        2026-08-11
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoRecordingOpenCvTestSupport.h"
#include "xHal_Rpi5CarTrace.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <chrono>
#include <filesystem>

namespace xwalk::agent::test::video_recording_opencv
{

    /** @brief Generates a small local AVI without using a camera device. */
    RecordedVideoFixture::RecordedVideoFixture()
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        directory = agent::filesystempath("/tmp") / (agent::string("xwalk-video-recording-") + std::to_string(stamp));
        std::filesystem::create_directories(directory);
        video = directory / "finite-source.avi";
        cv::VideoWriter writer(video.string(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10.0, cv::Size(32, 24));
        const agent::boolean writerOpen = writer.isOpened();
        if (!writerOpen)
        {
            XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, "Video-recording fixture could not open its writer");
        }
        for (agent::uint32 index = 0U; index < 8U; ++index)
        {
            writer.write(cv::Mat(24, 32, CV_8UC3, cv::Scalar(static_cast<double>(index), 20.0, 30.0)));
        }
        writer.release();
    }

    /** @brief Removes the isolated fixture directory without throwing. */
    RecordedVideoFixture::~RecordedVideoFixture() noexcept
    {
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }

} /* namespace xwalk::agent::test::video_recording_opencv */
