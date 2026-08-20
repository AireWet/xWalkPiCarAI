/******************************************************************************
 * @file        xAgent_Rpi5CarMjpegStreamTestSupport.cpp
 * @brief       Implements reusable deterministic MJPEG test data.
 * @project     xWalk Firmware
 * @module      xWalkVideoStreamingTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarMjpegStreamTestSupport.h"

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace xwalk::agent::test::mjpeg_stream
{

    agent::boolean startVideoCamera(agent::contextpointer context,
                                    const hal::XWalkCameraStreamConfiguration& configuration)
    {
        static_cast<void>(configuration);
        XWalkVideoStreamingTestState& state = *static_cast<XWalkVideoStreamingTestState*>(context);
        state.cameraStarted = true;
        return true;
    }

    void stopVideoCamera(agent::contextpointer context) noexcept
    {
        XWalkVideoStreamingTestState& state = *static_cast<XWalkVideoStreamingTestState*>(context);
        state.cameraStarted = false;
    }

    agent::boolean captureVideoFrame(agent::contextpointer context,
                                     const hal::XWalkCameraStreamConfiguration& configuration,
                                     agent::bytevector& jpeg)
    {
        static_cast<void>(configuration);
        XWalkVideoStreamingTestState& state = *static_cast<XWalkVideoStreamingTestState*>(context);
        if (state.captureAvailable == false)
        {
            return false;
        }
        ++state.capturedFrames;
        jpeg = jpegFrame(0x45U, 32U);
        return true;
    }

    agent::uint64 videoClock(agent::contextpointer context) noexcept
    {
        XWalkVideoStreamingTestState& state = *static_cast<XWalkVideoStreamingTestState*>(context);
        ++state.nowMilliseconds;
        return state.nowMilliseconds;
    }

    hal::XWalkCameraStreamCallbacks videoStreamingCallbacks() noexcept
    {
        return {&startVideoCamera, &stopVideoCamera, &captureVideoFrame};
    }

    agent::bytevector jpegFrame(agent::uint8 marker, agent::size payloadBytes)
    {
        const agent::size size = payloadBytes < 5U ? 5U : payloadBytes;
        agent::bytevector frame(size, marker);
        frame[0U] = 0xFFU;
        frame[1U] = 0xD8U;
        frame[size - 2U] = 0xFFU;
        frame[size - 1U] = 0xD9U;
        return frame;
    }

    agent::string multipartText(const agent::bytevector& multipart)
    {
        return agent::string(multipart.begin(), multipart.end());
    }

    agent::uint32 availableLoopbackPort() noexcept
    {
        int descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
        if (descriptor < 0)
        {
            return 0U;
        }
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0U;
        if (::bind(descriptor, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
        {
            closeTestDescriptor(descriptor);
            return 0U;
        }
        socklen_t size = sizeof(address);
        if (::getsockname(descriptor, reinterpret_cast<sockaddr*>(&address), &size) != 0)
        {
            closeTestDescriptor(descriptor);
            return 0U;
        }
        const agent::uint32 port = ntohs(address.sin_port);
        closeTestDescriptor(descriptor);
        return port;
    }

    int connectLoopback(agent::uint32 port) noexcept
    {
        int descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
        if (descriptor < 0)
        {
            return -1;
        }
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(static_cast<in_port_t>(port));
        if (::connect(descriptor, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
        {
            closeTestDescriptor(descriptor);
            return -1;
        }
        const int flags = ::fcntl(descriptor, F_GETFL, 0);
        if ((flags < 0) || (::fcntl(descriptor, F_SETFL, flags | O_NONBLOCK) != 0))
        {
            closeTestDescriptor(descriptor);
            return -1;
        }
        return descriptor;
    }

    agent::boolean sendRequest(int descriptor, agent::stringview request) noexcept
    {
        agent::size sent{};
        while (sent < request.size())
        {
            const ssize_t count = ::send(descriptor, request.data() + sent, request.size() - sent, MSG_NOSIGNAL);
            if (count <= 0)
            {
                return false;
            }
            sent += static_cast<agent::size>(count);
        }
        return true;
    }

    agent::string collectResponse(XWalkMjpegHttpServer& server,
                                  int descriptor,
                                  agent::uint64& logicalMilliseconds,
                                  agent::uint32 maximumPasses)
    {
        agent::string response;
        char buffer[8U * 1'024U];
        for (agent::uint32 pass = 0U; pass < maximumPasses; ++pass)
        {
            ++logicalMilliseconds;
            static_cast<void>(pumpMjpegHttpServer(server, logicalMilliseconds));
            while (true)
            {
                const ssize_t count = ::recv(descriptor, buffer, sizeof(buffer), 0);
                if (count > 0)
                {
                    response.append(buffer, static_cast<agent::size>(count));
                    continue;
                }
                if ((count == 0) || ((errno != EAGAIN) && (errno != EWOULDBLOCK) && (errno != EINTR)))
                {
                    return response;
                }
                break;
            }
        }
        return response;
    }

    void closeTestDescriptor(int& descriptor) noexcept
    {
        if (descriptor >= 0)
        {
            static_cast<void>(::close(descriptor));
            descriptor = -1;
        }
    }

} /* namespace xwalk::agent::test::mjpeg_stream */
