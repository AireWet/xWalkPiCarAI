/******************************************************************************
 * @file        xAgent_Rpi5CarMjpegHttpServer.cpp
 * @brief       Implements the bounded non-blocking MJPEG HTTP transport.
 * @project     xWalk Firmware
 * @module      xWalkVideoStreaming
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarMjpegHttpServer.h"

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <limits>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>

namespace xwalk::agent
{

    namespace
    {
        constexpr agent::size MAXIMUM_REQUEST_BYTES{64U * 1'024U};
        constexpr agent::size MAXIMUM_PENDING_BYTES{16U * 1'024U * 1'024U};
        constexpr agent::uint64 MAXIMUM_TIMEOUT_MILLISECONDS{300'000U};
        constexpr agent::size READ_CHUNK_BYTES{2U * 1'024U};

        /** @brief Saturating-increments one observability counter. */
        void incrementSaturated(agent::uint64& value) noexcept
        {
            if (value != std::numeric_limits<agent::uint64>::max())
            {
                ++value;
            }
        }

        /** @brief Reports whether an address is supported loopback text. */
        agent::boolean loopbackAddress(agent::stringview address) noexcept
        {
            return (address == "127.0.0.1") || (address == "localhost");
        }

        /** @brief Reports whether text is a supported numeric IPv4 listener address. */
        agent::boolean validIpv4Address(agent::stringview address) noexcept
        {
            if (address == "localhost")
            {
                return true;
            }
            if (address.size() > 15U)
            {
                return false;
            }
            in_addr parsed{};
            char owned[16U]{};
            std::copy(address.begin(), address.end(), owned);
            return ::inet_pton(AF_INET, owned, &parsed) == 1;
        }

        /** @brief Returns rollback-safe elapsed logical time. */
        agent::uint64 elapsed(agent::uint64 now, agent::uint64 then) noexcept
        {
            return now >= then ? now - then : 0U;
        }

        /** @brief Reads camera availability under the stream lock. */
        agent::boolean cameraAvailable(const XWalkMjpegHttpServer& server) noexcept
        {
            if (server.stream == nullptr)
            {
                return false;
            }
            const std::lock_guard<std::mutex> lock(server.stream->mutex);
            return server.stream->cameraAvailable;
        }

        /** @brief Makes one socket non-blocking without changing other descriptor flags. */
        agent::boolean makeNonBlocking(int descriptor) noexcept
        {
            const int flags = ::fcntl(descriptor, F_GETFL, 0);
            return (flags >= 0) && (::fcntl(descriptor, F_SETFL, flags | O_NONBLOCK) == 0);
        }

        /** @brief Closes one descriptor exactly once. */
        void closeDescriptor(int& descriptor) noexcept
        {
            if (descriptor >= 0)
            {
                static_cast<void>(::close(descriptor));
                descriptor = -1;
            }
        }

        /** @brief Appends text into bounded pending output. */
        agent::boolean setOutput(XWalkMjpegHttpClient& client,
                                 agent::stringview output,
                                 agent::size maximumBytes,
                                 agent::uint64 nowMilliseconds) noexcept
        {
            if (output.size() > maximumBytes)
            {
                return false;
            }
            client.pendingOutput.assign(output.begin(), output.end());
            client.outputOffset = 0U;
            client.outputQueuedAtMilliseconds = nowMilliseconds;
            return true;
        }

        /** @brief Returns a bounded JSON snapshot without exposing configuration secrets. */
        agent::string statusBody(const XWalkMjpegHttpServer& server)
        {
            agent::boolean cameraAvailable{};
            agent::uint64 nextSequence{};
            agent::uint64 droppedFrames{};
            agent::uint64 eventSequence{};
            agent::uint64 safeStateTransitions{};
            if (server.stream != nullptr)
            {
                const std::lock_guard<std::mutex> lock(server.stream->mutex);
                cameraAvailable = server.stream->cameraAvailable;
                nextSequence = server.stream->nextSequence;
                for (const XWalkMjpegClientState& client : server.stream->clients)
                {
                    if (std::numeric_limits<agent::uint64>::max() - droppedFrames < client.droppedFrames)
                    {
                        droppedFrames = std::numeric_limits<agent::uint64>::max();
                    }
                    else
                    {
                        droppedFrames += client.droppedFrames;
                    }
                }
            }
            if (server.configuration.observability != nullptr)
            {
                const ::xwalk::XWalkFailureSnapshot snapshot =
                    ::xwalk::failureSnapshot(*server.configuration.observability);
                eventSequence = snapshot.nextSequence;
                safeStateTransitions =
                    snapshot.counters[static_cast<agent::size>(::xwalk::XWalkFailureEventId::SafeStateTransition)];
            }
            return agent::string("{\"healthy\":") + (cameraAvailable ? "true" : "false") +
                   ",\"camera_available\":" + (cameraAvailable ? "true" : "false") +
                   ",\"clients\":" + std::to_string(server.clients.size()) +
                   ",\"next_sequence\":" + std::to_string(nextSequence) +
                   ",\"dropped_frames\":" + std::to_string(droppedFrames) +
                   ",\"next_event_sequence\":" + std::to_string(eventSequence) +
                   ",\"safe_state_transitions\":" + std::to_string(safeStateTransitions) +
                   ",\"accepted_clients\":" + std::to_string(server.acceptedClients) +
                   ",\"disconnected_clients\":" + std::to_string(server.disconnectedClients) + "}\n";
        }

        /** @brief Builds one close-delimited HTTP response. */
        agent::string
        httpResponse(int status, agent::stringview reason, agent::stringview contentType, agent::stringview body)
        {
            return "HTTP/1.1 " + std::to_string(status) + " " + agent::string(reason) +
                   "\r\nConnection: close\r\nCache-Control: no-store\r\nContent-Type: " + agent::string(contentType) +
                   "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + agent::string(body);
        }

        /** @brief Extracts an exact bearer token from a complete bounded request. */
        agent::stringview bearerToken(agent::stringview request) noexcept
        {
            constexpr agent::stringview PREFIX{"Authorization: Bearer "};
            agent::size position = request.find(PREFIX);
            if (position == agent::stringview::npos)
            {
                return {};
            }
            position += PREFIX.size();
            const agent::size end = request.find("\r\n", position);
            return request.substr(position,
                                  end == agent::stringview::npos ? request.size() - position : end - position);
        }

        /** @brief Authenticates external requests without retaining the credential. */
        agent::boolean requestAuthorized(const XWalkMjpegHttpServer& server, agent::stringview request) noexcept
        {
            if (loopbackAddress(server.configuration.stream.bindAddress))
            {
                return true;
            }
            return server.configuration.authenticate != nullptr &&
                   server.configuration.authenticate(server.configuration.authenticationContext,
                                                     server.configuration.authenticationReference,
                                                     bearerToken(request));
        }

        /** @brief Parses and routes one complete request into bounded output state. */
        agent::boolean
        routeRequest(XWalkMjpegHttpServer& server, XWalkMjpegHttpClient& client, agent::uint64 nowMilliseconds) noexcept
        {
            XWalkMjpegHttpRequest parsed;
            if (parseMjpegHttpRequest(client.request, server.configuration.maximumRequestBytes, parsed) !=
                XWalkMjpegHttpParseStatus::Ok)
            {
                return false;
            }
            if (!requestAuthorized(server, client.request))
            {
                client.mode = XWalkMjpegHttpClientMode::OneShotResponse;
                return setOutput(client,
                                 httpResponse(401, "Unauthorized", "text/plain", "authentication required\n"),
                                 server.configuration.maximumPendingBytes,
                                 nowMilliseconds);
            }
            if (parsed.path == "/health")
            {
                const agent::boolean healthy = cameraAvailable(server);
                client.mode = XWalkMjpegHttpClientMode::OneShotResponse;
                return setOutput(client,
                                 httpResponse(healthy ? 200 : 503,
                                              healthy ? "OK" : "Service Unavailable",
                                              "application/json",
                                              healthy ? "{\"healthy\":true}\n" : "{\"healthy\":false}\n"),
                                 server.configuration.maximumPendingBytes,
                                 nowMilliseconds);
            }
            if (parsed.path == "/status")
            {
                client.mode = XWalkMjpegHttpClientMode::OneShotResponse;
                return setOutput(client,
                                 httpResponse(200, "OK", "application/json", statusBody(server)),
                                 server.configuration.maximumPendingBytes,
                                 nowMilliseconds);
            }
            if (parsed.path == "/stream")
            {
                if (server.stream == nullptr ||
                    addMjpegStreamClient(*server.stream, client.identifier) != XWalkMjpegStreamStatus::Ok)
                {
                    client.mode = XWalkMjpegHttpClientMode::OneShotResponse;
                    return setOutput(client,
                                     httpResponse(503, "Service Unavailable", "text/plain", "stream unavailable\n"),
                                     server.configuration.maximumPendingBytes,
                                     nowMilliseconds);
                }
                client.mode = XWalkMjpegHttpClientMode::Stream;
                return setOutput(
                    client, mjpegHttpResponseHeader(), server.configuration.maximumPendingBytes, nowMilliseconds);
            }
            client.mode = XWalkMjpegHttpClientMode::OneShotResponse;
            return setOutput(client,
                             httpResponse(404, "Not Found", "text/plain", "not found\n"),
                             server.configuration.maximumPendingBytes,
                             nowMilliseconds);
        }

        /** @brief Removes one client and its matching bounded stream queue. */
        void disconnectClient(XWalkMjpegHttpServer& server, agent::size index, agent::uint64 nowMilliseconds) noexcept
        {
            XWalkMjpegHttpClient& client = server.clients[index];
            if ((client.mode == XWalkMjpegHttpClientMode::Stream) && (server.stream != nullptr))
            {
                static_cast<void>(removeMjpegStreamClient(*server.stream, client.identifier));
            }
            closeDescriptor(client.descriptor);
            server.clients.erase(server.clients.begin() + static_cast<std::ptrdiff_t>(index));
            incrementSaturated(server.disconnectedClients);
            if (server.configuration.observability != nullptr)
            {
                ::xwalk::recordFailureEvent(*server.configuration.observability,
                                            ::xwalk::XWalkFailureEventId::ClientDisconnection,
                                            nowMilliseconds);
            }
        }

        /** @brief Accepts every currently pending connection without blocking. */
        void acceptClients(XWalkMjpegHttpServer& server, agent::uint64 nowMilliseconds) noexcept
        {
            while (true)
            {
                sockaddr_in address{};
                socklen_t size = sizeof(address);
                int descriptor = ::accept(server.listener, reinterpret_cast<sockaddr*>(&address), &size);
                if (descriptor < 0)
                {
                    return;
                }
                if (!makeNonBlocking(descriptor) || server.clients.size() >= server.configuration.stream.maximumClients)
                {
                    if (server.clients.size() >= server.configuration.stream.maximumClients)
                    {
                        incrementSaturated(server.rejectedClients);
                    }
                    closeDescriptor(descriptor);
                    continue;
                }
                XWalkMjpegHttpClient client;
                client.descriptor = descriptor;
                client.identifier = server.nextClientIdentifier;
                ++server.nextClientIdentifier;
                if (server.nextClientIdentifier == 0U)
                {
                    server.nextClientIdentifier = 1U;
                }
                client.connectedAtMilliseconds = nowMilliseconds;
                client.lastProgressMilliseconds = nowMilliseconds;
                client.request.reserve(server.configuration.maximumRequestBytes);
                server.clients.push_back(std::move(client));
                incrementSaturated(server.acceptedClients);
                if (server.configuration.observability != nullptr)
                {
                    ::xwalk::recordFailureEvent(*server.configuration.observability,
                                                ::xwalk::XWalkFailureEventId::ClientConnection,
                                                nowMilliseconds);
                }
            }
        }

        /** @brief Receives bounded request bytes and routes a complete header. */
        agent::boolean
        readClient(XWalkMjpegHttpServer& server, XWalkMjpegHttpClient& client, agent::uint64 nowMilliseconds) noexcept
        {
            char buffer[READ_CHUNK_BYTES];
            const ssize_t count = ::recv(client.descriptor, buffer, sizeof(buffer), 0);
            if (count == 0)
            {
                return false;
            }
            if (count < 0)
            {
                return (errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR);
            }
            client.lastProgressMilliseconds = nowMilliseconds;
            const agent::size received = static_cast<agent::size>(count);
            if (received > server.configuration.maximumRequestBytes - client.request.size())
            {
                incrementSaturated(server.invalidRequests);
                client.mode = XWalkMjpegHttpClientMode::OneShotResponse;
                return setOutput(
                    client,
                    httpResponse(431, "Request Header Fields Too Large", "text/plain", "request too large\n"),
                    server.configuration.maximumPendingBytes,
                    nowMilliseconds);
            }
            client.request.append(buffer, received);
            if (client.request.find("\r\n\r\n") == agent::string::npos)
            {
                return true;
            }
            if (!routeRequest(server, client, nowMilliseconds))
            {
                incrementSaturated(server.invalidRequests);
                client.mode = XWalkMjpegHttpClientMode::OneShotResponse;
                return setOutput(client,
                                 httpResponse(400, "Bad Request", "text/plain", "invalid request\n"),
                                 server.configuration.maximumPendingBytes,
                                 nowMilliseconds);
            }
            client.request.clear();
            return true;
        }

        /** @brief Sends pending output and queues at most one new multipart frame. */
        agent::boolean
        writeClient(XWalkMjpegHttpServer& server, XWalkMjpegHttpClient& client, agent::uint64 nowMilliseconds) noexcept
        {
            if (client.outputOffset >= client.pendingOutput.size())
            {
                client.pendingOutput.clear();
                client.outputOffset = 0U;
                if (client.mode == XWalkMjpegHttpClientMode::OneShotResponse)
                {
                    return false;
                }
                if ((client.mode == XWalkMjpegHttpClientMode::Stream) && (server.stream != nullptr))
                {
                    agent::uint64 sequence{};
                    const XWalkMjpegStreamStatus status =
                        popMjpegMultipartFrame(*server.stream, client.identifier, client.pendingOutput, sequence);
                    if (status == XWalkMjpegStreamStatus::Ok)
                    {
                        client.outputQueuedAtMilliseconds = nowMilliseconds;
                    }
                    else if ((status != XWalkMjpegStreamStatus::NoFrame) &&
                             (status != XWalkMjpegStreamStatus::CameraUnavailable))
                    {
                        return false;
                    }
                }
            }
            if (client.pendingOutput.empty())
            {
                return true;
            }
            const agent::size remaining = client.pendingOutput.size() - client.outputOffset;
            const ssize_t count =
                ::send(client.descriptor, client.pendingOutput.data() + client.outputOffset, remaining, MSG_NOSIGNAL);
            if (count < 0)
            {
                return (errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR);
            }
            if (count == 0)
            {
                return false;
            }
            client.outputOffset += static_cast<agent::size>(count);
            client.lastProgressMilliseconds = nowMilliseconds;
            return true;
        }
    } /* namespace */

    XWalkMjpegHttpStatus validateMjpegHttpConfiguration(const XWalkMjpegHttpConfiguration& configuration) noexcept
    {
        if ((validateMjpegStreamConfiguration(configuration.stream) != XWalkMjpegStreamStatus::Ok) ||
            (configuration.maximumRequestBytes < 64U) || (configuration.maximumRequestBytes > MAXIMUM_REQUEST_BYTES) ||
            (configuration.maximumPendingBytes < 1'024U) ||
            (configuration.maximumPendingBytes > MAXIMUM_PENDING_BYTES) ||
            (configuration.headerTimeoutMilliseconds == 0U) ||
            (configuration.headerTimeoutMilliseconds > MAXIMUM_TIMEOUT_MILLISECONDS) ||
            (configuration.idleTimeoutMilliseconds == 0U) ||
            (configuration.idleTimeoutMilliseconds > MAXIMUM_TIMEOUT_MILLISECONDS) ||
            (configuration.slowClientTimeoutMilliseconds == 0U) ||
            (configuration.slowClientTimeoutMilliseconds > MAXIMUM_TIMEOUT_MILLISECONDS) ||
            !validIpv4Address(configuration.stream.bindAddress))
        {
            return XWalkMjpegHttpStatus::InvalidConfiguration;
        }
        if (!loopbackAddress(configuration.stream.bindAddress) &&
            (configuration.authenticationReference.empty() || (configuration.authenticate == nullptr)))
        {
            return XWalkMjpegHttpStatus::InvalidConfiguration;
        }
        return XWalkMjpegHttpStatus::Ok;
    }

    XWalkMjpegHttpParseStatus
    parseMjpegHttpRequest(agent::stringview input, agent::size maximumBytes, XWalkMjpegHttpRequest& request) noexcept
    {
        request = {};
        if (input.size() > maximumBytes)
        {
            return XWalkMjpegHttpParseStatus::TooLarge;
        }
        if ((input.find('\0') != agent::stringview::npos) || (input.find("\r\n\r\n") == agent::stringview::npos))
        {
            return input.find('\0') != agent::stringview::npos ? XWalkMjpegHttpParseStatus::Invalid
                                                               : XWalkMjpegHttpParseStatus::Incomplete;
        }
        const agent::size lineEnd = input.find("\r\n");
        if (lineEnd == agent::stringview::npos)
        {
            return XWalkMjpegHttpParseStatus::Invalid;
        }
        const agent::stringview line = input.substr(0U, lineEnd);
        constexpr agent::stringview PREFIX{"GET "};
        constexpr agent::stringview SUFFIX{" HTTP/1.1"};
        if ((line.size() <= PREFIX.size() + SUFFIX.size()) || (line.substr(0U, PREFIX.size()) != PREFIX) ||
            (line.substr(line.size() - SUFFIX.size()) != SUFFIX))
        {
            return XWalkMjpegHttpParseStatus::Invalid;
        }
        request.path = line.substr(PREFIX.size(), line.size() - PREFIX.size() - SUFFIX.size());
        if (request.path.empty() || (request.path.front() != '/') ||
            (request.path.find(' ') != agent::stringview::npos) || (request.path.find('\t') != agent::stringview::npos))
        {
            request = {};
            return XWalkMjpegHttpParseStatus::Invalid;
        }
        request.bearerToken = bearerToken(input);
        return XWalkMjpegHttpParseStatus::Ok;
    }

    XWalkMjpegHttpStatus startMjpegHttpServer(XWalkMjpegHttpServer& server,
                                              XWalkMjpegStreamState& stream,
                                              const XWalkMjpegHttpConfiguration& configuration) noexcept
    {
        if (server.started)
        {
            return XWalkMjpegHttpStatus::Ok;
        }
        if (validateMjpegHttpConfiguration(configuration) != XWalkMjpegHttpStatus::Ok)
        {
            return XWalkMjpegHttpStatus::InvalidConfiguration;
        }
        int listener = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listener < 0 || !makeNonBlocking(listener))
        {
            closeDescriptor(listener);
            return XWalkMjpegHttpStatus::SocketFailure;
        }
        int reuseAddress = 1;
        static_cast<void>(::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress)));
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<in_port_t>(configuration.stream.port));
        const agent::string bindAddress =
            configuration.stream.bindAddress == "localhost" ? "127.0.0.1" : configuration.stream.bindAddress;
        if ((::inet_pton(AF_INET, bindAddress.c_str(), &address.sin_addr) != 1) ||
            (::bind(listener, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) ||
            (::listen(listener, static_cast<int>(configuration.stream.maximumClients)) != 0))
        {
            closeDescriptor(listener);
            return XWalkMjpegHttpStatus::SocketFailure;
        }
        if (startMjpegStream(stream, configuration.stream) != XWalkMjpegStreamStatus::Ok)
        {
            closeDescriptor(listener);
            return XWalkMjpegHttpStatus::InvalidConfiguration;
        }
        server.configuration = configuration;
        server.stream = &stream;
        server.listener = listener;
        server.clients.clear();
        server.nextClientIdentifier = 1U;
        server.started = true;
        if (server.configuration.observability != nullptr)
        {
            ::xwalk::recordFailureEvent(
                *server.configuration.observability, ::xwalk::XWalkFailureEventId::StreamStart, 0U);
        }
        return XWalkMjpegHttpStatus::Ok;
    }

    XWalkMjpegHttpStatus pumpMjpegHttpServer(XWalkMjpegHttpServer& server, agent::uint64 nowMilliseconds) noexcept
    {
        if (!server.started || server.listener < 0)
        {
            return XWalkMjpegHttpStatus::NotStarted;
        }
        acceptClients(server, nowMilliseconds);
        agent::size index = 0U;
        while (index < server.clients.size())
        {
            XWalkMjpegHttpClient& client = server.clients[index];
            agent::boolean keep = true;
            if (client.mode == XWalkMjpegHttpClientMode::ReadingRequest)
            {
                keep = readClient(server, client, nowMilliseconds);
            }
            if (keep && (client.mode != XWalkMjpegHttpClientMode::ReadingRequest))
            {
                keep = writeClient(server, client, nowMilliseconds);
            }
            const agent::boolean headerTimedOut = (client.mode == XWalkMjpegHttpClientMode::ReadingRequest) &&
                                                  (elapsed(nowMilliseconds, client.connectedAtMilliseconds) >=
                                                   server.configuration.headerTimeoutMilliseconds);
            const agent::boolean idleTimedOut = elapsed(nowMilliseconds, client.lastProgressMilliseconds) >=
                                                server.configuration.idleTimeoutMilliseconds;
            const agent::boolean slowTimedOut =
                !client.pendingOutput.empty() && (elapsed(nowMilliseconds, client.outputQueuedAtMilliseconds) >=
                                                  server.configuration.slowClientTimeoutMilliseconds);
            if (!keep || headerTimedOut || idleTimedOut || slowTimedOut)
            {
                if (headerTimedOut || idleTimedOut || slowTimedOut)
                {
                    incrementSaturated(server.timedOutClients);
                    if (slowTimedOut && (server.configuration.observability != nullptr))
                    {
                        ::xwalk::recordFailureEvent(*server.configuration.observability,
                                                    ::xwalk::XWalkFailureEventId::SlowClientDisconnection,
                                                    nowMilliseconds);
                    }
                }
                disconnectClient(server, index, nowMilliseconds);
            }
            else
            {
                ++index;
            }
        }
        return XWalkMjpegHttpStatus::Ok;
    }

    XWalkMjpegHttpStatus stopMjpegHttpServer(XWalkMjpegHttpServer& server) noexcept
    {
        const agent::boolean wasStarted = server.started;
        ::xwalk::XWalkFailureObservability* observability = server.configuration.observability;
        while (!server.clients.empty())
        {
            disconnectClient(server, server.clients.size() - 1U, 0U);
        }
        closeDescriptor(server.listener);
        if (server.stream != nullptr)
        {
            static_cast<void>(stopMjpegStream(*server.stream));
        }
        server.stream = nullptr;
        server.started = false;
        if (wasStarted && (observability != nullptr))
        {
            ::xwalk::recordFailureEvent(*observability, ::xwalk::XWalkFailureEventId::StreamStop, 0U);
        }
        return XWalkMjpegHttpStatus::Ok;
    }

    agent::uint32 mjpegHttpServerPort(const XWalkMjpegHttpServer& server) noexcept
    {
        if (!server.started || server.listener < 0)
        {
            return 0U;
        }
        sockaddr_in address{};
        socklen_t addressLength = sizeof(address);
        if (::getsockname(server.listener, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0)
        {
            return 0U;
        }
        return ntohs(address.sin_port);
    }

} /* namespace xwalk::agent */
