/******************************************************************************
 * @file        xAgent_Rpi5CarMjpegHttpServer.cpp
 * @brief       Implements the bounded non-blocking MJPEG HTTP transport.
 * @project     xWalk Firmware
 * @module      xWalkVideoStreaming
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarMjpegHttpServer.h"
#include "xHal_Rpi5CarTrace.h"

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
        constexpr agent::size MAXIMUM_REQUEST_BYTES{agent::size{64U} * 1'024U};
        constexpr agent::size MAXIMUM_PENDING_BYTES{agent::size{16U} * 1'024U * 1'024U};
        constexpr agent::uint64 MAXIMUM_TIMEOUT_MILLISECONDS{300'000U};
        constexpr agent::size READ_CHUNK_BYTES{agent::size{2U} * 1'024U};

        /** @brief Saturating-increments one observability counter. */
        void incrementSaturated(agent::uint64& value) noexcept
        {
            const agent::uint64 maximumValue = std::numeric_limits<agent::uint64>::max();
            if (value != maximumValue)
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
            const agent::size addressSize = address.size();
            if (addressSize > 15U)
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
            const agent::size outputSize = output.size();
            if (outputSize > maximumBytes)
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
                    const agent::uint64 maximumDroppedFrames = std::numeric_limits<agent::uint64>::max();
                    if (maximumDroppedFrames - droppedFrames < client.droppedFrames)
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
            const agent::boolean loopback = loopbackAddress(server.configuration.stream.bindAddress);
            if (loopback)
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
            const XWalkMjpegHttpParseStatus parseStatus =
                parseMjpegHttpRequest(client.request, server.configuration.maximumRequestBytes, parsed);
            if (parseStatus != XWalkMjpegHttpParseStatus::Ok)
            {
                return false;
            }
            const agent::boolean authorized = requestAuthorized(server, client.request);
            if (!authorized)
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
                const agent::boolean streamUnavailable =
                    server.stream == nullptr ||
                    addMjpegStreamClient(*server.stream, client.identifier) != XWalkMjpegStreamStatus::Ok;
                if (streamUnavailable)
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
                const agent::boolean descriptorReady = makeNonBlocking(descriptor);
                const agent::size clientCount = server.clients.size();
                const agent::boolean clientLimitReached = clientCount >= server.configuration.stream.maximumClients;
                if (!descriptorReady || clientLimitReached)
                {
                    if (clientLimitReached)
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
            const agent::size requestSize = client.request.size();
            if (received > server.configuration.maximumRequestBytes - requestSize)
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
            const agent::size headerEnd = client.request.find("\r\n\r\n");
            if (headerEnd == agent::string::npos)
            {
                return true;
            }
            const agent::boolean requestRouted = routeRequest(server, client, nowMilliseconds);
            if (!requestRouted)
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
            const agent::size pendingOutputSize = client.pendingOutput.size();
            if (client.outputOffset >= pendingOutputSize)
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
            const agent::boolean pendingOutputEmpty = client.pendingOutput.empty();
            if (pendingOutputEmpty)
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
        const agent::boolean configurationInvalid =
            (validateMjpegStreamConfiguration(configuration.stream) != XWalkMjpegStreamStatus::Ok) ||
            (configuration.maximumRequestBytes < 64U) || (configuration.maximumRequestBytes > MAXIMUM_REQUEST_BYTES) ||
            (configuration.maximumPendingBytes < 1'024U) ||
            (configuration.maximumPendingBytes > MAXIMUM_PENDING_BYTES) ||
            (configuration.headerTimeoutMilliseconds == 0U) ||
            (configuration.headerTimeoutMilliseconds > MAXIMUM_TIMEOUT_MILLISECONDS) ||
            (configuration.idleTimeoutMilliseconds == 0U) ||
            (configuration.idleTimeoutMilliseconds > MAXIMUM_TIMEOUT_MILLISECONDS) ||
            (configuration.slowClientTimeoutMilliseconds == 0U) ||
            (configuration.slowClientTimeoutMilliseconds > MAXIMUM_TIMEOUT_MILLISECONDS) ||
            !validIpv4Address(configuration.stream.bindAddress);
        if (configurationInvalid)
        {
            return XWalkMjpegHttpStatus::InvalidConfiguration;
        }
        const agent::boolean loopback = loopbackAddress(configuration.stream.bindAddress);
        const agent::boolean authenticationReferenceEmpty = configuration.authenticationReference.empty();
        if (!loopback && (authenticationReferenceEmpty || (configuration.authenticate == nullptr)))
        {
            return XWalkMjpegHttpStatus::InvalidConfiguration;
        }
        return XWalkMjpegHttpStatus::Ok;
    }

    XWalkMjpegHttpParseStatus
    parseMjpegHttpRequest(agent::stringview input, agent::size maximumBytes, XWalkMjpegHttpRequest& request) noexcept
    {
        request = {};
        const agent::size inputSize = input.size();
        if (inputSize > maximumBytes)
        {
            return XWalkMjpegHttpParseStatus::TooLarge;
        }
        const agent::size nullPosition = input.find('\0');
        const agent::size headerEnd = input.find("\r\n\r\n");
        if ((nullPosition != agent::stringview::npos) || (headerEnd == agent::stringview::npos))
        {
            return nullPosition != agent::stringview::npos ? XWalkMjpegHttpParseStatus::Invalid
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
        const agent::size lineSize = line.size();
        const agent::size prefixSize = PREFIX.size();
        const agent::size suffixSize = SUFFIX.size();
        const agent::boolean requestLineInvalid = (lineSize <= prefixSize + suffixSize) ||
                                                  (line.substr(0U, prefixSize) != PREFIX) ||
                                                  (line.substr(lineSize - suffixSize) != SUFFIX);
        if (requestLineInvalid)
        {
            return XWalkMjpegHttpParseStatus::Invalid;
        }
        request.path = line.substr(PREFIX.size(), line.size() - PREFIX.size() - SUFFIX.size());
        const agent::boolean requestPathInvalid = request.path.empty() || (request.path.front() != '/') ||
                                                  (request.path.find(' ') != agent::stringview::npos) ||
                                                  (request.path.find('\t') != agent::stringview::npos);
        if (requestPathInvalid)
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
        const XWalkMjpegHttpStatus validationStatus = validateMjpegHttpConfiguration(configuration);
        if (validationStatus != XWalkMjpegHttpStatus::Ok)
        {
            return XWalkMjpegHttpStatus::InvalidConfiguration;
        }
        int listener = ::socket(AF_INET, SOCK_STREAM, 0);
        const agent::boolean listenerReady = listener >= 0 && makeNonBlocking(listener);
        if (!listenerReady)
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
        const int addressResult = ::inet_pton(AF_INET, bindAddress.c_str(), &address.sin_addr);
        const int bindResult =
            addressResult == 1 ? ::bind(listener, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) : -1;
        const int listenResult =
            bindResult == 0 ? ::listen(listener, static_cast<int>(configuration.stream.maximumClients)) : -1;
        if ((addressResult != 1) || (bindResult != 0) || (listenResult != 0))
        {
            closeDescriptor(listener);
            return XWalkMjpegHttpStatus::SocketFailure;
        }
        const XWalkMjpegStreamStatus streamStatus = startMjpegStream(stream, configuration.stream);
        if (streamStatus != XWalkMjpegStreamStatus::Ok)
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
        XWALK_RPIAGENT_TRACE_UID1(RPIAGENT .084, "MJPEG HTTP server started on port %u", configuration.stream.port);
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
        agent::size clientCount = server.clients.size();
        while (index < clientCount)
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
            clientCount = server.clients.size();
        }
        return XWalkMjpegHttpStatus::Ok;
    }

    XWalkMjpegHttpStatus stopMjpegHttpServer(XWalkMjpegHttpServer& server) noexcept
    {
        const agent::boolean wasStarted = server.started;
        ::xwalk::XWalkFailureObservability* observability = server.configuration.observability;
        agent::boolean clientsAvailable = !server.clients.empty();
        while (clientsAvailable)
        {
            disconnectClient(server, server.clients.size() - 1U, 0U);
            clientsAvailable = !server.clients.empty();
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
        const int socketNameResult =
            ::getsockname(server.listener, reinterpret_cast<sockaddr*>(&address), &addressLength);
        if (socketNameResult != 0)
        {
            return 0U;
        }
        return ntohs(address.sin_port);
    }

} /* namespace xwalk::agent */
