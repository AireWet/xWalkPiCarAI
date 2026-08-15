/******************************************************************************
 * @file        xAgent_Rpi5CarMjpegStream.h
 * @brief       Declares the bounded in-process MJPEG stream core.
 * @project     xWalk Firmware
 * @module      xWalkVideoStreaming
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_MJPEG_STREAM_H
#define XAGENT_RPI5CAR_MJPEG_STREAM_H

#include "xHal_Rpi5CarTypes.h"

#include <deque>
#include <mutex>
#include <vector>

namespace xwalk::agent
{

    /** @brief Reports one non-throwing MJPEG stream operation result. */
    enum class XWalkMjpegStreamStatus : agent::uint8
    {
        Ok = 0U,
        InvalidConfiguration,
        NotStarted,
        ClientLimitReached,
        ClientNotFound,
        InvalidFrame,
        NoFrame,
        CameraUnavailable
    };

    /** @brief Stores bounded stream and network-exposure policy. */
    struct XWalkMjpegStreamConfiguration
    {
            agent::string bindAddress{"127.0.0.1"}; /**< Listener address reserved for a transport adapter. */
            agent::uint32 port{8'080U};             /**< Listener port from one through 65,535. */
            agent::uint32 maximumClients{4U};       /**< Client limit from one through 32. */
            agent::uint32 queueCapacity{2U};        /**< Per-client retained-frame limit from one through 16. */
            agent::size maximumJpegBytes{2U * 1'024U * 1'024U}; /**< Maximum accepted JPEG bytes. */
            agent::boolean allowExternalBind{};                 /**< Explicit opt-in for non-loopback binding. */
    };

    /** @brief Stores one sequence-numbered encoded JPEG awaiting one client. */
    struct XWalkMjpegQueuedFrame
    {
            agent::uint64 sequence{}; /**< Monotonic stream sequence beginning at one. */
            agent::bytevector jpeg{}; /**< Owned bounded JPEG payload. */
    };

    /** @brief Stores one logical stream client and its bounded pending queue. */
    struct XWalkMjpegClientState
    {
            agent::uint32 identifier{};                  /**< Caller-selected non-zero logical client identifier. */
            std::deque<XWalkMjpegQueuedFrame> pending{}; /**< Oldest-first bounded frame queue. */
            agent::uint64 droppedFrames{};               /**< Frames discarded for this slow client. */
    };

    /** @brief Stores synchronized in-process MJPEG stream state without opening sockets. */
    struct XWalkMjpegStreamState
    {
            XWalkMjpegStreamConfiguration configuration{}; /**< Active validated settings. */
            std::vector<XWalkMjpegClientState> clients{};  /**< Active logical clients. */
            agent::uint64 nextSequence{1U};                /**< Next frame sequence number. */
            agent::boolean started{};                      /**< True after successful explicit start. */
            agent::boolean cameraAvailable{};              /**< True while frames may be published. */
            mutable std::mutex mutex{};                    /**< Serializes every public stream operation. */
    };

    /** @brief Validates stream bounds and loopback-by-default exposure policy. */
    XWalkMjpegStreamStatus
    validateMjpegStreamConfiguration(const XWalkMjpegStreamConfiguration& configuration) noexcept;
    /** @brief Starts or idempotently reaffirms one empty stream. */
    XWalkMjpegStreamStatus startMjpegStream(XWalkMjpegStreamState& state,
                                            const XWalkMjpegStreamConfiguration& configuration) noexcept;
    /** @brief Clears clients and queued data through idempotent shutdown. */
    XWalkMjpegStreamStatus stopMjpegStream(XWalkMjpegStreamState& state) noexcept;
    /** @brief Adds one logical client without opening a network connection. */
    XWalkMjpegStreamStatus addMjpegStreamClient(XWalkMjpegStreamState& state, agent::uint32 clientIdentifier) noexcept;
    /** @brief Disconnects one client and releases all of its queued frames. */
    XWalkMjpegStreamStatus removeMjpegStreamClient(XWalkMjpegStreamState& state,
                                                   agent::uint32 clientIdentifier) noexcept;
    /** @brief Publishes one JPEG independently into every bounded client queue. */
    XWalkMjpegStreamStatus publishMjpegFrame(XWalkMjpegStreamState& state, const agent::bytevector& jpeg) noexcept;
    /** @brief Pops and encodes one multipart frame for a selected client. */
    XWalkMjpegStreamStatus popMjpegMultipartFrame(XWalkMjpegStreamState& state,
                                                  agent::uint32 clientIdentifier,
                                                  agent::bytevector& multipartFrame,
                                                  agent::uint64& sequence) noexcept;
    /** @brief Clears stale frames and prevents publication after camera loss. */
    XWalkMjpegStreamStatus reportMjpegCameraLoss(XWalkMjpegStreamState& state) noexcept;
    /** @brief Returns the HTTP response header required by an embedding transport. */
    agent::string mjpegHttpResponseHeader();

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_MJPEG_STREAM_H */
