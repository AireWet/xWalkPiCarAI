/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingOpenCv.h
 * @brief       Declares the OpenCV continuous video-recording provider.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording OpenCV Backend
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VIDEO_RECORDING_OPENCV_H
#define XAGENT_RPI5CAR_VIDEO_RECORDING_OPENCV_H

#include "xAgent_Rpi5CarVideoRecording.h"

#include <atomic>
#include <mutex>
#include <opencv2/videoio.hpp>
#include <thread>

namespace xwalk::agent
{

/** @brief Selects how OpenCV opens the recording frame source. */
enum class XWalkVideoRecordingOpenCvBackend : agent::uint8
{
    V4l2 = 0U,
    Gstreamer,
    VideoFile,
    ImageSequence,
    Automatic
};

struct XWalkVideoRecordingOpenCvConfiguration
{
    XWalkVideoRecordingOpenCvBackend cameraBackend{XWalkVideoRecordingOpenCvBackend::V4l2};
    agent::string cameraDevice{};
    agent::string videoDirectory{"/tmp/xwalk-videos"};
    agent::uint32 widthPixels{640U};
    agent::uint32 heightPixels{480U};
    agent::float64 framesPerSecond{20.0};
    /** @brief Best-effort OpenCV frame-read timeout from 1 through 60000 milliseconds. */
    agent::uint32 readTimeoutMilliseconds{1'000U};
};

/** @brief Owns one camera, AVI writer, and continuous capture worker. */
class XWalkVideoRecordingOpenCv final
{
private:
    XWalkVideoRecordingOpenCvConfiguration configurationValue{};
    cv::VideoCapture camera{};
    cv::VideoWriter writer{};
    std::thread captureThread{};
    mutable std::mutex stateMutex{};
    std::atomic<agent::boolean> stopRequested{false};
    std::atomic<agent::boolean> workerFailed{false};
    /** @brief Distinguishes normal finite-source exhaustion from camera failure. */
    std::atomic<agent::boolean> sourceEnded{false};
    /** @brief Actual width reported by the opened camera. */
    agent::uint32 frameWidthPixelsValue{};
    /** @brief Actual height reported by the opened camera. */
    agent::uint32 frameHeightPixelsValue{};
    agent::boolean recordingValue{};
    agent::boolean pausedValue{};
    agent::string outputPathValue{};

protected:
    static XWalkVideoRecordingOpenCv& provider(agent::contextpointer context);
    static agent::boolean startCamera(agent::contextpointer context);
    static void stopCamera(agent::contextpointer context) noexcept;
    static agent::string beginRecording(agent::contextpointer context,
        agent::stringview recordingName);
    static void pauseRecording(agent::contextpointer context);
    static void continueRecording(agent::contextpointer context);
    static void stopRecording(agent::contextpointer context) noexcept;
    static agent::string timestamp(agent::contextpointer context);
    void captureLoop() noexcept;
    void stopProvider() noexcept;

public:
    /** @brief Parses a deployment backend name. */
    static XWalkVideoRecordingOpenCvBackend backendFromString(agent::stringview backend);
    explicit XWalkVideoRecordingOpenCv(
        const XWalkVideoRecordingOpenCvConfiguration& configuration = {});
    ~XWalkVideoRecordingOpenCv() noexcept;

    XWalkVideoRecordingOpenCv(const XWalkVideoRecordingOpenCv&) = delete;
    XWalkVideoRecordingOpenCv(XWalkVideoRecordingOpenCv&&) = delete;
    XWalkVideoRecordingOpenCv& operator=(
        const XWalkVideoRecordingOpenCv&) = delete;
    XWalkVideoRecordingOpenCv& operator=(XWalkVideoRecordingOpenCv&&) = delete;

    XWalkVideoRecordingCallbacks callbacks() const noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VIDEO_RECORDING_OPENCV_H */
