/******************************************************************************
 * @file        xControllerSchedulerTestSupport.cpp
 * @brief       Implements reusable host support for scheduler process tests.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Scheduler Test
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerSchedulerTestSupport.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include <cerrno>
#include <cstring>
#include <time.h>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::ctrl::test::scheduler
{

    ::ctrl::int32 XWalkSchedulerTestAccess::allocateLocalIndexForTest(xSchedulerMailbox* mailbox,
                                                                      ::ctrl::uint32* localIndex) const noexcept
    {
        return allocateLocalIndex(mailbox, localIndex);
    }

    ::ctrl::int32 XWalkSchedulerTestAccess::processPacketForTest(xSchedulerMailbox* mailbox,
                                                                 const xSchedulerPacket* packet,
                                                                 xProcessStatus* status) noexcept
    {
        return processPacket(mailbox, packet, status);
    }

    void XWalkSchedulerTestAccess::initializePacketForTest(xSchedulerPacket* packet) noexcept
    {
        initializePacket(packet);
    }

    /** @brief Writes one fixed-size callback event to the inherited test pipe. */
    static ::ctrl::int32
    writeEvent(SchedulerTestContext* context, SchedulerTestEventType type, ::ctrl::uint32 commandId) noexcept
    {
        if ((context == nullptr) || (context->eventDescriptor < 0))
        {
            return -1;
        }
        struct timespec timestamp
        {
        };
        static_cast<void>(::clock_gettime(CLOCK_MONOTONIC, &timestamp));
        const ::ctrl::uint64 seconds = static_cast<::ctrl::uint64>(timestamp.tv_sec);
        const ::ctrl::uint64 nanoseconds = static_cast<::ctrl::uint64>(timestamp.tv_nsec);
        SchedulerTestEvent event{};
        std::memset(&event, 0, sizeof(event));
        event.type = type;
        event.commandId = commandId;
        event.processId = ::getpid();
        event.monotonicMicroseconds = (seconds * 1'000'000U) + (nanoseconds / 1'000U);
        const ssize_t bytesWritten = ::write(context->eventDescriptor, &event, sizeof(event));
        return bytesWritten == static_cast<ssize_t>(sizeof(event)) ? 0 : -1;
    }

    /** @brief Accepts one child Start request. */
    static ::ctrl::int32 startCallback(::ctrl::contextpointer context) noexcept
    {
        static_cast<void>(context);
        return 0;
    }

    /** @brief Records one child Stop request. */
    static ::ctrl::int32 stopCallback(::ctrl::contextpointer context) noexcept
    {
        auto* testContext = static_cast<SchedulerTestContext*>(context);
        return writeEvent(testContext, SCHEDULER_TEST_STOPPED, 0U);
    }

    /** @brief Records callback start and completion around one bounded delay. */
    static ::ctrl::int32
    commandCallback(::ctrl::uint32 commandId, ::ctrl::cstring commandData, ::ctrl::contextpointer context) noexcept
    {
        static_cast<void>(commandData);
        auto* testContext = static_cast<SchedulerTestContext*>(context);
        const ::ctrl::int32 startWriteResult = writeEvent(testContext, SCHEDULER_TEST_STARTED, commandId);
        if (startWriteResult != 0)
        {
            return -1;
        }
        static_cast<void>(::usleep(testContext->delayMilliseconds * 1'000U));
        const ::ctrl::int32 completionWriteResult = writeEvent(testContext, SCHEDULER_TEST_COMPLETED, commandId);
        if (completionWriteResult != 0)
        {
            return -1;
        }
        return testContext->commandResult;
    }

    /** @brief Records one complete native signal callback in the child. */
    static ::ctrl::int32 signalCallback(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept
    {
        auto* testContext = static_cast<SchedulerTestContext*>(context);
        const ::ctrl::int32 writeResult =
            signal == nullptr ? -1 : writeEvent(testContext, SCHEDULER_TEST_STARTED, signal->sigNo);
        if ((signal == nullptr) || (writeResult != 0))
        {
            return -1;
        }
        return testContext->commandResult;
    }

    xModuleCallbacks makeCallbacks(SchedulerTestContext* context) noexcept
    {
        return {&startCallback, &stopCallback, &commandCallback, nullptr, context, nullptr};
    }

    xModuleCallbacks makeSignalCallbacks(SchedulerTestContext* context) noexcept
    {
        return {&startCallback, &stopCallback, nullptr, nullptr, context, &signalCallback};
    }

    ::ctrl::int32 waitForProcessState(XWalkScheduler* scheduler,
                                      ::ctrl::uint32 mailBoxId,
                                      xProcessState expectedState,
                                      ::ctrl::int32 timeoutMilliseconds,
                                      xProcessStatus* observedStatus) noexcept
    {
        if ((scheduler == nullptr) || (observedStatus == nullptr))
        {
            return -1;
        }
        ::ctrl::int32 elapsed{0};
        xProcessStatus events[XWALK_MAX_STATUS_EVENTS]{};
        while (elapsed < timeoutMilliseconds)
        {
            const ::ctrl::int32 eventCount =
                scheduler->pollStatus(events, XWALK_MAX_STATUS_EVENTS, XWALK_SCHEDULER_POLL_SLICE_MS);
            if (eventCount < 0)
            {
                return eventCount;
            }
            for (::ctrl::int32 eventIndex = 0; eventIndex < eventCount; ++eventIndex)
            {
                if ((events[eventIndex].mailBoxId == mailBoxId) && (events[eventIndex].state == expectedState))
                {
                    *observedStatus = events[eventIndex];
                    return 0;
                }
            }
            elapsed += XWALK_SCHEDULER_POLL_SLICE_MS;
        }
        return -1;
    }

    ::ctrl::int32 waitForRequest(XWalkScheduler* scheduler,
                                 const xClientAddress* address,
                                 ::ctrl::int32 timeoutMilliseconds,
                                 xRequestStatus* requestStatus) noexcept
    {
        if ((scheduler == nullptr) || (address == nullptr) || (requestStatus == nullptr))
        {
            return -1;
        }
        ::ctrl::int32 elapsed{0};
        xProcessStatus events[XWALK_MAX_STATUS_EVENTS]{};
        while (elapsed < timeoutMilliseconds)
        {
            const ::ctrl::int32 eventCount =
                scheduler->pollStatus(events, XWALK_MAX_STATUS_EVENTS, XWALK_SCHEDULER_POLL_SLICE_MS);
            if (eventCount < 0)
            {
                return eventCount;
            }
            const ::ctrl::int32 requestStatusResult = scheduler->getRequestStatus(address, requestStatus);
            if (requestStatusResult == XWALK_SCHEDULER_OK)
            {
                if ((requestStatus->state == XWALK_REQUEST_COMPLETED) ||
                    (requestStatus->state == XWALK_REQUEST_FAILED) || (requestStatus->state == XWALK_REQUEST_CANCELLED))
                {
                    return 0;
                }
            }
            elapsed += XWALK_SCHEDULER_POLL_SLICE_MS;
        }
        return -1;
    }

    ::ctrl::int32
    readEvent(::ctrl::int32 descriptor, SchedulerTestEvent* event, ::ctrl::int32 timeoutMilliseconds) noexcept
    {
        if ((descriptor < 0) || (event == nullptr))
        {
            return -1;
        }
        struct pollfd pollDescriptor
        {
        };
        pollDescriptor.fd = descriptor;
        pollDescriptor.events = POLLIN;
        const ::ctrl::int32 pollResult = ::poll(&pollDescriptor, 1U, timeoutMilliseconds);
        if ((pollResult <= 0) || ((pollDescriptor.revents & POLLIN) == 0))
        {
            return -1;
        }
        const ssize_t bytesRead = ::read(descriptor, event, sizeof(*event));
        return bytesRead == static_cast<ssize_t>(sizeof(*event)) ? 0 : -1;
    }

} /* namespace xwalk::ctrl::test::scheduler */
