/******************************************************************************
 * @file        xAgent_Rpi5CarMjpegHttpServer.h
 * @brief       Declares the bounded non-blocking MJPEG HTTP transport.
 * @project     xWalk Firmware
 * @module      xWalkVideoStreaming
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_MJPEG_HTTP_SERVER_H
#define XAGENT_RPI5CAR_MJPEG_HTTP_SERVER_H

#include "xAgent_Rpi5CarMjpegStream.h"
#include "xWalk_Rpi5CarFailureObservability.h"

#include <vector>

namespace xwalk::agent
{

/** @brief Authenticates a supplied bearer token against a configured secret reference. */
using XWalkMjpegAuthenticate = agent::boolean (*)(agent::contextpointer context,
    agent::stringview secretReference, agent::stringview suppliedToken) noexcept;

/** @brief Reports one non-throwing HTTP transport operation result. */
enum class XWalkMjpegHttpStatus : agent::uint8
{
    Ok = 0U,
    InvalidConfiguration,
    NotStarted,
    SocketFailure
};

/** @brief Reports bounded HTTP header parsing without allocation. */
enum class XWalkMjpegHttpParseStatus : agent::uint8
{
    Ok = 0U,
    Incomplete,
    TooLarge,
    Invalid
};

/** @brief Stores non-owning fields from one validated GET request. */
struct XWalkMjpegHttpRequest
{
    agent::stringview path{}; /**< Exact origin-form request path. */
    agent::stringview bearerToken{}; /**< Optional bearer token without prefix. */
};

/** @brief Stores bounded HTTP transport and exposure policy. */
struct XWalkMjpegHttpConfiguration
{
    XWalkMjpegStreamConfiguration stream{}; /**< Existing queue and listener settings. */
    agent::size maximumRequestBytes{8U * 1'024U}; /**< Maximum buffered HTTP header bytes. */
    agent::size maximumPendingBytes{4U * 1'024U * 1'024U}; /**< Per-client send bound. */
    agent::uint64 headerTimeoutMilliseconds{2'000U}; /**< Header completion deadline. */
    agent::uint64 idleTimeoutMilliseconds{15'000U}; /**< No-progress disconnect deadline. */
    agent::uint64 slowClientTimeoutMilliseconds{5'000U}; /**< Pending-output deadline. */
    agent::string authenticationReference{}; /**< Secret-store reference, never a credential. */
    agent::contextpointer authenticationContext{}; /**< Caller-owned authentication context. */
    XWalkMjpegAuthenticate authenticate{}; /**< Required for every external bind. */
    ::xwalk::XWalkFailureObservability* observability{}; /**< Optional network-path event sink. */
};

/** @brief Identifies one active HTTP response mode. */
enum class XWalkMjpegHttpClientMode : agent::uint8
{
    ReadingRequest = 0U,
    OneShotResponse,
    Stream
};

/** @brief Stores one bounded non-blocking socket client. */
struct XWalkMjpegHttpClient
{
    int descriptor{-1}; /**< Owned non-blocking connected socket. */
    agent::uint32 identifier{}; /**< Matching in-process stream-client identifier. */
    agent::string request{}; /**< Bounded request header under construction. */
    agent::bytevector pendingOutput{}; /**< Bounded response bytes awaiting send. */
    agent::size outputOffset{}; /**< First unsent byte in pending output. */
    agent::uint64 connectedAtMilliseconds{}; /**< Monotonic accept time. */
    agent::uint64 lastProgressMilliseconds{}; /**< Most recent successful read or write. */
    agent::uint64 outputQueuedAtMilliseconds{}; /**< Time current output became pending. */
    XWalkMjpegHttpClientMode mode{XWalkMjpegHttpClientMode::ReadingRequest};
};

/** @brief Stores pump-driven HTTP server state without a worker thread. */
struct XWalkMjpegHttpServer
{
    XWalkMjpegHttpConfiguration configuration{}; /**< Active validated configuration. */
    XWalkMjpegStreamState* stream{}; /**< Caller-owned bounded stream core. */
    int listener{-1}; /**< Owned non-blocking listening socket. */
    std::vector<XWalkMjpegHttpClient> clients{}; /**< Bounded active client set. */
    agent::uint32 nextClientIdentifier{1U}; /**< Monotonic logical client identifier. */
    agent::uint64 acceptedClients{}; /**< Saturating accepted-client counter. */
    agent::uint64 disconnectedClients{}; /**< Saturating disconnected-client counter. */
    agent::uint64 rejectedClients{}; /**< Saturating client-limit counter. */
    agent::uint64 invalidRequests{}; /**< Saturating malformed-request counter. */
    agent::uint64 timedOutClients{}; /**< Saturating timeout counter. */
    agent::boolean started{}; /**< True while listener ownership is active. */
};

/** @brief Validates transport bounds, external binding and authentication policy. */
XWalkMjpegHttpStatus validateMjpegHttpConfiguration(
    const XWalkMjpegHttpConfiguration& configuration) noexcept;
/** @brief Parses one complete bounded HTTP/1.1 GET header for routing and fuzzing. */
XWalkMjpegHttpParseStatus parseMjpegHttpRequest(agent::stringview input,
    agent::size maximumBytes, XWalkMjpegHttpRequest& request) noexcept;
/** @brief Opens the configured non-blocking listener and starts the queue core. */
XWalkMjpegHttpStatus startMjpegHttpServer(XWalkMjpegHttpServer& server,
    XWalkMjpegStreamState& stream,
    const XWalkMjpegHttpConfiguration& configuration) noexcept;
/** @brief Performs one bounded accept/read/write pass without blocking. */
XWalkMjpegHttpStatus pumpMjpegHttpServer(XWalkMjpegHttpServer& server,
    agent::uint64 nowMilliseconds) noexcept;
/** @brief Idempotently closes every socket and clears queued stream data. */
XWalkMjpegHttpStatus stopMjpegHttpServer(XWalkMjpegHttpServer& server) noexcept;
/** @brief Returns the listener port in host byte order, or zero when stopped. */
agent::uint32 mjpegHttpServerPort(const XWalkMjpegHttpServer& server) noexcept;

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_MJPEG_HTTP_SERVER_H */
