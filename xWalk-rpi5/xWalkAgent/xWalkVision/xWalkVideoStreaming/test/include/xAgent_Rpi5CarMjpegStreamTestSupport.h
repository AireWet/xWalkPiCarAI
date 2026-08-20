/******************************************************************************
 * @file        xAgent_Rpi5CarMjpegStreamTestSupport.h
 * @brief       Declares reusable deterministic MJPEG test data.
 * @project     xWalk Firmware
 * @module      xWalkVideoStreamingTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_MJPEG_STREAM_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_MJPEG_STREAM_TEST_SUPPORT_H

#include "xAgent_Rpi5CarMjpegStream.h"
#include "xAgent_Rpi5CarMjpegHttpServer.h"
#include "xAgent_Rpi5CarVideoStreaming.h"

namespace xwalk::agent::test::mjpeg_stream
{

    /**
     * @struct XWalkVideoStreamingTestState
     * @brief Retains deterministic fake-camera state shared by streaming tests.
     */
    struct XWalkVideoStreamingTestState
    {
            /** @brief True while the fake camera is started. */
            agent::boolean cameraStarted{};
            /** @brief Selects whether the next fake capture succeeds. */
            agent::boolean captureAvailable{true};
            /** @brief Number of fake frames captured since construction. */
            agent::uint32 capturedFrames{};
            /** @brief Fake monotonic clock value in milliseconds. */
            agent::uint64 nowMilliseconds{};
    };

    /** @brief Marks the fake HAL camera in `context` as started. */
    agent::boolean startVideoCamera(agent::contextpointer context,
                                    const hal::XWalkCameraStreamConfiguration& configuration);
    /** @brief Marks the fake camera in `context` as stopped. */
    void stopVideoCamera(agent::contextpointer context) noexcept;
    /** @brief Produces one deterministic JPEG-like fake HAL frame when enabled. */
    agent::boolean captureVideoFrame(agent::contextpointer context,
                                     const hal::XWalkCameraStreamConfiguration& configuration,
                                     agent::bytevector& jpeg);
    /** @brief Returns the fake monotonic millisecond clock from `context`. */
    agent::uint64 videoClock(agent::contextpointer context) noexcept;
    /** @brief Creates the complete fake HAL camera callback table. */
    hal::XWalkCameraStreamCallbacks videoStreamingCallbacks() noexcept;

    /** @brief Creates one minimal marker-bearing JPEG-like bounded test payload. */
    agent::bytevector jpegFrame(agent::uint8 marker, agent::size payloadBytes);
    /** @brief Converts multipart bytes to text for header assertions. */
    agent::string multipartText(const agent::bytevector& multipart);
    /** @brief Reserves and releases one currently available loopback TCP port. */
    agent::uint32 availableLoopbackPort() noexcept;
    /** @brief Opens one blocking loopback client for the configured server port. */
    int connectLoopback(agent::uint32 port) noexcept;
    /** @brief Sends one complete short test request. */
    agent::boolean sendRequest(int descriptor, agent::stringview request) noexcept;
    /** @brief Pumps a server and collects response bytes without sleeping. */
    agent::string collectResponse(XWalkMjpegHttpServer& server,
                                  int descriptor,
                                  agent::uint64& logicalMilliseconds,
                                  agent::uint32 maximumPasses);
    /** @brief Closes one test descriptor when it is valid. */
    void closeTestDescriptor(int& descriptor) noexcept;

} /* namespace xwalk::agent::test::mjpeg_stream */

#endif /* XAGENT_RPI5CAR_MJPEG_STREAM_TEST_SUPPORT_H */
