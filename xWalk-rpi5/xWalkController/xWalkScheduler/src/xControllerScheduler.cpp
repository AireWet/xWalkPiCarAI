/******************************************************************************
 * @file        xControllerScheduler.cpp
 * @brief       Implements the Linux child-process Controller scheduler.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerScheduler.h"
#include "xControllerSchedulerSignal.h"

#include "xHal_Rpi5CarLinuxHeaders.h"
#include "xHal_Rpi5CarTrace.h"

#include <cerrno>
#include <climits>
#include <cstring>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::ctrl
{

    /**************************************************************************
     * Public member function definitions
     **************************************************************************/

    XWalkScheduler::XWalkScheduler() noexcept : mailboxes{}, shutdownInProgress(false)
    {
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            mailboxes[slot].processSlot = static_cast<::ctrl::uint32>(slot);
            mailboxes[slot].parentSocket = -1;
            mailboxes[slot].processState = XWALK_PROCESS_UNUSED;
        }
        XWALK_CTRL_TRACE_UID0(CTRL .090, "Controller process scheduler initialized");
    }

    XWalkScheduler::~XWalkScheduler() noexcept
    {
        shutdownAll();
    }

    ::ctrl::int32 XWalkScheduler::addModule(::ctrl::uint32 mailBoxId,
                                            ::ctrl::cstring clientAddress,
                                            ::ctrl::uint32 moduleType,
                                            const xModuleCallbacks* callbacks) noexcept
    {
        const ::ctrl::boolean registrationInvalid =
            (mailBoxId == 0U) || (clientAddress == nullptr) || (clientAddress[0] == '\0') ||
            (::strnlen(clientAddress, XWALK_CLIENT_ADDRESS_SIZE) >= XWALK_CLIENT_ADDRESS_SIZE) ||
            (callbacks == nullptr) || ((callbacks->onCommand == nullptr) && (callbacks->onSignal == nullptr));
        if (registrationInvalid)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler module registration arguments are invalid");
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        if (shutdownInProgress)
        {
            return XWALK_SCHEDULER_SHUTTING_DOWN;
        }
        const xSchedulerMailbox* existingMailbox = findMailbox(mailBoxId);
        if (existingMailbox != nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Duplicate scheduler mailbox %u", mailBoxId);
            return XWALK_SCHEDULER_DUPLICATE_MAILBOX;
        }

        xSchedulerMailbox* mailbox = nullptr;
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            if (mailboxes[slot].registered == false)
            {
                mailbox = &mailboxes[slot];
                break;
            }
        }
        if (mailbox == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler has no free process slot for mailbox %u", mailBoxId);
            return XWALK_SCHEDULER_NO_PROCESS_SLOT;
        }

        ::ctrl::int32 sockets[2U]{-1, -1};
        const ::ctrl::int32 socketPairResult = ::socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sockets);
        if (socketPairResult != 0)
        {
            XWALK_CTRL_ERROR(
                XWALK_EXCEPTION, "Scheduler socketpair failed for mailbox %u: %s", mailBoxId, std::strerror(errno));
            return XWALK_SCHEDULER_IPC_FAILURE;
        }

        const pid_t childProcessId = ::fork();
        if (childProcessId < 0)
        {
            XWALK_CTRL_ERROR(
                XWALK_EXCEPTION, "Scheduler fork failed for mailbox %u: %s", mailBoxId, std::strerror(errno));
            closeDescriptor(&sockets[0U]);
            closeDescriptor(&sockets[1U]);
            return XWALK_SCHEDULER_PROCESS_FAILURE;
        }
        if (childProcessId == 0)
        {
            cxx_xWalkUnbindScheduler_LPP(this);
            closeDescriptor(&sockets[0U]);
            for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
            {
                closeDescriptor(&mailboxes[slot].parentSocket);
            }
            const ::ctrl::int32 childResult = runChild(sockets[1U], mailBoxId, callbacks);
            closeDescriptor(&sockets[1U]);
            ::_exit(childResult == XWALK_SCHEDULER_OK ? EXIT_SUCCESS : EXIT_FAILURE);
        }

        closeDescriptor(&sockets[1U]);
        const ::ctrl::uint32 stableSlot = mailbox->processSlot;
        std::memset(mailbox, 0, sizeof(*mailbox));
        mailbox->registered = true;
        mailbox->processSlot = stableSlot;
        mailbox->mailBoxId = mailBoxId;
        static_cast<void>(std::memcpy(mailbox->clientAddress, clientAddress, std::strlen(clientAddress)));
        mailbox->moduleType = moduleType;
        mailbox->processId = childProcessId;
        mailbox->parentSocket = sockets[0U];
        mailbox->nextLocalIndex = 1U;
        mailbox->processState = XWALK_PROCESS_CREATED;
        mailbox->dispatchEnabled = false;
        mailbox->callbacks = *callbacks;
        updateLastStatus(mailbox, nullptr, XWALK_SCHEDULER_OK);

        XWALK_CTRL_TRACE_UID4(CTRL .091,
                              "Scheduler registered mailbox %u slot %u pid %ld module %u",
                              mailBoxId,
                              stableSlot,
                              static_cast<long>(childProcessId),
                              moduleType);
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 XWalkScheduler::startModule(::ctrl::uint32 mailBoxId) noexcept
    {
        xSchedulerMailbox* mailbox = findMailbox(mailBoxId);
        if (mailbox == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler start requested unknown mailbox %u", mailBoxId);
            return XWALK_SCHEDULER_UNKNOWN_MAILBOX;
        }
        if (mailbox->shutdownRequested || (mailbox->processState == XWALK_PROCESS_EXITED) ||
            (mailbox->processState == XWALK_PROCESS_FAILED))
        {
            return XWALK_SCHEDULER_INVALID_STATE;
        }
        if ((mailbox->processState == XWALK_PROCESS_RUNNING) || (mailbox->processState == XWALK_PROCESS_STARTING))
        {
            XWALK_CTRL_WARNING(XWALK_LOGIC, "Scheduler mailbox %u is already running or starting", mailBoxId);
            return XWALK_SCHEDULER_OK;
        }

        mailbox->processState = XWALK_PROCESS_STARTING;
        mailbox->dispatchEnabled = false;
        updateLastStatus(mailbox, nullptr, XWALK_SCHEDULER_OK);
        return sendLifecycleCommand(mailbox, XWALK_COMMAND_START);
    }

    ::ctrl::int32 XWalkScheduler::stopModule(::ctrl::uint32 mailBoxId) noexcept
    {
        xSchedulerMailbox* mailbox = findMailbox(mailBoxId);
        if (mailbox == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler stop requested unknown mailbox %u", mailBoxId);
            return XWALK_SCHEDULER_UNKNOWN_MAILBOX;
        }
        if ((mailbox->processState == XWALK_PROCESS_STOPPED) || (mailbox->processState == XWALK_PROCESS_STOPPING))
        {
            XWALK_CTRL_WARNING(XWALK_LOGIC, "Scheduler mailbox %u is already stopped or stopping", mailBoxId);
            return XWALK_SCHEDULER_OK;
        }
        if ((mailbox->processState != XWALK_PROCESS_RUNNING) && (mailbox->processState != XWALK_PROCESS_STARTING))
        {
            return XWALK_SCHEDULER_INVALID_STATE;
        }

        mailbox->dispatchEnabled = false;
        mailbox->processState = XWALK_PROCESS_STOPPING;
        updateLastStatus(mailbox, nullptr, XWALK_SCHEDULER_OK);
        return sendLifecycleCommand(mailbox, XWALK_COMMAND_STOP);
    }

    ::ctrl::int32 XWalkScheduler::shutdownModule(::ctrl::uint32 mailBoxId) noexcept
    {
        xSchedulerMailbox* mailbox = findMailbox(mailBoxId);
        if (mailbox == nullptr)
        {
            return XWALK_SCHEDULER_UNKNOWN_MAILBOX;
        }
        if (mailbox->shutdownRequested)
        {
            XWALK_CTRL_WARNING(XWALK_LOGIC, "Scheduler mailbox %u shutdown was already requested", mailBoxId);
            return XWALK_SCHEDULER_OK;
        }

        mailbox->shutdownRequested = true;
        mailbox->dispatchEnabled = false;
        resolveRequests(mailbox, XWALK_REQUEST_CANCELLED, XWALK_SCHEDULER_SHUTTING_DOWN);
        if ((mailbox->processId <= 0) || (mailbox->parentSocket < 0))
        {
            return XWALK_SCHEDULER_OK;
        }
        return sendLifecycleCommand(mailbox, XWALK_COMMAND_SHUTDOWN);
    }

    ::ctrl::int32 XWalkScheduler::submitCommand(::ctrl::uint32 mailBoxId,
                                                ::ctrl::uint32 commandId,
                                                ::ctrl::cstring commandData,
                                                xClientAddress* requestAddress) noexcept
    {
        const ::ctrl::boolean commandInvalid =
            (commandId == 0U) || (commandData == nullptr) || (requestAddress == nullptr) ||
            (::strnlen(commandData, XWALK_SCHEDULER_COMMAND_DATA_SIZE) >= XWALK_SCHEDULER_COMMAND_DATA_SIZE);
        if (commandInvalid)
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        xSchedulerMailbox* mailbox = findMailbox(mailBoxId);
        if (mailbox == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler command requested unknown mailbox %u", mailBoxId);
            return XWALK_SCHEDULER_UNKNOWN_MAILBOX;
        }
        if (shutdownInProgress || mailbox->shutdownRequested)
        {
            return XWALK_SCHEDULER_SHUTTING_DOWN;
        }
        if ((mailbox->processState == XWALK_PROCESS_EXITED) || (mailbox->processState == XWALK_PROCESS_FAILED))
        {
            return XWALK_SCHEDULER_INVALID_STATE;
        }
        if (mailbox->queueCount >= XWALK_MAX_PENDING_COMMANDS)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler queue full for mailbox %u", mailBoxId);
            return XWALK_SCHEDULER_QUEUE_FULL;
        }

        ::ctrl::uint32 localIndex{0U};
        const ::ctrl::int32 indexResult = allocateLocalIndex(mailbox, &localIndex);
        if (indexResult != XWALK_SCHEDULER_OK)
        {
            return indexResult;
        }

        xSchedulerCommand command{};
        command.address.mailBoxId = mailbox->mailBoxId;
        static_cast<void>(
            std::memcpy(command.address.clientAddress, mailbox->clientAddress, sizeof(command.address.clientAddress)));
        command.address.xWalkLocalIndex = localIndex;
        command.address.moduleType = mailbox->moduleType;
        command.commandId = commandId;
        command.dataType = XWALK_SCHEDULER_DATA_TEXT;
        command.dataSize = std::strlen(commandData);
        static_cast<void>(std::memcpy(command.commandData, commandData, command.dataSize));

        const ::ctrl::int32 recordResult = allocateRequestRecord(mailbox, &command);
        if (recordResult != XWALK_SCHEDULER_OK)
        {
            return recordResult;
        }
        mailbox->pendingQueue[mailbox->queueTail] = command;
        mailbox->queueTail = (mailbox->queueTail + 1U) % XWALK_MAX_PENDING_COMMANDS;
        ++mailbox->queueCount;
        *requestAddress = command.address;

        XWALK_CTRL_TRACE_UID4(CTRL .092,
                              "Scheduler queued mailbox %u local %u head %zu count %zu",
                              mailBoxId,
                              localIndex,
                              mailbox->queueHead,
                              mailbox->queueCount);
        if ((mailbox->processState == XWALK_PROCESS_RUNNING) && mailbox->dispatchEnabled &&
            (mailbox->requestActive == false))
        {
            return dispatchNext(mailbox);
        }
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 XWalkScheduler::submitSignal(::ctrl::uint32 mailBoxId,
                                               ::ctrl::uint32 signalNumber,
                                               const ::ctrl::uint8* payload,
                                               ::ctrl::size payloadSize,
                                               const xClientAddress* clientAddress,
                                               xClientAddress* requestAddress) noexcept
    {
        const ::ctrl::boolean signalInvalid = (signalNumber == 0U) || ((payload == nullptr) && (payloadSize != 0U)) ||
                                              (payloadSize > XWALK_SCHEDULER_COMMAND_DATA_SIZE) ||
                                              (cxx_xWalkIsRequestSignal_LPP(signalNumber) == 0) ||
                                              (cxx_xWalkSignalMatchesMailbox_LPP(mailBoxId, signalNumber) == 0);
        if (signalInvalid)
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        xSchedulerMailbox* mailbox = findMailbox(mailBoxId);
        if (mailbox == nullptr)
        {
            return XWALK_SCHEDULER_UNKNOWN_MAILBOX;
        }
        if (shutdownInProgress || mailbox->shutdownRequested)
        {
            return XWALK_SCHEDULER_SHUTTING_DOWN;
        }
        if ((mailbox->callbacks.onSignal == nullptr) || (mailbox->processState == XWALK_PROCESS_EXITED) ||
            (mailbox->processState == XWALK_PROCESS_FAILED))
        {
            return XWALK_SCHEDULER_INVALID_STATE;
        }
        if (mailbox->queueCount >= XWALK_MAX_PENDING_COMMANDS)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler signal queue full for mailbox %u", mailBoxId);
            return XWALK_SCHEDULER_QUEUE_FULL;
        }

        ::ctrl::uint32 localIndex{0U};
        const ::ctrl::int32 indexResult = allocateLocalIndex(mailbox, &localIndex);
        if (indexResult != XWALK_SCHEDULER_OK)
        {
            return indexResult;
        }

        xSchedulerCommand command{};
        command.address.mailBoxId = mailBoxId;
        const char* sourceAddress = mailbox->clientAddress;
        command.address.moduleType = mailbox->moduleType;
        if (clientAddress != nullptr)
        {
            const ::ctrl::boolean clientAddressInvalid =
                (clientAddress->mailBoxId != mailBoxId) || (clientAddress->xWalkLocalIndex != 0U) ||
                (clientAddress->clientAddress[0] == '\0') ||
                (::strnlen(clientAddress->clientAddress, XWALK_CLIENT_ADDRESS_SIZE) >= XWALK_CLIENT_ADDRESS_SIZE);
            if (clientAddressInvalid)
            {
                return XWALK_SCHEDULER_INVALID_ARGUMENT;
            }
            sourceAddress = clientAddress->clientAddress;
            command.address.moduleType = clientAddress->moduleType;
        }
        static_cast<void>(std::memcpy(
            command.address.clientAddress, sourceAddress, ::strnlen(sourceAddress, XWALK_CLIENT_ADDRESS_SIZE)));
        command.address.xWalkLocalIndex = localIndex;
        command.commandId = signalNumber;
        command.dataType = XWALK_SCHEDULER_DATA_SIGNAL;

        XWalkSignal signal{};
        signal.sigNo = signalNumber;
        signal.destination = mailBoxId == XWALK_CTRL_MAILBOX_ID    ? XWALK_MODULE_CTRL
                             : mailBoxId == XWALK_AGENT_MAILBOX_ID ? XWALK_MODULE_AGENT
                                                                   : XWALK_MODULE_HAL;
        if (payloadSize != 0U)
        {
            static_cast<void>(std::memcpy(signal.payload, payload, payloadSize));
        }
        signal.payloadSize = payloadSize;
        signal.clientInfo = command.address;
        command.dataSize = sizeof(signal);
        static_cast<void>(std::memcpy(command.commandData, &signal, sizeof(signal)));

        const ::ctrl::int32 recordResult = allocateRequestRecord(mailbox, &command);
        if (recordResult != XWALK_SCHEDULER_OK)
        {
            return recordResult;
        }
        mailbox->pendingQueue[mailbox->queueTail] = command;
        mailbox->queueTail = (mailbox->queueTail + 1U) % XWALK_MAX_PENDING_COMMANDS;
        ++mailbox->queueCount;
        if (requestAddress != nullptr)
        {
            *requestAddress = command.address;
        }
        XWALK_CTRL_TRACE_UID4(CTRL .102,
                              "Scheduler queued signal %u mailbox %u local %u bytes %zu",
                              signalNumber,
                              mailBoxId,
                              localIndex,
                              sizeof(signal));
        if ((mailbox->processState == XWALK_PROCESS_RUNNING) && mailbox->dispatchEnabled &&
            (mailbox->requestActive == false))
        {
            return dispatchNext(mailbox);
        }
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 XWalkScheduler::requestStatus(::ctrl::uint32 mailBoxId) noexcept
    {
        xSchedulerMailbox* mailbox = findMailbox(mailBoxId);
        if (mailbox == nullptr)
        {
            return XWALK_SCHEDULER_UNKNOWN_MAILBOX;
        }
        return sendLifecycleCommand(mailbox, XWALK_COMMAND_GET_STATUS);
    }

    ::ctrl::int32 XWalkScheduler::pollStatus(xProcessStatus* statusList,
                                             ::ctrl::size statusListSize,
                                             ::ctrl::int32 timeoutMilliseconds) noexcept
    {
        if ((statusList == nullptr) || (statusListSize == 0U) || (timeoutMilliseconds < 0))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }

        struct pollfd pollDescriptors[XWALK_MAX_PROCESSES]{};
        ::ctrl::size mailboxSlots[XWALK_MAX_PROCESSES]{};
        nfds_t descriptorCount{0U};
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            if (mailboxes[slot].registered && (mailboxes[slot].parentSocket >= 0))
            {
                pollDescriptors[descriptorCount].fd = mailboxes[slot].parentSocket;
                pollDescriptors[descriptorCount].events = POLLIN;
                mailboxSlots[descriptorCount] = slot;
                ++descriptorCount;
            }
        }

        ::ctrl::int32 pollResult{-1};
        do
        {
            pollResult = ::poll(pollDescriptors, descriptorCount, timeoutMilliseconds);
        } while ((pollResult < 0) && (errno == EINTR));
        if (pollResult < 0)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler poll failed: %s", std::strerror(errno));
            reapChildren();
            return XWALK_SCHEDULER_IPC_FAILURE;
        }

        ::ctrl::size statusCount{0U};
        for (nfds_t descriptorIndex = 0U; descriptorIndex < descriptorCount; ++descriptorIndex)
        {
            xSchedulerMailbox* mailbox = &mailboxes[mailboxSlots[descriptorIndex]];
            const short events = pollDescriptors[descriptorIndex].revents;
            const ::ctrl::boolean expectedPeerClosure =
                static_cast<::ctrl::boolean>(((events & POLLHUP) != 0) && mailbox->shutdownRequested);
            if (((events & POLLIN) != 0) && (expectedPeerClosure == false))
            {
                xSchedulerPacket packet{};
                const ::ctrl::int32 receiveResult = receivePacket(mailbox->parentSocket, &packet);
                if (receiveResult == XWALK_SCHEDULER_OK)
                {
                    xProcessStatus event{};
                    const ::ctrl::int32 processResult = processPacket(mailbox, &packet, &event);
                    if (processResult == XWALK_SCHEDULER_OK)
                    {
                        if (statusCount < statusListSize)
                        {
                            statusList[statusCount] = event;
                            ++statusCount;
                        }
                        else
                        {
                            XWALK_CTRL_WARNING(XWALK_RANGE, "Scheduler status output buffer is too small");
                        }
                    }
                }
                else
                {
                    mailbox->processState = XWALK_PROCESS_FAILED;
                    updateLastStatus(mailbox, nullptr, receiveResult);
                }
            }
            if ((events & (POLLERR | POLLNVAL)) != 0)
            {
                XWALK_CTRL_WARNING(XWALK_SYSTEM,
                                   "Scheduler mailbox %u reported socket event 0x%x",
                                   mailbox->mailBoxId,
                                   static_cast<unsigned>(events));
                mailbox->processState = XWALK_PROCESS_FAILED;
                updateLastStatus(mailbox, nullptr, XWALK_SCHEDULER_IPC_FAILURE);
                closeDescriptor(&mailbox->parentSocket);
            }
            else if ((events & POLLHUP) != 0)
            {
                if (mailbox->shutdownRequested == false)
                {
                    XWALK_CTRL_WARNING(
                        XWALK_SYSTEM, "Scheduler mailbox %u closed its status socket", mailbox->mailBoxId);
                }
                closeDescriptor(&mailbox->parentSocket);
            }
        }
        reapChildren();
        return static_cast<::ctrl::int32>(statusCount);
    }

    ::ctrl::int32 XWalkScheduler::getRequestStatus(const xClientAddress* requestAddress,
                                                   xRequestStatus* requestStatus) const noexcept
    {
        if ((requestAddress == nullptr) || (requestStatus == nullptr) || (requestAddress->xWalkLocalIndex == 0U))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        const xSchedulerMailbox* mailbox = findMailbox(requestAddress->mailBoxId);
        if (mailbox == nullptr)
        {
            return XWALK_SCHEDULER_UNKNOWN_MAILBOX;
        }
        const xRequestStatus* record = findRequest(mailbox, requestAddress);
        if (record == nullptr)
        {
            return XWALK_SCHEDULER_NOT_FOUND;
        }
        *requestStatus = *record;
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 XWalkScheduler::getProcessStatusMasks(xProcessStatusMasks* statusMasks) const noexcept
    {
        if (statusMasks == nullptr)
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        std::memset(statusMasks, 0, sizeof(*statusMasks));
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            const xSchedulerMailbox& mailbox = mailboxes[slot];
            if (mailbox.registered == false)
            {
                continue;
            }
            const ::ctrl::uint64 bit = XWALK_PROCESS_BIT(slot);
            statusMasks->registeredMask |= bit;
            if (mailbox.processId > 0)
            {
                statusMasks->aliveMask |= bit;
            }
            switch (mailbox.processState)
            {
                case XWALK_PROCESS_CREATED:
                    statusMasks->createdMask |= bit;
                    break;
                case XWALK_PROCESS_STOPPED:
                    statusMasks->stoppedMask |= bit;
                    break;
                case XWALK_PROCESS_STARTING:
                    statusMasks->startingMask |= bit;
                    break;
                case XWALK_PROCESS_RUNNING:
                    statusMasks->runningMask |= bit;
                    break;
                case XWALK_PROCESS_STOPPING:
                    statusMasks->stoppingMask |= bit;
                    break;
                case XWALK_PROCESS_EXITED:
                    statusMasks->exitedMask |= bit;
                    break;
                case XWALK_PROCESS_FAILED:
                    statusMasks->failedMask |= bit;
                    break;
                case XWALK_PROCESS_UNUSED:
                default:
                    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Registered scheduler slot %zu has invalid process state", slot);
                    return XWALK_SCHEDULER_PROCESS_FAILURE;
            }
        }
        statusMasks->registeredCount = getProcessCount(statusMasks->registeredMask);
        statusMasks->aliveCount = getProcessCount(statusMasks->aliveMask);
        statusMasks->runningCount = getProcessCount(statusMasks->runningMask);
        statusMasks->stoppedCount = getProcessCount(statusMasks->stoppedMask);
        statusMasks->failedCount = getProcessCount(statusMasks->failedMask);
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::uint64 XWalkScheduler::getProcessMask(xProcessState state) const noexcept
    {
        xProcessStatusMasks masks{};
        const ::ctrl::int32 statusResult = getProcessStatusMasks(&masks);
        if (statusResult != XWALK_SCHEDULER_OK)
        {
            return 0U;
        }
        switch (state)
        {
            case XWALK_PROCESS_CREATED:
                return masks.createdMask;
            case XWALK_PROCESS_STOPPED:
                return masks.stoppedMask;
            case XWALK_PROCESS_STARTING:
                return masks.startingMask;
            case XWALK_PROCESS_RUNNING:
                return masks.runningMask;
            case XWALK_PROCESS_STOPPING:
                return masks.stoppingMask;
            case XWALK_PROCESS_EXITED:
                return masks.exitedMask;
            case XWALK_PROCESS_FAILED:
                return masks.failedMask;
            case XWALK_PROCESS_UNUSED:
            default:
                return 0U;
        }
    }

    ::ctrl::uint32 XWalkScheduler::getProcessCount(::ctrl::uint64 processMask) const noexcept
    {
        ::ctrl::uint32 count{0U};
        while (processMask != 0U)
        {
            count += static_cast<::ctrl::uint32>(processMask & 1U);
            processMask >>= 1U;
        }
        return count;
    }

    ::ctrl::int32 XWalkScheduler::getProcessesByMask(::ctrl::uint64 processMask,
                                                     xProcessStatus* statusList,
                                                     ::ctrl::size statusListSize) const noexcept
    {
        if ((statusList == nullptr) && (processMask != 0U))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        const ::ctrl::uint32 requestedCount = getProcessCount(processMask);
        if (statusListSize < requestedCount)
        {
            XWALK_CTRL_WARNING(XWALK_RANGE, "Scheduler process-status output buffer is too small");
            return XWALK_SCHEDULER_BUFFER_TOO_SMALL;
        }
        ::ctrl::size outputIndex{0U};
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            const ::ctrl::uint64 processBit = XWALK_PROCESS_BIT(slot);
            if ((processMask & processBit) != 0U)
            {
                if (mailboxes[slot].registered == false)
                {
                    return XWALK_SCHEDULER_INVALID_ARGUMENT;
                }
                statusList[outputIndex] = mailboxes[slot].lastStatus;
                ++outputIndex;
            }
        }
        return static_cast<::ctrl::int32>(outputIndex);
    }

    void XWalkScheduler::shutdownAll() noexcept
    {
        if (shutdownInProgress)
        {
            return;
        }
        shutdownInProgress = true;
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            if (mailboxes[slot].registered)
            {
                static_cast<void>(shutdownModule(mailboxes[slot].mailBoxId));
            }
        }
        waitForChildren(XWALK_SCHEDULER_GRACE_TIMEOUT_MS);

        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            xSchedulerMailbox& mailbox = mailboxes[slot];
            if (mailbox.registered && (mailbox.processId > 0))
            {
                XWALK_CTRL_WARNING(XWALK_SYSTEM,
                                   "Scheduler grace period expired for mailbox %u pid %ld",
                                   mailbox.mailBoxId,
                                   static_cast<long>(mailbox.processId));
                const ::ctrl::int32 killResult = ::kill(mailbox.processId, SIGTERM);
                if (killResult != 0)
                {
                    XWALK_CTRL_ERROR(XWALK_EXCEPTION,
                                     "Scheduler SIGTERM failed for owned pid %ld: %s",
                                     static_cast<long>(mailbox.processId),
                                     std::strerror(errno));
                }
            }
        }
        waitForChildren(XWALK_SCHEDULER_TERMINATE_TIMEOUT_MS);

        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            xSchedulerMailbox& mailbox = mailboxes[slot];
            if (mailbox.registered && (mailbox.processId > 0))
            {
                XWALK_CTRL_ERROR(XWALK_EXCEPTION,
                                 "Scheduler could not reap owned pid %ld after SIGTERM",
                                 static_cast<long>(mailbox.processId));
            }
            closeDescriptor(&mailbox.parentSocket);
            resolveRequests(&mailbox, XWALK_REQUEST_CANCELLED, XWALK_SCHEDULER_SHUTTING_DOWN);
        }
        clearFinalizedRecords();
        XWALK_CTRL_TRACE_UID0(CTRL .099, "Controller process scheduler shutdown completed");
    }

    /**************************************************************************
     * Protected member function definitions
     **************************************************************************/

    ::ctrl::int32 XWalkScheduler::runChild(::ctrl::int32 childSocket,
                                           ::ctrl::uint32 mailBoxId,
                                           const xModuleCallbacks* callbacks) noexcept
    {
        if ((childSocket < 0) || (mailBoxId == 0U) || (callbacks == nullptr) ||
            ((callbacks->onCommand == nullptr) && (callbacks->onSignal == nullptr)))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }

        xWalkSetChildSocket(childSocket);
        xProcessState processState{XWALK_PROCESS_CREATED};
        ::ctrl::boolean childRunning{true};
        while (childRunning)
        {
            struct pollfd descriptor
            {
            };
            descriptor.fd = childSocket;
            descriptor.events = POLLIN;
            ::ctrl::int32 pollResult{-1};
            do
            {
                pollResult = ::poll(&descriptor, 1U, XWALK_SCHEDULER_POLL_SLICE_MS);
            } while ((pollResult < 0) && (errno == EINTR));
            if (pollResult < 0)
            {
                return XWALK_SCHEDULER_IPC_FAILURE;
            }
            if (pollResult == 0)
            {
                if ((processState == XWALK_PROCESS_RUNNING) && (callbacks->onTick != nullptr))
                {
                    static_cast<void>(callbacks->onTick(callbacks->context));
                }
                continue;
            }
            if ((descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0)
            {
                return XWALK_SCHEDULER_IPC_FAILURE;
            }
            if ((descriptor.revents & POLLIN) == 0)
            {
                continue;
            }

            xSchedulerPacket command{};
            const ::ctrl::int32 receiveResult = receivePacket(childSocket, &command);
            const ::ctrl::boolean commandPacketValid = packetValid(&command, mailBoxId);
            const ::ctrl::boolean receiveFailed = (receiveResult != XWALK_SCHEDULER_OK) || !commandPacketValid ||
                                                  (command.packetType != XWALK_SCHEDULER_PACKET_COMMAND);
            if (receiveFailed)
            {
                return XWALK_SCHEDULER_IPC_FAILURE;
            }

            xSchedulerPacket status{};
            initializePacket(&status);
            status.packetType = XWALK_SCHEDULER_PACKET_PROCESS_STATUS;
            status.commandType = command.commandType;
            status.address = command.address;
            status.commandId = command.commandId;
            status.processState = processState;
            status.requestState = XWALK_REQUEST_UNUSED;
            ::ctrl::int32 statusSendResult{XWALK_SCHEDULER_OK};

            switch (command.commandType)
            {
                case XWALK_COMMAND_START:
                    status.result = callbacks->onStart == nullptr ? 0 : callbacks->onStart(callbacks->context);
                    processState = status.result == 0 ? XWALK_PROCESS_RUNNING : XWALK_PROCESS_FAILED;
                    status.processState = processState;
                    statusSendResult = sendPacket(childSocket, &status);
                    if (statusSendResult != XWALK_SCHEDULER_OK)
                    {
                        return XWALK_SCHEDULER_IPC_FAILURE;
                    }
                    break;
                case XWALK_COMMAND_STOP:
                    status.result = callbacks->onStop == nullptr ? 0 : callbacks->onStop(callbacks->context);
                    processState = status.result == 0 ? XWALK_PROCESS_STOPPED : XWALK_PROCESS_FAILED;
                    status.processState = processState;
                    statusSendResult = sendPacket(childSocket, &status);
                    if (statusSendResult != XWALK_SCHEDULER_OK)
                    {
                        return XWALK_SCHEDULER_IPC_FAILURE;
                    }
                    break;
                case XWALK_COMMAND_CUSTOM:
                    if ((processState != XWALK_PROCESS_RUNNING) || (command.address.xWalkLocalIndex == 0U))
                    {
                        return XWALK_SCHEDULER_INVALID_STATE;
                    }
                    status.packetType = XWALK_SCHEDULER_PACKET_REQUEST_STATUS;
                    status.requestState = XWALK_REQUEST_RUNNING;
                    statusSendResult = sendPacket(childSocket, &status);
                    if (statusSendResult != XWALK_SCHEDULER_OK)
                    {
                        return XWALK_SCHEDULER_IPC_FAILURE;
                    }
                    status.dataType = command.dataType;
                    if (command.dataType == XWALK_SCHEDULER_DATA_SIGNAL)
                    {
                        XWalkSignal signal{};
                        if ((callbacks->onSignal == nullptr) || (command.dataSize != sizeof(signal)))
                        {
                            status.result = XWALK_SCHEDULER_INVALID_ARGUMENT;
                        }
                        else
                        {
                            static_cast<void>(std::memcpy(&signal, command.commandData, sizeof(signal)));
                            xWalkBeginChildSignal(&signal);
                            status.result = callbacks->onSignal(callbacks->context, &signal);
                            const ::ctrl::uint32 responseSignal =
                                status.result == 0 ? cxx_xWalkGetConfirmationSignal_LPP(command.commandId)
                                                   : cxx_xWalkGetRejectionSignal_LPP(command.commandId);
                            XWalkSignal handlerResponse{};
                            const ::ctrl::boolean responseReady = xWalkTakeChildSignal(&handlerResponse);
                            xWalkEndChildSignal();
                            if (responseSignal == CXX_XWALK_SIGNAL_UNSPECIFIED)
                            {
                                status.result = XWALK_SCHEDULER_INVALID_ARGUMENT;
                            }
                            else
                            {
                                if (responseReady && (handlerResponse.sigNo != responseSignal))
                                {
                                    status.result = XWALK_SCHEDULER_INVALID_ARGUMENT;
                                    signal.sigNo = cxx_xWalkGetRejectionSignal_LPP(command.commandId);
                                    signal.payloadSize = 0U;
                                }
                                else if (responseReady)
                                {
                                    signal = handlerResponse;
                                }
                                else if (status.result == 0)
                                {
                                    signal.sigNo = responseSignal;
                                    signal.payloadSize = 0U;
                                }
                                else
                                {
                                    signal.sigNo = responseSignal;
                                    xWalkRejectPayload rejection{};
                                    const std::int64_t signedResult = status.result;
                                    const ::ctrl::uint64 magnitude = signedResult < 0
                                                                         ? static_cast<::ctrl::uint64>(-signedResult)
                                                                         : static_cast<::ctrl::uint64>(signedResult);
                                    rejection.reason = static_cast<::ctrl::uint32>(magnitude);
                                    rejection.errorSignal = 6U;
                                    constexpr char detail[]{"Scheduled handler returned failure"};
                                    static_cast<void>(std::memcpy(rejection.detail, detail, sizeof(detail)));
                                    signal.payloadSize = sizeof(rejection);
                                    static_cast<void>(std::memcpy(signal.payload, &rejection, sizeof(rejection)));
                                }
                                status.dataSize = sizeof(signal);
                                if (status.dataSize > sizeof(status.commandData))
                                {
                                    status.result = XWALK_SCHEDULER_PROCESS_FAILURE;
                                    status.dataSize = 0U;
                                }
                                else
                                {
                                    static_cast<void>(std::memcpy(status.commandData, &signal, sizeof(signal)));
                                }
                                XWALK_CTRL_TRACE_UID3(CTRL .103,
                                                      "Scheduler generated response %u mailbox %u local %u",
                                                      signal.sigNo,
                                                      command.address.mailBoxId,
                                                      command.address.xWalkLocalIndex);
                            }
                        }
                    }
                    else if (callbacks->onCommand != nullptr)
                    {
                        status.result = callbacks->onCommand(
                            command.commandId, reinterpret_cast<const char*>(command.commandData), callbacks->context);
                    }
                    else
                    {
                        status.result = XWALK_SCHEDULER_INVALID_STATE;
                    }
                    status.requestState = status.result == 0 ? XWALK_REQUEST_COMPLETED : XWALK_REQUEST_FAILED;
                    statusSendResult = sendPacket(childSocket, &status);
                    if (statusSendResult != XWALK_SCHEDULER_OK)
                    {
                        return XWALK_SCHEDULER_IPC_FAILURE;
                    }
                    break;
                case XWALK_COMMAND_GET_STATUS:
                    status.processState = processState;
                    statusSendResult = sendPacket(childSocket, &status);
                    if (statusSendResult != XWALK_SCHEDULER_OK)
                    {
                        return XWALK_SCHEDULER_IPC_FAILURE;
                    }
                    break;
                case XWALK_COMMAND_SHUTDOWN:
                    if ((processState == XWALK_PROCESS_RUNNING) || (processState == XWALK_PROCESS_STOPPING))
                    {
                        status.result = callbacks->onStop == nullptr ? 0 : callbacks->onStop(callbacks->context);
                    }
                    processState = status.result == 0 ? XWALK_PROCESS_EXITED : XWALK_PROCESS_FAILED;
                    status.processState = processState;
                    static_cast<void>(sendPacket(childSocket, &status));
                    childRunning = false;
                    break;
                default:
                    return XWALK_SCHEDULER_INVALID_ARGUMENT;
            }
        }
        xWalkSetChildSocket(-1);
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 XWalkScheduler::sendPacket(::ctrl::int32 socketDescriptor, const xSchedulerPacket* packet) noexcept
    {
        if ((socketDescriptor < 0) || (packet == nullptr))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        ssize_t sentBytes{-1};
        do
        {
            sentBytes = ::send(socketDescriptor, packet, sizeof(*packet), MSG_NOSIGNAL);
        } while ((sentBytes < 0) && (errno == EINTR));
        if ((sentBytes < 0) && (errno == EPERM))
        {
            do
            {
                sentBytes = ::write(socketDescriptor, packet, sizeof(*packet));
            } while ((sentBytes < 0) && (errno == EINTR));
        }
        if (sentBytes != static_cast<ssize_t>(sizeof(*packet)))
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler IPC send failed: %s", std::strerror(errno));
            return XWALK_SCHEDULER_IPC_FAILURE;
        }
        XWALK_CTRL_TRACE_UID3(CTRL .100,
                              "Scheduler sent packet type %u mailbox %u local %u",
                              static_cast<::ctrl::uint32>(packet->packetType),
                              packet->address.mailBoxId,
                              packet->address.xWalkLocalIndex);
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 XWalkScheduler::receivePacket(::ctrl::int32 socketDescriptor, xSchedulerPacket* packet) noexcept
    {
        if ((socketDescriptor < 0) || (packet == nullptr))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        ssize_t receivedBytes{-1};
        do
        {
            receivedBytes = ::recv(socketDescriptor, packet, sizeof(*packet), MSG_TRUNC);
        } while ((receivedBytes < 0) && (errno == EINTR));
        if (receivedBytes != static_cast<ssize_t>(sizeof(*packet)))
        {
            XWALK_CTRL_ERROR(
                XWALK_EXCEPTION, "Scheduler IPC packet has invalid size %ld", static_cast<long>(receivedBytes));
            return XWALK_SCHEDULER_IPC_FAILURE;
        }
        XWALK_CTRL_TRACE_UID3(CTRL .101,
                              "Scheduler received packet type %u mailbox %u local %u",
                              static_cast<::ctrl::uint32>(packet->packetType),
                              packet->address.mailBoxId,
                              packet->address.xWalkLocalIndex);
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::boolean XWalkScheduler::packetValid(const xSchedulerPacket* packet, ::ctrl::uint32 mailBoxId) noexcept
    {
        if ((packet == nullptr) || (packet->protocolVersion != XWALK_SCHEDULER_PROTOCOL_VERSION) ||
            (packet->messageSize != sizeof(*packet)) || (packet->address.mailBoxId != mailBoxId) ||
            (packet->dataSize > sizeof(packet->commandData)) ||
            ((packet->dataType != XWALK_SCHEDULER_DATA_TEXT) && (packet->dataType != XWALK_SCHEDULER_DATA_SIGNAL)) ||
            ((packet->dataType == XWALK_SCHEDULER_DATA_TEXT) &&
             ((packet->dataSize >= sizeof(packet->commandData)) || (packet->commandData[packet->dataSize] != 0U))))
        {
            return false;
        }
        for (::ctrl::size index = 0U; index < (sizeof(packet->reserved) / sizeof(packet->reserved[0U])); ++index)
        {
            if (packet->reserved[index] != 0U)
            {
                return false;
            }
        }
        return true;
    }

    void XWalkScheduler::initializePacket(xSchedulerPacket* packet) noexcept
    {
        if (packet != nullptr)
        {
            std::memset(packet, 0, sizeof(*packet));
            packet->protocolVersion = XWALK_SCHEDULER_PROTOCOL_VERSION;
            packet->messageSize = sizeof(*packet);
        }
    }

    void XWalkScheduler::closeDescriptor(::ctrl::int32* descriptor) noexcept
    {
        if ((descriptor != nullptr) && (*descriptor >= 0))
        {
            const ::ctrl::int32 closeResult = ::close(*descriptor);
            if (closeResult != 0)
            {
                XWALK_CTRL_WARNING(XWALK_SYSTEM, "Scheduler descriptor close failed: %s", std::strerror(errno));
            }
            *descriptor = -1;
        }
    }

    xSchedulerMailbox* XWalkScheduler::findMailbox(::ctrl::uint32 mailBoxId) noexcept
    {
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            if (mailboxes[slot].registered && (mailboxes[slot].mailBoxId == mailBoxId))
            {
                return &mailboxes[slot];
            }
        }
        return nullptr;
    }

    const xSchedulerMailbox* XWalkScheduler::findMailbox(::ctrl::uint32 mailBoxId) const noexcept
    {
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            if (mailboxes[slot].registered && (mailboxes[slot].mailBoxId == mailBoxId))
            {
                return &mailboxes[slot];
            }
        }
        return nullptr;
    }

    xRequestStatus* XWalkScheduler::findRequest(xSchedulerMailbox* mailbox, const xClientAddress* address) noexcept
    {
        if ((mailbox == nullptr) || (address == nullptr))
        {
            return nullptr;
        }
        for (::ctrl::size index = 0U; index < XWALK_MAX_REQUEST_RECORDS; ++index)
        {
            xRequestStatus& record = mailbox->requestRecords[index];
            const ::ctrl::int32 addressComparison =
                std::strncmp(record.address.clientAddress, address->clientAddress, XWALK_CLIENT_ADDRESS_SIZE);
            if ((record.state != XWALK_REQUEST_UNUSED) &&
                (record.address.xWalkLocalIndex == address->xWalkLocalIndex) &&
                (record.address.mailBoxId == address->mailBoxId) && (addressComparison == 0))
            {
                return &record;
            }
        }
        return nullptr;
    }

    const xRequestStatus* XWalkScheduler::findRequest(const xSchedulerMailbox* mailbox,
                                                      const xClientAddress* address) const noexcept
    {
        if ((mailbox == nullptr) || (address == nullptr))
        {
            return nullptr;
        }
        for (::ctrl::size index = 0U; index < XWALK_MAX_REQUEST_RECORDS; ++index)
        {
            const xRequestStatus& record = mailbox->requestRecords[index];
            const ::ctrl::int32 addressComparison =
                std::strncmp(record.address.clientAddress, address->clientAddress, XWALK_CLIENT_ADDRESS_SIZE);
            if ((record.state != XWALK_REQUEST_UNUSED) &&
                (record.address.xWalkLocalIndex == address->xWalkLocalIndex) &&
                (record.address.mailBoxId == address->mailBoxId) && (addressComparison == 0))
            {
                return &record;
            }
        }
        return nullptr;
    }

    ::ctrl::int32 XWalkScheduler::allocateRequestRecord(xSchedulerMailbox* mailbox,
                                                        const xSchedulerCommand* command) noexcept
    {
        if ((mailbox == nullptr) || (command == nullptr))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        ::ctrl::size selectedIndex{XWALK_MAX_REQUEST_RECORDS};
        ::ctrl::uint64 oldestSequence{UINT64_MAX};
        for (::ctrl::size index = 0U; index < XWALK_MAX_REQUEST_RECORDS; ++index)
        {
            const xRequestState state = mailbox->requestRecords[index].state;
            if (state == XWALK_REQUEST_UNUSED)
            {
                selectedIndex = index;
                break;
            }
            if (((state == XWALK_REQUEST_COMPLETED) || (state == XWALK_REQUEST_FAILED) ||
                 (state == XWALK_REQUEST_CANCELLED)) &&
                (mailbox->requestSequence[index] < oldestSequence))
            {
                selectedIndex = index;
                oldestSequence = mailbox->requestSequence[index];
            }
        }
        if (selectedIndex >= XWALK_MAX_REQUEST_RECORDS)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler request table full for mailbox %u", mailbox->mailBoxId);
            return XWALK_SCHEDULER_REQUEST_TABLE_FULL;
        }
        if (mailbox->requestRecords[selectedIndex].state != XWALK_REQUEST_UNUSED)
        {
            XWALK_CTRL_WARNING(XWALK_RANGE, "Scheduler evicted a retained request for mailbox %u", mailbox->mailBoxId);
        }
        xRequestStatus& record = mailbox->requestRecords[selectedIndex];
        std::memset(&record, 0, sizeof(record));
        record.address = command->address;
        record.commandId = command->commandId;
        record.state = XWALK_REQUEST_PENDING;
        record.result = XWALK_SCHEDULER_OK;
        mailbox->requestSequence[selectedIndex] = mailbox->nextRequestSequence;
        ++mailbox->nextRequestSequence;
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 XWalkScheduler::allocateLocalIndex(xSchedulerMailbox* mailbox,
                                                     ::ctrl::uint32* localIndex) const noexcept
    {
        if ((mailbox == nullptr) || (localIndex == nullptr))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        ::ctrl::uint32 candidate = mailbox->nextLocalIndex;
        for (::ctrl::uint64 attempts = 0U; attempts < UINT32_MAX; ++attempts)
        {
            if (candidate == 0U)
            {
                candidate = 1U;
            }
            xClientAddress address{};
            address.mailBoxId = mailbox->mailBoxId;
            static_cast<void>(
                std::memcpy(address.clientAddress, mailbox->clientAddress, sizeof(address.clientAddress)));
            address.xWalkLocalIndex = candidate;
            const xRequestStatus* existingRequest = findRequest(mailbox, &address);
            if (existingRequest == nullptr)
            {
                *localIndex = candidate;
                mailbox->nextLocalIndex = candidate + 1U;
                if (mailbox->nextLocalIndex == 0U)
                {
                    mailbox->nextLocalIndex = 1U;
                }
                return XWALK_SCHEDULER_OK;
            }
            ++candidate;
        }
        XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler local-index space exhausted for mailbox %u", mailbox->mailBoxId);
        return XWALK_SCHEDULER_LOCAL_INDEX_EXHAUSTED;
    }

    ::ctrl::int32 XWalkScheduler::dispatchNext(xSchedulerMailbox* mailbox) noexcept
    {
        if ((mailbox == nullptr) || (mailbox->queueCount == 0U) || mailbox->requestActive ||
            !mailbox->dispatchEnabled || (mailbox->processState != XWALK_PROCESS_RUNNING))
        {
            return XWALK_SCHEDULER_OK;
        }
        mailbox->activeRequest = mailbox->pendingQueue[mailbox->queueHead];
        mailbox->queueHead = (mailbox->queueHead + 1U) % XWALK_MAX_PENDING_COMMANDS;
        --mailbox->queueCount;
        mailbox->requestActive = true;

        xSchedulerPacket packet{};
        initializePacket(&packet);
        packet.packetType = XWALK_SCHEDULER_PACKET_COMMAND;
        packet.commandType = XWALK_COMMAND_CUSTOM;
        packet.address = mailbox->activeRequest.address;
        packet.commandId = mailbox->activeRequest.commandId;
        packet.dataType = mailbox->activeRequest.dataType;
        packet.dataSize = mailbox->activeRequest.dataSize;
        static_cast<void>(
            std::memcpy(packet.commandData, mailbox->activeRequest.commandData, sizeof(packet.commandData)));
        const ::ctrl::int32 sendResult = sendPacket(mailbox->parentSocket, &packet);
        if (sendResult != XWALK_SCHEDULER_OK)
        {
            xRequestStatus* record = findRequest(mailbox, &mailbox->activeRequest.address);
            if (record != nullptr)
            {
                record->state = XWALK_REQUEST_FAILED;
                record->result = sendResult;
            }
            mailbox->requestActive = false;
            mailbox->processState = XWALK_PROCESS_FAILED;
            return sendResult;
        }
        XWALK_CTRL_TRACE_UID3(CTRL .093,
                              "Scheduler dispatched mailbox %u local %u command %u",
                              mailbox->mailBoxId,
                              mailbox->activeRequest.address.xWalkLocalIndex,
                              mailbox->activeRequest.commandId);
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 XWalkScheduler::processPacket(xSchedulerMailbox* mailbox,
                                                const xSchedulerPacket* packet,
                                                xProcessStatus* status) noexcept
    {
        const ::ctrl::boolean packetIsValid = mailbox != nullptr && packetValid(packet, mailbox->mailBoxId);
        const ::ctrl::boolean packetRejected = !packetIsValid || (status == nullptr);
        if (packetRejected)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduler rejected an invalid status packet");
            return XWALK_SCHEDULER_IPC_FAILURE;
        }
        if (packet->packetType == XWALK_SCHEDULER_PACKET_NESTED_SIGNAL)
        {
            XWalkSignal signal{};
            if (packet->dataSize != sizeof(signal))
            {
                return XWALK_SCHEDULER_INVALID_ARGUMENT;
            }
            static_cast<void>(std::memcpy(&signal, packet->commandData, sizeof(signal)));
            const ::ctrl::uint32 destinationMailbox = signal.destination == XWALK_MODULE_AGENT ? XWALK_AGENT_MAILBOX_ID
                                                      : signal.destination == XWALK_MODULE_HAL ? XWALK_HAL_MAILBOX_ID
                                                                                               : 0U;
            if ((destinationMailbox == 0U) || (signal.clientInfo.mailBoxId != destinationMailbox))
            {
                return XWALK_SCHEDULER_INVALID_ARGUMENT;
            }
            const ::ctrl::int32 sendResult = submitSignal(
                destinationMailbox, signal.sigNo, signal.payload, signal.payloadSize, &signal.clientInfo, nullptr);
            updateLastStatus(mailbox, nullptr, sendResult);
            *status = mailbox->lastStatus;
            return sendResult;
        }
        if (packet->packetType == XWALK_SCHEDULER_PACKET_PROCESS_STATUS)
        {
            mailbox->processState = packet->processState;
            mailbox->dispatchEnabled = static_cast<::ctrl::boolean>(packet->processState == XWALK_PROCESS_RUNNING);
            updateLastStatus(mailbox, nullptr, packet->result);
            *status = mailbox->lastStatus;
            if (packet->processState == XWALK_PROCESS_RUNNING)
            {
                XWALK_CTRL_TRACE_UID2(CTRL .094,
                                      "Scheduler started mailbox %u pid %ld",
                                      mailbox->mailBoxId,
                                      static_cast<long>(mailbox->processId));
                static_cast<void>(dispatchNext(mailbox));
            }
            else if (packet->processState == XWALK_PROCESS_STOPPED)
            {
                XWALK_CTRL_TRACE_UID2(CTRL .095,
                                      "Scheduler stopped mailbox %u pid %ld",
                                      mailbox->mailBoxId,
                                      static_cast<long>(mailbox->processId));
            }
            return XWALK_SCHEDULER_OK;
        }
        if (packet->packetType != XWALK_SCHEDULER_PACKET_REQUEST_STATUS)
        {
            XWALK_CTRL_WARNING(
                XWALK_INVAL, "Scheduler ignored unknown status packet for mailbox %u", mailbox->mailBoxId);
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        const ::ctrl::int32 clientAddressComparison = std::strncmp(
            mailbox->activeRequest.address.clientAddress, packet->address.clientAddress, XWALK_CLIENT_ADDRESS_SIZE);
        if (!mailbox->requestActive ||
            (mailbox->activeRequest.address.xWalkLocalIndex != packet->address.xWalkLocalIndex) ||
            (mailbox->activeRequest.commandId != packet->commandId) || (clientAddressComparison != 0))
        {
            XWALK_CTRL_WARNING(XWALK_LOGIC,
                               "Scheduler rejected stale or mismatched completion for mailbox %u local %u",
                               mailbox->mailBoxId,
                               packet->address.xWalkLocalIndex);
            return XWALK_SCHEDULER_NOT_FOUND;
        }
        xRequestStatus* record = findRequest(mailbox, &packet->address);
        if (record == nullptr)
        {
            return XWALK_SCHEDULER_NOT_FOUND;
        }
        if (packet->requestState == XWALK_REQUEST_RUNNING)
        {
            if (record->state != XWALK_REQUEST_PENDING)
            {
                XWALK_CTRL_WARNING(XWALK_LOGIC,
                                   "Scheduler rejected duplicate running status for mailbox %u local %u",
                                   mailbox->mailBoxId,
                                   packet->address.xWalkLocalIndex);
                return XWALK_SCHEDULER_INVALID_STATE;
            }
            record->state = XWALK_REQUEST_RUNNING;
            record->result = packet->result;
        }
        else if ((packet->requestState == XWALK_REQUEST_COMPLETED) || (packet->requestState == XWALK_REQUEST_FAILED))
        {
            if ((record->state != XWALK_REQUEST_RUNNING) && (record->state != XWALK_REQUEST_PENDING))
            {
                XWALK_CTRL_WARNING(XWALK_LOGIC,
                                   "Scheduler rejected duplicate completion for mailbox %u local %u",
                                   mailbox->mailBoxId,
                                   packet->address.xWalkLocalIndex);
                return XWALK_SCHEDULER_INVALID_STATE;
            }
            record->state = packet->requestState;
            record->result = packet->result;
            mailbox->requestActive = false;
            std::memset(&mailbox->activeRequest, 0, sizeof(mailbox->activeRequest));
            XWALK_CTRL_TRACE_UID3(CTRL .096,
                                  "Scheduler completed mailbox %u local %u result %d",
                                  mailbox->mailBoxId,
                                  packet->address.xWalkLocalIndex,
                                  packet->result);
            static_cast<void>(dispatchNext(mailbox));
        }
        else
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        updateLastStatus(mailbox, record, packet->result);
        if ((packet->dataType == XWALK_SCHEDULER_DATA_SIGNAL) && (packet->dataSize <= XWALK_SCHEDULER_SIGNAL_SIZE))
        {
            mailbox->lastStatus.signalSize = packet->dataSize;
            static_cast<void>(std::memcpy(mailbox->lastStatus.signalData, packet->commandData, packet->dataSize));
        }
        *status = mailbox->lastStatus;
        return XWALK_SCHEDULER_OK;
    }

    void XWalkScheduler::resolveRequests(xSchedulerMailbox* mailbox, xRequestState state, ::ctrl::int32 result) noexcept
    {
        if (mailbox == nullptr)
        {
            return;
        }
        for (::ctrl::size index = 0U; index < XWALK_MAX_REQUEST_RECORDS; ++index)
        {
            xRequestStatus& record = mailbox->requestRecords[index];
            if ((record.state == XWALK_REQUEST_PENDING) || (record.state == XWALK_REQUEST_RUNNING))
            {
                record.state = state;
                record.result = result;
            }
        }
        mailbox->queueHead = 0U;
        mailbox->queueTail = 0U;
        mailbox->queueCount = 0U;
        mailbox->requestActive = false;
        std::memset(&mailbox->activeRequest, 0, sizeof(mailbox->activeRequest));
    }

    void XWalkScheduler::updateLastStatus(xSchedulerMailbox* mailbox,
                                          const xRequestStatus* request,
                                          ::ctrl::int32 result) noexcept
    {
        if (mailbox == nullptr)
        {
            return;
        }
        std::memset(&mailbox->lastStatus, 0, sizeof(mailbox->lastStatus));
        mailbox->lastStatus.processSlot = mailbox->processSlot;
        mailbox->lastStatus.mailBoxId = mailbox->mailBoxId;
        mailbox->lastStatus.moduleType = mailbox->moduleType;
        mailbox->lastStatus.processId = mailbox->processId;
        mailbox->lastStatus.state = mailbox->processState;
        mailbox->lastStatus.result = result;
        if (request != nullptr)
        {
            mailbox->lastStatus.request = *request;
        }
    }

    void XWalkScheduler::reapChildren() noexcept
    {
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            xSchedulerMailbox& mailbox = mailboxes[slot];
            if (!mailbox.registered || (mailbox.processId <= 0))
            {
                continue;
            }
            ::ctrl::int32 childStatus{0};
            pid_t waitResult{-1};
            do
            {
                waitResult = ::waitpid(mailbox.processId, &childStatus, WNOHANG);
            } while ((waitResult < 0) && (errno == EINTR));
            if (waitResult == mailbox.processId)
            {
                handleChildExit(&mailbox, childStatus);
            }
            else if ((waitResult < 0) && (errno == ECHILD))
            {
                mailbox.processId = 0;
                closeDescriptor(&mailbox.parentSocket);
                mailbox.processState = XWALK_PROCESS_FAILED;
                updateLastStatus(&mailbox, nullptr, XWALK_SCHEDULER_PROCESS_FAILURE);
            }
            else if (waitResult < 0)
            {
                XWALK_CTRL_ERROR(XWALK_EXCEPTION,
                                 "Scheduler waitpid failed for pid %ld: %s",
                                 static_cast<long>(mailbox.processId),
                                 std::strerror(errno));
            }
        }
    }

    void XWalkScheduler::handleChildExit(xSchedulerMailbox* mailbox, ::ctrl::int32 childStatus) noexcept
    {
        if (mailbox == nullptr)
        {
            return;
        }
        const pid_t exitedProcessId = mailbox->processId;
        mailbox->processId = 0;
        closeDescriptor(&mailbox->parentSocket);
        const ::ctrl::boolean childExited = WIFEXITED(childStatus);
        const ::ctrl::int32 childExitStatus = childExited ? WEXITSTATUS(childStatus) : EXIT_FAILURE;
        if (childExited && (childExitStatus == EXIT_SUCCESS))
        {
            mailbox->processState = XWALK_PROCESS_EXITED;
            XWALK_CTRL_TRACE_UID2(CTRL .097,
                                  "Scheduler reaped mailbox %u pid %ld",
                                  mailbox->mailBoxId,
                                  static_cast<long>(exitedProcessId));
        }
        else
        {
            mailbox->processState = XWALK_PROCESS_FAILED;
            const ::ctrl::boolean childSignaled = WIFSIGNALED(childStatus);
            if (childSignaled)
            {
                XWALK_CTRL_ERROR(XWALK_EXCEPTION,
                                 "Scheduler child pid %ld terminated by signal %d",
                                 static_cast<long>(exitedProcessId),
                                 WTERMSIG(childStatus));
            }
            else
            {
                XWALK_CTRL_ERROR(XWALK_EXCEPTION,
                                 "Scheduler child pid %ld exited unsuccessfully",
                                 static_cast<long>(exitedProcessId));
            }
        }
        resolveRequests(mailbox, XWALK_REQUEST_FAILED, XWALK_SCHEDULER_PROCESS_FAILURE);
        const ::ctrl::int32 exitResult =
            mailbox->processState == XWALK_PROCESS_EXITED ? XWALK_SCHEDULER_OK : XWALK_SCHEDULER_PROCESS_FAILURE;
        updateLastStatus(mailbox, nullptr, exitResult);
        mailbox->cleanupComplete = true;
    }

    ::ctrl::int32 XWalkScheduler::sendLifecycleCommand(xSchedulerMailbox* mailbox, xCommandType commandType) noexcept
    {
        if ((mailbox == nullptr) || (mailbox->parentSocket < 0) || (mailbox->processId <= 0))
        {
            return XWALK_SCHEDULER_INVALID_STATE;
        }
        xSchedulerPacket packet{};
        initializePacket(&packet);
        packet.packetType = XWALK_SCHEDULER_PACKET_COMMAND;
        packet.commandType = commandType;
        packet.address.mailBoxId = mailbox->mailBoxId;
        static_cast<void>(
            std::memcpy(packet.address.clientAddress, mailbox->clientAddress, sizeof(packet.address.clientAddress)));
        packet.address.moduleType = mailbox->moduleType;
        return sendPacket(mailbox->parentSocket, &packet);
    }

    ::ctrl::boolean XWalkScheduler::allChildrenReaped() const noexcept
    {
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            if (mailboxes[slot].registered && (mailboxes[slot].processId > 0))
            {
                return false;
            }
        }
        return true;
    }

    void XWalkScheduler::waitForChildren(::ctrl::int32 timeoutMilliseconds) noexcept
    {
        ::ctrl::int32 elapsedMilliseconds{0};
        xProcessStatus events[XWALK_MAX_STATUS_EVENTS]{};
        ::ctrl::boolean childrenPending = !allChildrenReaped() && (elapsedMilliseconds < timeoutMilliseconds);
        while (childrenPending)
        {
            static_cast<void>(pollStatus(events, XWALK_MAX_STATUS_EVENTS, XWALK_SCHEDULER_POLL_SLICE_MS));
            elapsedMilliseconds += XWALK_SCHEDULER_POLL_SLICE_MS;
            childrenPending = !allChildrenReaped() && (elapsedMilliseconds < timeoutMilliseconds);
        }
        reapChildren();
    }

    void XWalkScheduler::clearFinalizedRecords() noexcept
    {
        for (::ctrl::size slot = 0U; slot < XWALK_MAX_PROCESSES; ++slot)
        {
            xSchedulerMailbox& mailbox = mailboxes[slot];
            if (mailbox.processId <= 0)
            {
                const ::ctrl::uint32 stableSlot = mailbox.processSlot;
                closeDescriptor(&mailbox.parentSocket);
                std::memset(&mailbox, 0, sizeof(mailbox));
                mailbox.processSlot = stableSlot;
                mailbox.parentSocket = -1;
                mailbox.processState = XWALK_PROCESS_UNUSED;
            }
        }
    }

} /* namespace xwalk::ctrl */
