/******************************************************************************
 * @file        xControllerSchedulerTestSupport.h
 * @brief       Declares reusable host support for scheduler process tests.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Scheduler Test
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#ifndef XCONTROLLER_SCHEDULER_TEST_SUPPORT_H
#define XCONTROLLER_SCHEDULER_TEST_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerScheduler.h"
#include "xControllerSchedulerSignal.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::ctrl::test::scheduler
{

    /** @brief Identifies a callback observation written atomically by a child. */
    typedef enum SchedulerTestEventType
    {
        SCHEDULER_TEST_STARTED = 1,
        SCHEDULER_TEST_COMPLETED,
        SCHEDULER_TEST_STOPPED
    } SchedulerTestEventType;

    /** @brief Carries one child callback observation through a test pipe. */
    typedef struct SchedulerTestEvent
    {
            SchedulerTestEventType type;
            ::ctrl::uint32 commandId;
            pid_t processId;
            ::ctrl::uint64 monotonicMicroseconds;
    } SchedulerTestEvent;

    /** @brief Configures one inherited child callback table. */
    typedef struct SchedulerTestContext
    {
            ::ctrl::int32 eventDescriptor;
            ::ctrl::uint32 delayMilliseconds;
            ::ctrl::int32 commandResult;
    } SchedulerTestContext;

    /** @brief Identifies one module observation in the native CBB chain test. */
    typedef struct NativeChainContext
    {
            ::ctrl::int32 eventDescriptor;
            xWalkModuleId moduleId;
    } NativeChainContext;

    /** @brief Exposes protected correlation helpers to deterministic unit tests. */
    class XWalkSchedulerTestAccess final : public XWalkScheduler
    {
        public:
            ::ctrl::int32 allocateLocalIndexForTest(xSchedulerMailbox* mailbox,
                                                    ::ctrl::uint32* localIndex) const noexcept;
            ::ctrl::int32 processPacketForTest(xSchedulerMailbox* mailbox,
                                               const xSchedulerPacket* packet,
                                               xProcessStatus* status) noexcept;
            static void initializePacketForTest(xSchedulerPacket* packet) noexcept;
    };

    xModuleCallbacks makeCallbacks(SchedulerTestContext* context) noexcept;
    xModuleCallbacks makeSignalCallbacks(SchedulerTestContext* context) noexcept;
    ::ctrl::int32 waitForProcessState(XWalkScheduler* scheduler,
                                      ::ctrl::uint32 mailBoxId,
                                      xProcessState expectedState,
                                      ::ctrl::int32 timeoutMilliseconds,
                                      xProcessStatus* observedStatus) noexcept;
    ::ctrl::int32 waitForRequest(XWalkScheduler* scheduler,
                                 const xClientAddress* address,
                                 ::ctrl::int32 timeoutMilliseconds,
                                 xRequestStatus* requestStatus) noexcept;
    ::ctrl::int32
    readEvent(::ctrl::int32 descriptor, SchedulerTestEvent* event, ::ctrl::int32 timeoutMilliseconds) noexcept;

} /* namespace xwalk::ctrl::test::scheduler */

#endif /* XCONTROLLER_SCHEDULER_TEST_SUPPORT_H */
