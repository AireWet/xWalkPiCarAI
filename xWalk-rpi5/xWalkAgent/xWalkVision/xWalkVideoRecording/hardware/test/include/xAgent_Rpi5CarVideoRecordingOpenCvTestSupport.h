/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingOpenCvTestSupport.h
 * @brief       Declares deterministic recorded-video test support.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording OpenCV Host Test
 * @author      Joxy John
 * @date        2026-08-11
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VIDEO_RECORDING_OPENCV_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_VIDEO_RECORDING_OPENCV_TEST_SUPPORT_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::agent::test::video_recording_opencv
{

/** @brief Owns one isolated, programmatically generated video fixture. */
struct RecordedVideoFixture
{
    /** @brief Isolated temporary directory. */
    agent::filesystempath directory{};
    /** @brief Generated finite AVI source. */
    agent::filesystempath video{};

    /** @brief Generates a small local AVI without using a camera device. */
    RecordedVideoFixture();
    /** @brief Removes the isolated fixture directory without throwing. */
    ~RecordedVideoFixture() noexcept;
    /** @brief Disables copying of fixture ownership. */
    RecordedVideoFixture(const RecordedVideoFixture&) = delete;
    /** @brief Disables assignment of fixture ownership. */
    RecordedVideoFixture& operator=(const RecordedVideoFixture&) = delete;
};

} /* namespace xwalk::agent::test::video_recording_opencv */

#endif /* XAGENT_RPI5CAR_VIDEO_RECORDING_OPENCV_TEST_SUPPORT_H */
