/******************************************************************************
 * @file        xControllerScheduler.h
 * @brief       Declares the Linux child-process Controller scheduler.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Scheduler
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XCONTROLLER_SCHEDULER_H
#define XCONTROLLER_SCHEDULER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerSchedulerTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::ctrl
{

    /**
     * @class XWalkScheduler
     * @brief Owns fixed-capacity mailboxes, private IPC sockets, and Linux children.
     */
    class XWalkScheduler
    {
        private:
            xSchedulerMailbox mailboxes[XWALK_MAX_PROCESSES];
            ::ctrl::boolean shutdownInProgress;

        protected:
            static ::ctrl::int32
            runChild(::ctrl::int32 childSocket, ::ctrl::uint32 mailBoxId, const xModuleCallbacks* callbacks) noexcept;
            static ::ctrl::int32 sendPacket(::ctrl::int32 socketDescriptor, const xSchedulerPacket* packet) noexcept;
            static ::ctrl::int32 receivePacket(::ctrl::int32 socketDescriptor, xSchedulerPacket* packet) noexcept;
            static ::ctrl::boolean packetValid(const xSchedulerPacket* packet, ::ctrl::uint32 mailBoxId) noexcept;
            static void initializePacket(xSchedulerPacket* packet) noexcept;
            static void closeDescriptor(::ctrl::int32* descriptor) noexcept;
            xSchedulerMailbox* findMailbox(::ctrl::uint32 mailBoxId) noexcept;
            const xSchedulerMailbox* findMailbox(::ctrl::uint32 mailBoxId) const noexcept;
            xRequestStatus* findRequest(xSchedulerMailbox* mailbox, const xClientAddress* address) noexcept;
            const xRequestStatus* findRequest(const xSchedulerMailbox* mailbox,
                                              const xClientAddress* address) const noexcept;
            ::ctrl::int32 allocateRequestRecord(xSchedulerMailbox* mailbox, const xSchedulerCommand* command) noexcept;
            ::ctrl::int32 allocateLocalIndex(xSchedulerMailbox* mailbox, ::ctrl::uint32* localIndex) const noexcept;
            ::ctrl::int32 dispatchNext(xSchedulerMailbox* mailbox) noexcept;
            ::ctrl::int32
            processPacket(xSchedulerMailbox* mailbox, const xSchedulerPacket* packet, xProcessStatus* status) noexcept;
            void resolveRequests(xSchedulerMailbox* mailbox, xRequestState state, ::ctrl::int32 result) noexcept;
            void
            updateLastStatus(xSchedulerMailbox* mailbox, const xRequestStatus* request, ::ctrl::int32 result) noexcept;
            void reapChildren() noexcept;
            void handleChildExit(xSchedulerMailbox* mailbox, ::ctrl::int32 childStatus) noexcept;
            ::ctrl::int32 sendLifecycleCommand(xSchedulerMailbox* mailbox, xCommandType commandType) noexcept;
            ::ctrl::boolean allChildrenReaped() const noexcept;
            void waitForChildren(::ctrl::int32 timeoutMilliseconds) noexcept;
            void clearFinalizedRecords() noexcept;

        public:
            XWalkScheduler() noexcept;
            ~XWalkScheduler() noexcept;
            XWalkScheduler(const XWalkScheduler&) = delete;
            XWalkScheduler& operator=(const XWalkScheduler&) = delete;

            ::ctrl::int32 addModule(::ctrl::uint32 mailBoxId,
                                    ::ctrl::cstring clientAddress,
                                    ::ctrl::uint32 moduleType,
                                    const xModuleCallbacks* callbacks) noexcept;
            ::ctrl::int32 startModule(::ctrl::uint32 mailBoxId) noexcept;
            ::ctrl::int32 stopModule(::ctrl::uint32 mailBoxId) noexcept;
            ::ctrl::int32 shutdownModule(::ctrl::uint32 mailBoxId) noexcept;
            ::ctrl::int32 submitCommand(::ctrl::uint32 mailBoxId,
                                        ::ctrl::uint32 commandId,
                                        ::ctrl::cstring commandData,
                                        xClientAddress* requestAddress) noexcept;
            ::ctrl::int32 submitSignal(::ctrl::uint32 mailBoxId,
                                       ::ctrl::uint32 signalNumber,
                                       const ::ctrl::uint8* payload,
                                       ::ctrl::size payloadSize,
                                       const xClientAddress* clientAddress,
                                       xClientAddress* requestAddress) noexcept;
            ::ctrl::int32 requestStatus(::ctrl::uint32 mailBoxId) noexcept;
            ::ctrl::int32 pollStatus(xProcessStatus* statusList,
                                     ::ctrl::size statusListSize,
                                     ::ctrl::int32 timeoutMilliseconds) noexcept;
            ::ctrl::int32 getRequestStatus(const xClientAddress* requestAddress,
                                           xRequestStatus* requestStatus) const noexcept;
            ::ctrl::int32 getProcessStatusMasks(xProcessStatusMasks* statusMasks) const noexcept;
            ::ctrl::uint64 getProcessMask(xProcessState state) const noexcept;
            ::ctrl::uint32 getProcessCount(::ctrl::uint64 processMask) const noexcept;
            ::ctrl::int32 getProcessesByMask(::ctrl::uint64 processMask,
                                             xProcessStatus* statusList,
                                             ::ctrl::size statusListSize) const noexcept;
            void shutdownAll() noexcept;
    };

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_SCHEDULER_H */
