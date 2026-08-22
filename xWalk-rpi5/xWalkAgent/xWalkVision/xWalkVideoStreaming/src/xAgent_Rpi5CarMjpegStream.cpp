/******************************************************************************
 * @file        xAgent_Rpi5CarMjpegStream.cpp
 * @brief       Implements the bounded in-process MJPEG stream core.
 * @project     xWalk Firmware
 * @module      xWalkVideoStreaming
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarMjpegStream.h"

#include "xHal_Rpi5CarTrace.h"

#include <algorithm>

namespace xwalk::agent
{

    namespace
    {
        constexpr agent::size MINIMUM_JPEG_BYTES{4U};
        constexpr agent::size MAXIMUM_JPEG_BYTES{agent::size{10U} * 1'024U * 1'024U};
        constexpr agent::uint32 MAXIMUM_CLIENTS{32U};
        constexpr agent::uint32 MAXIMUM_QUEUE_CAPACITY{16U};
        constexpr agent::stringview MULTIPART_BOUNDARY{"--xwalk-frame\r\n"};

        /** @brief Reports whether text is non-empty, single-line, and whitespace-free. */
        agent::boolean validAddressText(agent::stringview address) noexcept
        {
            return !address.empty() && (address.find('\n') == agent::stringview::npos) &&
                   (address.find('\r') == agent::stringview::npos) && (address.find(' ') == agent::stringview::npos) &&
                   (address.find('\t') == agent::stringview::npos);
        }

        /** @brief Returns one client iterator under the caller-held stream lock. */
        std::vector<XWalkMjpegClientState>::iterator findClient(XWalkMjpegStreamState& state,
                                                                agent::uint32 identifier) noexcept
        {
            return std::find_if(state.clients.begin(),
                                state.clients.end(),
                                [identifier](const XWalkMjpegClientState& client)
                                {
                                    return client.identifier == identifier;
                                });
        }

        /** @brief Appends text bytes to one multipart output buffer. */
        void appendText(agent::bytevector& output, agent::stringview text)
        {
            output.insert(output.end(), text.begin(), text.end());
        }
    } /* namespace */

    XWalkMjpegStreamStatus validateMjpegStreamConfiguration(const XWalkMjpegStreamConfiguration& configuration) noexcept
    {
        const agent::boolean loopback = (configuration.bindAddress == "127.0.0.1") ||
                                        (configuration.bindAddress == "localhost") ||
                                        (configuration.bindAddress == "::1");
        const agent::boolean configurationInvalid =
            !validAddressText(configuration.bindAddress) || (!loopback && !configuration.allowExternalBind) ||
            (configuration.port == 0U) || (configuration.port > 65'535U) || (configuration.maximumClients == 0U) ||
            (configuration.maximumClients > MAXIMUM_CLIENTS) || (configuration.queueCapacity == 0U) ||
            (configuration.queueCapacity > MAXIMUM_QUEUE_CAPACITY) ||
            (configuration.maximumJpegBytes < MINIMUM_JPEG_BYTES) ||
            (configuration.maximumJpegBytes > MAXIMUM_JPEG_BYTES);
        if (configurationInvalid)
        {
            return XWalkMjpegStreamStatus::InvalidConfiguration;
        }
        return XWalkMjpegStreamStatus::Ok;
    }

    XWalkMjpegStreamStatus startMjpegStream(XWalkMjpegStreamState& state,
                                            const XWalkMjpegStreamConfiguration& configuration) noexcept
    {
        const XWalkMjpegStreamStatus validationStatus = validateMjpegStreamConfiguration(configuration);
        if (validationStatus != XWalkMjpegStreamStatus::Ok)
        {
            return XWalkMjpegStreamStatus::InvalidConfiguration;
        }
        {
            const std::lock_guard<std::mutex> lock(state.mutex);
            if (state.started)
            {
                return XWalkMjpegStreamStatus::Ok;
            }
            state.configuration = configuration;
            state.clients.clear();
            state.nextSequence = 1U;
            state.cameraAvailable = true;
            state.started = true;
        }
        XWALK_RPIAGENT_TRACE_UID2(RPIAGENT .036,
                                  "MJPEG stream started for %u client(s) with queue capacity %u",
                                  configuration.maximumClients,
                                  configuration.queueCapacity);
        return XWalkMjpegStreamStatus::Ok;
    }

    XWalkMjpegStreamStatus stopMjpegStream(XWalkMjpegStreamState& state) noexcept
    {
        const std::lock_guard<std::mutex> lock(state.mutex);
        state.clients.clear();
        state.cameraAvailable = false;
        state.started = false;
        return XWalkMjpegStreamStatus::Ok;
    }

    XWalkMjpegStreamStatus addMjpegStreamClient(XWalkMjpegStreamState& state, agent::uint32 clientIdentifier) noexcept
    {
        const std::lock_guard<std::mutex> lock(state.mutex);
        if (!state.started)
        {
            return XWalkMjpegStreamStatus::NotStarted;
        }
        if (!state.cameraAvailable)
        {
            return XWalkMjpegStreamStatus::CameraUnavailable;
        }
        if (clientIdentifier == 0U)
        {
            return XWalkMjpegStreamStatus::ClientNotFound;
        }
        const auto existingClient = findClient(state, clientIdentifier);
        const auto clientsEnd = state.clients.end();
        if (existingClient != clientsEnd)
        {
            return XWalkMjpegStreamStatus::Ok;
        }
        const agent::size clientCount = state.clients.size();
        if (clientCount >= state.configuration.maximumClients)
        {
            return XWalkMjpegStreamStatus::ClientLimitReached;
        }
        state.clients.push_back({clientIdentifier, {}, 0U});
        return XWalkMjpegStreamStatus::Ok;
    }

    XWalkMjpegStreamStatus removeMjpegStreamClient(XWalkMjpegStreamState& state,
                                                   agent::uint32 clientIdentifier) noexcept
    {
        const std::lock_guard<std::mutex> lock(state.mutex);
        const auto client = findClient(state, clientIdentifier);
        const auto clientsEnd = state.clients.end();
        if (client == clientsEnd)
        {
            return XWalkMjpegStreamStatus::ClientNotFound;
        }
        state.clients.erase(client);
        return XWalkMjpegStreamStatus::Ok;
    }

    XWalkMjpegStreamStatus publishMjpegFrame(XWalkMjpegStreamState& state, const agent::bytevector& jpeg) noexcept
    {
        const std::lock_guard<std::mutex> lock(state.mutex);
        if (!state.started)
        {
            return XWalkMjpegStreamStatus::NotStarted;
        }
        if (!state.cameraAvailable)
        {
            return XWalkMjpegStreamStatus::CameraUnavailable;
        }
        const agent::size jpegSize = jpeg.size();
        const agent::boolean jpegInvalid =
            (jpegSize < MINIMUM_JPEG_BYTES) || (jpegSize > state.configuration.maximumJpegBytes) ||
            (jpeg[0U] != 0xFFU) || (jpeg[1U] != 0xD8U) || (jpeg[jpegSize - 2U] != 0xFFU) || (jpeg.back() != 0xD9U);
        if (jpegInvalid)
        {
            return XWalkMjpegStreamStatus::InvalidFrame;
        }
        const agent::uint64 sequence = state.nextSequence;
        ++state.nextSequence;
        for (XWalkMjpegClientState& client : state.clients)
        {
            const agent::size pendingCount = client.pending.size();
            if (pendingCount >= state.configuration.queueCapacity)
            {
                client.pending.pop_front();
                ++client.droppedFrames;
            }
            client.pending.push_back({sequence, jpeg});
        }
        return XWalkMjpegStreamStatus::Ok;
    }

    XWalkMjpegStreamStatus popMjpegMultipartFrame(XWalkMjpegStreamState& state,
                                                  agent::uint32 clientIdentifier,
                                                  agent::bytevector& multipartFrame,
                                                  agent::uint64& sequence) noexcept
    {
        const std::lock_guard<std::mutex> lock(state.mutex);
        const auto client = findClient(state, clientIdentifier);
        const auto clientsEnd = state.clients.end();
        if (client == clientsEnd)
        {
            return XWalkMjpegStreamStatus::ClientNotFound;
        }
        const agent::boolean pendingEmpty = client->pending.empty();
        if (pendingEmpty)
        {
            return XWalkMjpegStreamStatus::NoFrame;
        }
        const XWalkMjpegQueuedFrame frame = std::move(client->pending.front());
        client->pending.pop_front();
        const agent::string contentLength = std::to_string(frame.jpeg.size());
        const agent::string sequenceText = std::to_string(frame.sequence);
        multipartFrame.clear();
        multipartFrame.reserve(MULTIPART_BOUNDARY.size() + frame.jpeg.size() + 96U);
        appendText(multipartFrame, MULTIPART_BOUNDARY);
        appendText(multipartFrame, "Content-Type: image/jpeg\r\nContent-Length: ");
        appendText(multipartFrame, contentLength);
        appendText(multipartFrame, "\r\nX-Sequence: ");
        appendText(multipartFrame, sequenceText);
        appendText(multipartFrame, "\r\n\r\n");
        multipartFrame.insert(multipartFrame.end(), frame.jpeg.begin(), frame.jpeg.end());
        appendText(multipartFrame, "\r\n");
        sequence = frame.sequence;
        return XWalkMjpegStreamStatus::Ok;
    }

    XWalkMjpegStreamStatus reportMjpegCameraLoss(XWalkMjpegStreamState& state) noexcept
    {
        const std::lock_guard<std::mutex> lock(state.mutex);
        for (XWalkMjpegClientState& client : state.clients)
        {
            client.pending.clear();
        }
        state.cameraAvailable = false;
        return state.started ? XWalkMjpegStreamStatus::CameraUnavailable : XWalkMjpegStreamStatus::NotStarted;
    }

    agent::string mjpegHttpResponseHeader()
    {
        return "HTTP/1.1 200 OK\r\nCache-Control: no-store\r\n"
               "Content-Type: multipart/x-mixed-replace; boundary=xwalk-frame\r\n\r\n";
    }

} /* namespace xwalk::agent */
