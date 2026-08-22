/******************************************************************************
 * @file        xControllerSchedulerTest.cpp
 * @brief       Verifies Controller scheduler process, FIFO, mask, and cleanup behavior.
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
#include <gtest/gtest.h>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::ctrl::test::scheduler
{

    TEST(XWalkSchedulerTest, WraparoundSkipsZeroAndCollidingRetainedIndexes)
    {
        XWalkSchedulerTestAccess scheduler;
        xSchedulerMailbox mailbox{};
        mailbox.mailBoxId = 1001U;
        mailbox.nextLocalIndex = UINT32_MAX;
        static_cast<void>(std::strncpy(mailbox.clientAddress, "camera", XWALK_CLIENT_ADDRESS_SIZE - 1U));
        mailbox.requestRecords[0U].address.mailBoxId = mailbox.mailBoxId;
        mailbox.requestRecords[0U].address.xWalkLocalIndex = 1U;
        static_cast<void>(std::strncpy(
            mailbox.requestRecords[0U].address.clientAddress, mailbox.clientAddress, XWALK_CLIENT_ADDRESS_SIZE - 1U));
        mailbox.requestRecords[0U].state = XWALK_REQUEST_PENDING;

        ::ctrl::uint32 localIndex{0U};
        ASSERT_EQ(scheduler.allocateLocalIndexForTest(&mailbox, &localIndex), XWALK_SCHEDULER_OK);
        EXPECT_EQ(localIndex, UINT32_MAX);
        ASSERT_EQ(scheduler.allocateLocalIndexForTest(&mailbox, &localIndex), XWALK_SCHEDULER_OK);
        EXPECT_EQ(localIndex, 2U);
        EXPECT_NE(localIndex, 0U);
    }

    TEST(XWalkSchedulerTest, MismatchedAndDuplicateCompletionCannotAdvanceAnActiveRequest)
    {
        XWalkSchedulerTestAccess scheduler;
        xSchedulerMailbox mailbox{};
        mailbox.registered = true;
        mailbox.mailBoxId = 1001U;
        mailbox.processState = XWALK_PROCESS_RUNNING;
        mailbox.requestActive = true;
        mailbox.activeRequest.address.mailBoxId = mailbox.mailBoxId;
        mailbox.activeRequest.address.xWalkLocalIndex = 7U;
        mailbox.activeRequest.commandId = 55U;
        static_cast<void>(
            std::strncpy(mailbox.activeRequest.address.clientAddress, "camera", XWALK_CLIENT_ADDRESS_SIZE - 1U));
        mailbox.requestRecords[0U].address = mailbox.activeRequest.address;
        mailbox.requestRecords[0U].commandId = mailbox.activeRequest.commandId;
        mailbox.requestRecords[0U].state = XWALK_REQUEST_RUNNING;

        xSchedulerPacket packet{};
        XWalkSchedulerTestAccess::initializePacketForTest(&packet);
        packet.packetType = XWALK_SCHEDULER_PACKET_REQUEST_STATUS;
        packet.commandType = XWALK_COMMAND_CUSTOM;
        packet.address = mailbox.activeRequest.address;
        packet.address.xWalkLocalIndex = 8U;
        packet.commandId = mailbox.activeRequest.commandId;
        packet.processState = XWALK_PROCESS_RUNNING;
        packet.requestState = XWALK_REQUEST_COMPLETED;
        xProcessStatus status{};
        EXPECT_EQ(scheduler.processPacketForTest(&mailbox, &packet, &status), XWALK_SCHEDULER_NOT_FOUND);
        EXPECT_TRUE(mailbox.requestActive);
        EXPECT_EQ(mailbox.requestRecords[0U].state, XWALK_REQUEST_RUNNING);

        packet.address = mailbox.activeRequest.address;
        mailbox.requestRecords[0U].state = XWALK_REQUEST_COMPLETED;
        EXPECT_EQ(scheduler.processPacketForTest(&mailbox, &packet, &status), XWALK_SCHEDULER_INVALID_STATE);
        EXPECT_TRUE(mailbox.requestActive);
    }

    TEST(XWalkSchedulerTest, RegistersDistinctChildrenAndMaintainsLifecycleMasks)
    {
        ::ctrl::int32 eventPipe[2U]{-1, -1};
        ASSERT_EQ(::pipe2(eventPipe, O_CLOEXEC), 0);
        SchedulerTestContext firstContext{eventPipe[1U], 0U, 0};
        SchedulerTestContext secondContext{eventPipe[1U], 0U, 0};
        const xModuleCallbacks firstCallbacks = makeCallbacks(&firstContext);
        const xModuleCallbacks secondCallbacks = makeCallbacks(&secondContext);
        XWalkScheduler scheduler;

        ASSERT_EQ(scheduler.addModule(1001U, "camera", 1U, &firstCallbacks), XWALK_SCHEDULER_OK);
        EXPECT_EQ(scheduler.addModule(1001U, "camera", 1U, &firstCallbacks), XWALK_SCHEDULER_DUPLICATE_MAILBOX);
        ASSERT_EQ(scheduler.addModule(2001U, "controller", 2U, &secondCallbacks), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.startModule(1001U), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.startModule(2001U), XWALK_SCHEDULER_OK);

        xProcessStatus firstStatus{};
        xProcessStatus secondStatus{};
        ASSERT_EQ(waitForProcessState(&scheduler, 1001U, XWALK_PROCESS_RUNNING, 1000, &firstStatus), 0);
        const ::ctrl::int32 secondWaitResult =
            waitForProcessState(&scheduler, 2001U, XWALK_PROCESS_RUNNING, 1000, &secondStatus);
        if (secondWaitResult != 0)
        {
            xProcessStatus statuses[2U]{};
            ASSERT_EQ(scheduler.getProcessesByMask(scheduler.getProcessMask(XWALK_PROCESS_RUNNING), statuses, 2U), 2);
            secondStatus = statuses[statuses[0U].mailBoxId == 2001U ? 0U : 1U];
        }
        EXPECT_GT(firstStatus.processId, 0);
        EXPECT_GT(secondStatus.processId, 0);
        EXPECT_NE(firstStatus.processId, secondStatus.processId);

        xProcessStatusMasks masks{};
        ASSERT_EQ(scheduler.getProcessStatusMasks(&masks), XWALK_SCHEDULER_OK);
        EXPECT_EQ(masks.registeredCount, 2U);
        EXPECT_EQ(masks.aliveCount, 2U);
        EXPECT_EQ(masks.runningCount, 2U);
        EXPECT_EQ(masks.registeredMask, masks.runningMask);
        EXPECT_EQ(scheduler.getProcessCount(masks.runningMask), 2U);

        const pid_t firstProcessId = firstStatus.processId;
        const pid_t secondProcessId = secondStatus.processId;
        scheduler.shutdownAll();
        EXPECT_EQ(::waitpid(firstProcessId, nullptr, WNOHANG), -1);
        EXPECT_EQ(errno, ECHILD);
        EXPECT_EQ(::waitpid(secondProcessId, nullptr, WNOHANG), -1);
        EXPECT_EQ(errno, ECHILD);
        ASSERT_EQ(scheduler.getProcessStatusMasks(&masks), XWALK_SCHEDULER_OK);
        EXPECT_EQ(masks.aliveMask, 0U);
        EXPECT_EQ(masks.registeredMask, 0U);
        static_cast<void>(::close(eventPipe[0U]));
        static_cast<void>(::close(eventPipe[1U]));
    }

    TEST(XWalkSchedulerTest, SerializesSameMailboxInFifoOrderWithIncreasingLocalIndexes)
    {
        ::ctrl::int32 eventPipe[2U]{-1, -1};
        ASSERT_EQ(::pipe2(eventPipe, O_CLOEXEC), 0);
        SchedulerTestContext context{eventPipe[1U], 10U, 0};
        const xModuleCallbacks callbacks = makeCallbacks(&context);
        XWalkScheduler scheduler;
        ASSERT_EQ(scheduler.addModule(1001U, "camera", 1U, &callbacks), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.startModule(1001U), XWALK_SCHEDULER_OK);
        xProcessStatus processStatus{};
        ASSERT_EQ(waitForProcessState(&scheduler, 1001U, XWALK_PROCESS_RUNNING, 1000, &processStatus), 0);

        xClientAddress addresses[3U]{};
        ASSERT_EQ(scheduler.submitCommand(1001U, 11U, "one", &addresses[0U]), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.submitCommand(1001U, 12U, "two", &addresses[1U]), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.submitCommand(1001U, 13U, "three", &addresses[2U]), XWALK_SCHEDULER_OK);
        EXPECT_EQ(addresses[0U].xWalkLocalIndex, 1U);
        EXPECT_EQ(addresses[1U].xWalkLocalIndex, 2U);
        EXPECT_EQ(addresses[2U].xWalkLocalIndex, 3U);
        EXPECT_EQ(scheduler.getProcessCount(scheduler.getProcessMask(XWALK_PROCESS_RUNNING)), 1U);

        xRequestStatus requestStatus{};
        ASSERT_EQ(waitForRequest(&scheduler, &addresses[2U], 2000, &requestStatus), 0);
        EXPECT_EQ(requestStatus.state, XWALK_REQUEST_COMPLETED);
        for (::ctrl::uint32 commandOffset = 0U; commandOffset < 3U; ++commandOffset)
        {
            SchedulerTestEvent started{};
            SchedulerTestEvent completed{};
            ASSERT_EQ(readEvent(eventPipe[0U], &started, 1000), 0);
            ASSERT_EQ(readEvent(eventPipe[0U], &completed, 1000), 0);
            EXPECT_EQ(started.type, SCHEDULER_TEST_STARTED);
            EXPECT_EQ(completed.type, SCHEDULER_TEST_COMPLETED);
            EXPECT_EQ(started.commandId, 11U + commandOffset);
            EXPECT_EQ(completed.commandId, 11U + commandOffset);
            EXPECT_EQ(started.processId, processStatus.processId);
            EXPECT_EQ(completed.processId, processStatus.processId);
        }
        scheduler.shutdownAll();
        static_cast<void>(::close(eventPipe[0U]));
        static_cast<void>(::close(eventPipe[1U]));
    }

    TEST(XWalkSchedulerTest, RunsDifferentMailboxesInParallelAndIsolatesFailure)
    {
        ::ctrl::int32 eventPipe[2U]{-1, -1};
        ASSERT_EQ(::pipe2(eventPipe, O_CLOEXEC), 0);
        SchedulerTestContext slowFailure{eventPipe[1U], 150U, 7};
        SchedulerTestContext fastSuccess{eventPipe[1U], 150U, 0};
        const xModuleCallbacks failureCallbacks = makeCallbacks(&slowFailure);
        const xModuleCallbacks successCallbacks = makeCallbacks(&fastSuccess);
        XWalkScheduler scheduler;
        ASSERT_EQ(scheduler.addModule(1001U, "camera", 1U, &failureCallbacks), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.addModule(2001U, "controller", 2U, &successCallbacks), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.startModule(1001U), XWALK_SCHEDULER_OK);
        xProcessStatus processStatus{};
        ASSERT_EQ(waitForProcessState(&scheduler, 1001U, XWALK_PROCESS_RUNNING, 1000, &processStatus), 0);
        ASSERT_EQ(scheduler.startModule(2001U), XWALK_SCHEDULER_OK);
        ASSERT_EQ(waitForProcessState(&scheduler, 2001U, XWALK_PROCESS_RUNNING, 1000, &processStatus), 0);

        xClientAddress failureAddress{};
        xClientAddress successAddress{};
        ASSERT_EQ(scheduler.submitCommand(1001U, 21U, "slow", &failureAddress), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.submitCommand(2001U, 22U, "fast", &successAddress), XWALK_SCHEDULER_OK);
        EXPECT_EQ(failureAddress.xWalkLocalIndex, 1U);
        EXPECT_EQ(successAddress.xWalkLocalIndex, 1U);

        SchedulerTestEvent firstEvent{};
        SchedulerTestEvent secondEvent{};
        ASSERT_EQ(readEvent(eventPipe[0U], &firstEvent, 1000), 0);
        ASSERT_EQ(readEvent(eventPipe[0U], &secondEvent, 1000), 0);
        EXPECT_EQ(firstEvent.type, SCHEDULER_TEST_STARTED);
        EXPECT_EQ(secondEvent.type, SCHEDULER_TEST_STARTED);
        EXPECT_NE(firstEvent.processId, secondEvent.processId);

        xRequestStatus failureStatus{};
        xRequestStatus successStatus{};
        ASSERT_EQ(waitForRequest(&scheduler, &failureAddress, 2000, &failureStatus), 0);
        ASSERT_EQ(waitForRequest(&scheduler, &successAddress, 2000, &successStatus), 0);
        EXPECT_EQ(failureStatus.state, XWALK_REQUEST_FAILED);
        EXPECT_EQ(failureStatus.result, 7);
        EXPECT_EQ(successStatus.state, XWALK_REQUEST_COMPLETED);
        EXPECT_EQ(successStatus.result, 0);
        EXPECT_EQ(scheduler.getProcessCount(scheduler.getProcessMask(XWALK_PROCESS_RUNNING)), 2U);
        scheduler.shutdownAll();
        static_cast<void>(::close(eventPipe[0U]));
        static_cast<void>(::close(eventPipe[1U]));
    }

    TEST(XWalkSchedulerTest, StopPausesPendingDispatchAndStartResumesItOnce)
    {
        ::ctrl::int32 eventPipe[2U]{-1, -1};
        ASSERT_EQ(::pipe2(eventPipe, O_CLOEXEC), 0);
        SchedulerTestContext context{eventPipe[1U], 0U, 0};
        const xModuleCallbacks callbacks = makeCallbacks(&context);
        XWalkScheduler scheduler;
        ASSERT_EQ(scheduler.addModule(1001U, "camera", 1U, &callbacks), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.startModule(1001U), XWALK_SCHEDULER_OK);
        xProcessStatus processStatus{};
        ASSERT_EQ(waitForProcessState(&scheduler, 1001U, XWALK_PROCESS_RUNNING, 1000, &processStatus), 0);
        ASSERT_EQ(scheduler.stopModule(1001U), XWALK_SCHEDULER_OK);
        ASSERT_EQ(waitForProcessState(&scheduler, 1001U, XWALK_PROCESS_STOPPED, 1000, &processStatus), 0);
        SchedulerTestEvent stoppedEvent{};
        ASSERT_EQ(readEvent(eventPipe[0U], &stoppedEvent, 1000), 0);
        EXPECT_EQ(stoppedEvent.type, SCHEDULER_TEST_STOPPED);

        xClientAddress address{};
        ASSERT_EQ(scheduler.submitCommand(1001U, 31U, "paused", &address), XWALK_SCHEDULER_OK);
        SchedulerTestEvent event{};
        EXPECT_NE(readEvent(eventPipe[0U], &event, 100), 0);
        ASSERT_EQ(scheduler.startModule(1001U), XWALK_SCHEDULER_OK);
        ASSERT_EQ(waitForProcessState(&scheduler, 1001U, XWALK_PROCESS_RUNNING, 1000, &processStatus), 0);
        xRequestStatus requestStatus{};
        ASSERT_EQ(waitForRequest(&scheduler, &address, 1000, &requestStatus), 0);
        EXPECT_EQ(requestStatus.state, XWALK_REQUEST_COMPLETED);
        ASSERT_EQ(readEvent(eventPipe[0U], &event, 1000), 0);
        EXPECT_EQ(event.type, SCHEDULER_TEST_STARTED);
        ASSERT_EQ(readEvent(eventPipe[0U], &event, 1000), 0);
        EXPECT_EQ(event.type, SCHEDULER_TEST_COMPLETED);
        EXPECT_NE(readEvent(eventPipe[0U], &event, 100), 0);
        scheduler.shutdownAll();
        static_cast<void>(::close(eventPipe[0U]));
        static_cast<void>(::close(eventPipe[1U]));
    }

    TEST(XWalkSchedulerTest, ReportsQueueCapacityAndCancelsEveryPendingRequest)
    {
        ::ctrl::int32 eventPipe[2U]{-1, -1};
        ASSERT_EQ(::pipe2(eventPipe, O_CLOEXEC), 0);
        SchedulerTestContext context{eventPipe[1U], 0U, 0};
        const xModuleCallbacks callbacks = makeCallbacks(&context);
        XWalkScheduler scheduler;
        ASSERT_EQ(scheduler.addModule(1001U, "camera", 1U, &callbacks), XWALK_SCHEDULER_OK);
        xClientAddress addresses[XWALK_MAX_PENDING_COMMANDS]{};
        for (::ctrl::size index = 0U; index < XWALK_MAX_PENDING_COMMANDS; ++index)
        {
            ASSERT_EQ(
                scheduler.submitCommand(1001U, static_cast<::ctrl::uint32>(index + 1U), "pending", &addresses[index]),
                XWALK_SCHEDULER_OK);
        }
        xClientAddress overflowAddress{};
        EXPECT_EQ(scheduler.submitCommand(1001U, 99U, "overflow", &overflowAddress), XWALK_SCHEDULER_QUEUE_FULL);
        ASSERT_EQ(scheduler.shutdownModule(1001U), XWALK_SCHEDULER_OK);
        for (::ctrl::size index = 0U; index < XWALK_MAX_PENDING_COMMANDS; ++index)
        {
            xRequestStatus requestStatus{};
            ASSERT_EQ(scheduler.getRequestStatus(&addresses[index], &requestStatus), XWALK_SCHEDULER_OK);
            EXPECT_EQ(requestStatus.state, XWALK_REQUEST_CANCELLED);
        }
        scheduler.shutdownAll();
        static_cast<void>(::close(eventPipe[0U]));
        static_cast<void>(::close(eventPipe[1U]));
    }

    TEST(XWalkSchedulerTest, TerminatesAndReapsOnlyTheTrackedStuckChildAfterGracePeriod)
    {
        ::ctrl::int32 eventPipe[2U]{-1, -1};
        ASSERT_EQ(::pipe2(eventPipe, O_CLOEXEC), 0);
        SchedulerTestContext context{eventPipe[1U], 5000U, 0};
        const xModuleCallbacks callbacks = makeCallbacks(&context);
        XWalkScheduler scheduler;
        ASSERT_EQ(scheduler.addModule(1001U, "camera", 1U, &callbacks), XWALK_SCHEDULER_OK);
        ASSERT_EQ(scheduler.startModule(1001U), XWALK_SCHEDULER_OK);
        xProcessStatus processStatus{};
        ASSERT_EQ(waitForProcessState(&scheduler, 1001U, XWALK_PROCESS_RUNNING, 1000, &processStatus), 0);
        xClientAddress address{};
        ASSERT_EQ(scheduler.submitCommand(1001U, 41U, "stuck", &address), XWALK_SCHEDULER_OK);
        SchedulerTestEvent event{};
        ASSERT_EQ(readEvent(eventPipe[0U], &event, 1000), 0);
        EXPECT_EQ(event.type, SCHEDULER_TEST_STARTED);
        const pid_t trackedProcessId = processStatus.processId;

        scheduler.shutdownAll();
        EXPECT_EQ(::waitpid(trackedProcessId, nullptr, WNOHANG), -1);
        EXPECT_EQ(errno, ECHILD);
        xProcessStatusMasks masks{};
        ASSERT_EQ(scheduler.getProcessStatusMasks(&masks), XWALK_SCHEDULER_OK);
        EXPECT_EQ(masks.aliveMask, 0U);
        xRequestStatus requestStatus{};
        EXPECT_EQ(scheduler.getRequestStatus(&address, &requestStatus), XWALK_SCHEDULER_UNKNOWN_MAILBOX);
        static_cast<void>(::close(eventPipe[0U]));
        static_cast<void>(::close(eventPipe[1U]));
    }

} /* namespace xwalk::ctrl::test::scheduler */
