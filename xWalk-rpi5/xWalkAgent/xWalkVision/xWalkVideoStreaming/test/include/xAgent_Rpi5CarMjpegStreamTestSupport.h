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

namespace xwalk::agent::test::mjpeg_stream
{

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
agent::string collectResponse(XWalkMjpegHttpServer& server, int descriptor,
    agent::uint64& logicalMilliseconds, agent::uint32 maximumPasses);
/** @brief Closes one test descriptor when it is valid. */
void closeTestDescriptor(int& descriptor) noexcept;

} /* namespace xwalk::agent::test::mjpeg_stream */

#endif /* XAGENT_RPI5CAR_MJPEG_STREAM_TEST_SUPPORT_H */
