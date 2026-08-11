/******************************************************************************
 * @file        xWalk_Rpi5CarFailureObservability.h
 * @brief       Declares bounded thread-safe failure event observability.
 * @project     xWalk Firmware
 * @module      xWalkLibraryCommon
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XWALK_RPI5CAR_FAILURE_OBSERVABILITY_H
#define XWALK_RPI5CAR_FAILURE_OBSERVABILITY_H

#include "xHal_Rpi5CarTypes.h"

#include <array>
#include <limits>
#include <mutex>

namespace xwalk
{

constexpr hal::size XWALK_FAILURE_EVENT_CAPACITY{256U};

/** @brief Stable project-wide safety and lifecycle event identifiers. */
enum class XWalkFailureEventId : hal::uint8
{
    WatchdogExpiry = 0U,
    CameraLoss,
    CameraFreeze,
    InferenceTimeout,
    DroppedFrame,
    QueueSaturation,
    SlowClientDisconnection,
    EmergencyStop,
    InvalidCommand,
    SensorFault,
    I2cFault,
    MotorWriteFailure,
    StaleCommandRejection,
    SafeStateTransition,
    StreamStart,
    StreamStop,
    ClientConnection,
    ClientDisconnection,
    Count
};

constexpr hal::size XWALK_FAILURE_EVENT_ID_COUNT =
    static_cast<hal::size>(XWalkFailureEventId::Count);

/** @brief Stores one non-sensitive ordered event record. */
struct XWalkFailureEvent
{
    hal::uint64 sequence{}; /**< Monotonic sequence beginning at one. */
    hal::uint64 timestamp{}; /**< Caller-provided logical or monotonic timestamp. */
    XWalkFailureEventId identifier{XWalkFailureEventId::WatchdogExpiry};
    hal::uint64 value{}; /**< Non-sensitive event-specific value. */
};

/** @brief Stores the fixed event ring and saturating counters. */
struct XWalkFailureObservability
{
    std::array<XWalkFailureEvent, XWALK_FAILURE_EVENT_CAPACITY> events{};
    std::array<hal::uint64, XWALK_FAILURE_EVENT_ID_COUNT> counters{};
    hal::size start{};
    hal::size count{};
    hal::uint64 nextSequence{1U};
    mutable std::mutex mutex{};
};

/** @brief Stores one stable ordered copy suitable for status reporting. */
struct XWalkFailureSnapshot
{
    std::array<XWalkFailureEvent, XWALK_FAILURE_EVENT_CAPACITY> events{};
    std::array<hal::uint64, XWALK_FAILURE_EVENT_ID_COUNT> counters{};
    hal::size eventCount{};
    hal::uint64 nextSequence{1U};
};

/** @brief Records one event with O(1) overwrite-oldest behavior. */
inline void recordFailureEvent(XWalkFailureObservability& observability,
    XWalkFailureEventId identifier, hal::uint64 timestamp,
    hal::uint64 value = 0U) noexcept
{
    const hal::size identifierIndex = static_cast<hal::size>(identifier);
    if (identifierIndex >= XWALK_FAILURE_EVENT_ID_COUNT)
    {
        return;
    }
    const std::lock_guard<std::mutex> lock(observability.mutex);
    hal::uint64& counter = observability.counters[identifierIndex];
    if (counter != std::numeric_limits<hal::uint64>::max())
    {
        ++counter;
    }
    hal::size index{};
    if (observability.count < observability.events.size())
    {
        index = (observability.start + observability.count) % observability.events.size();
        ++observability.count;
    }
    else
    {
        index = observability.start;
        observability.start = (observability.start + 1U) % observability.events.size();
    }
    observability.events[index] = {observability.nextSequence, timestamp,
        identifier, value};
    if (observability.nextSequence != std::numeric_limits<hal::uint64>::max())
    {
        ++observability.nextSequence;
    }
}

/** @brief Copies counters and retained events in oldest-to-newest order. */
inline XWalkFailureSnapshot failureSnapshot(
    const XWalkFailureObservability& observability) noexcept
{
    const std::lock_guard<std::mutex> lock(observability.mutex);
    XWalkFailureSnapshot snapshot;
    snapshot.counters = observability.counters;
    snapshot.eventCount = observability.count;
    snapshot.nextSequence = observability.nextSequence;
    for (hal::size index = 0U; index < observability.count; ++index)
    {
        snapshot.events[index] = observability.events[
            (observability.start + index) % observability.events.size()];
    }
    return snapshot;
}

/** @brief Resets one lifecycle after all producers have been stopped. */
inline void resetFailureObservability(XWalkFailureObservability& observability) noexcept
{
    const std::lock_guard<std::mutex> lock(observability.mutex);
    observability.events = {};
    observability.counters = {};
    observability.start = 0U;
    observability.count = 0U;
    observability.nextSequence = 1U;
}

} /* namespace xwalk */

#endif /* XWALK_RPI5CAR_FAILURE_OBSERVABILITY_H */
