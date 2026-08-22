/******************************************************************************
 * @file        xControllerSchedulerTypes.h
 * @brief       Defines fixed-capacity Controller process-scheduler contracts.
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

#ifndef XCONTROLLER_SCHEDULER_TYPES_H
#define XCONTROLLER_SCHEDULER_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"
#include "xWalkCbbSignal.h"

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

/******************************************************************************
 * Macro definitions
 ******************************************************************************/

#define XWALK_SCHEDULER_COMMAND_DATA_SIZE 512U
#define XWALK_SCHEDULER_SIGNAL_SIZE 1024U
#define XWALK_MAX_PROCESSES 64U
#define XWALK_MAX_PENDING_COMMANDS 32U
#define XWALK_MAX_REQUEST_RECORDS 128U
#define XWALK_MAX_STATUS_EVENTS 64U
#define XWALK_SCHEDULER_PROTOCOL_VERSION 1U
#define XWALK_SCHEDULER_GRACE_TIMEOUT_MS 1500
#define XWALK_SCHEDULER_TERMINATE_TIMEOUT_MS 1500
#define XWALK_SCHEDULER_POLL_SLICE_MS 25

/** @brief Returns the stable mask bit for a validated process-table slot. */
#define XWALK_PROCESS_BIT(INDEX) (::xwalk::ctrl::xWalkProcessBit((INDEX)))

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::ctrl
{

    /** @brief Returns zero instead of performing an invalid shift for an out-of-range slot. */
    inline constexpr ::ctrl::uint64 xWalkProcessBit(::ctrl::size index) noexcept
    {
        return index < XWALK_MAX_PROCESSES ? (UINT64_C(1) << index) : UINT64_C(0);
    }

    /**************************************************************************
     * Enumeration declarations
     **************************************************************************/

    /** @brief Reports one scheduler API result without throwing an exception. */
    typedef enum xSchedulerResult
    {
        XWALK_SCHEDULER_OK = 0,
        XWALK_SCHEDULER_INVALID_ARGUMENT = -1,
        XWALK_SCHEDULER_DUPLICATE_MAILBOX = -2,
        XWALK_SCHEDULER_UNKNOWN_MAILBOX = -3,
        XWALK_SCHEDULER_NO_PROCESS_SLOT = -4,
        XWALK_SCHEDULER_QUEUE_FULL = -5,
        XWALK_SCHEDULER_REQUEST_TABLE_FULL = -6,
        XWALK_SCHEDULER_LOCAL_INDEX_EXHAUSTED = -7,
        XWALK_SCHEDULER_INVALID_STATE = -8,
        XWALK_SCHEDULER_IPC_FAILURE = -9,
        XWALK_SCHEDULER_PROCESS_FAILURE = -10,
        XWALK_SCHEDULER_TIMEOUT = -11,
        XWALK_SCHEDULER_BUFFER_TOO_SMALL = -12,
        XWALK_SCHEDULER_NOT_FOUND = -13,
        XWALK_SCHEDULER_SHUTTING_DOWN = -14
    } xSchedulerResult;

    /** @brief Identifies the lifecycle of one registered child process. */
    typedef enum xProcessState
    {
        XWALK_PROCESS_UNUSED = 0,
        XWALK_PROCESS_CREATED,
        XWALK_PROCESS_STOPPED,
        XWALK_PROCESS_STARTING,
        XWALK_PROCESS_RUNNING,
        XWALK_PROCESS_STOPPING,
        XWALK_PROCESS_EXITED,
        XWALK_PROCESS_FAILED
    } xProcessState;

    /** @brief Identifies the independent lifecycle of one operational request. */
    typedef enum xRequestState
    {
        XWALK_REQUEST_UNUSED = 0,
        XWALK_REQUEST_PENDING,
        XWALK_REQUEST_RUNNING,
        XWALK_REQUEST_COMPLETED,
        XWALK_REQUEST_FAILED,
        XWALK_REQUEST_CANCELLED
    } xRequestState;

    /** @brief Identifies one parent-to-child scheduler command. */
    typedef enum xCommandType
    {
        XWALK_COMMAND_START = 0,
        XWALK_COMMAND_STOP,
        XWALK_COMMAND_CUSTOM,
        XWALK_COMMAND_GET_STATUS,
        XWALK_COMMAND_SHUTDOWN
    } xCommandType;

    /** @brief Identifies one message carried by the private scheduler socket. */
    typedef enum xSchedulerPacketType
    {
        XWALK_SCHEDULER_PACKET_COMMAND = 1,
        XWALK_SCHEDULER_PACKET_PROCESS_STATUS,
        XWALK_SCHEDULER_PACKET_REQUEST_STATUS,
        XWALK_SCHEDULER_PACKET_NESTED_SIGNAL
    } xSchedulerPacketType;

    /** @brief Identifies text commands and serialized Protobuf signal commands. */
    typedef enum xSchedulerDataType
    {
        XWALK_SCHEDULER_DATA_TEXT = 0,
        XWALK_SCHEDULER_DATA_SIGNAL
    } xSchedulerDataType;

    /**************************************************************************
     * Structure declarations
     **************************************************************************/

    typedef ::ctrl::int32 (*xModuleStartCallback)(::ctrl::contextpointer context);
    typedef ::ctrl::int32 (*xModuleStopCallback)(::ctrl::contextpointer context);
    typedef ::ctrl::int32 (*xModuleCommandCallback)(::ctrl::uint32 commandId,
                                                    ::ctrl::cstring commandData,
                                                    ::ctrl::contextpointer context);
    typedef ::ctrl::int32 (*xModuleTickCallback)(::ctrl::contextpointer context);
    typedef ::ctrl::boolean (*xSchedulerContinueCallback)(::ctrl::contextpointer contextStruct);

    /** @brief Groups C-style callbacks inherited by one scheduler child. */
    typedef struct xModuleCallbacks
    {
            xModuleStartCallback onStart;
            xModuleStopCallback onStop;
            xModuleCommandCallback onCommand;
            xModuleTickCallback onTick;
            ::ctrl::contextpointer context;
            xWalkSignalHandler_LPP onSignal;
    } xModuleCallbacks;

    /** @brief Describes one request retained for status correlation. */
    typedef struct xRequestStatus
    {
            xClientAddress address;
            ::ctrl::uint32 commandId;
            xRequestState state;
            ::ctrl::int32 result;
    } xRequestStatus;

    /** @brief Describes one scheduler-managed process and its latest event. */
    typedef struct xProcessStatus
    {
            ::ctrl::uint32 processSlot;
            ::ctrl::uint32 mailBoxId;
            ::ctrl::uint32 moduleType;
            pid_t processId;
            xProcessState state;
            xRequestStatus request;
            ::ctrl::int32 result;
            ::ctrl::size signalSize;
            ::ctrl::uint8 signalData[XWALK_SCHEDULER_SIGNAL_SIZE];
    } xProcessStatus;

    /** @brief Holds lifecycle masks and counts derived from those masks. */
    typedef struct xProcessStatusMasks
    {
            ::ctrl::uint64 registeredMask;
            ::ctrl::uint64 aliveMask;
            ::ctrl::uint64 createdMask;
            ::ctrl::uint64 stoppedMask;
            ::ctrl::uint64 startingMask;
            ::ctrl::uint64 runningMask;
            ::ctrl::uint64 stoppingMask;
            ::ctrl::uint64 exitedMask;
            ::ctrl::uint64 failedMask;
            ::ctrl::uint32 registeredCount;
            ::ctrl::uint32 aliveCount;
            ::ctrl::uint32 runningCount;
            ::ctrl::uint32 stoppedCount;
            ::ctrl::uint32 failedCount;
    } xProcessStatusMasks;

    /** @brief Stores one fixed-size operational command. */
    typedef struct xSchedulerCommand
    {
            xClientAddress address;
            ::ctrl::uint32 commandId;
            xSchedulerDataType dataType;
            ::ctrl::size dataSize;
            ::ctrl::uint8 commandData[XWALK_SCHEDULER_SIGNAL_SIZE];
    } xSchedulerCommand;

    /** @brief Stores one fixed-size, pointer-free private IPC packet. */
    typedef struct xSchedulerPacket
    {
            ::ctrl::uint32 protocolVersion;
            ::ctrl::uint32 messageSize;
            xSchedulerPacketType packetType;
            xCommandType commandType;
            xClientAddress address;
            ::ctrl::uint32 commandId;
            xProcessState processState;
            xRequestState requestState;
            ::ctrl::int32 result;
            xSchedulerDataType dataType;
            ::ctrl::size dataSize;
            ::ctrl::uint8 commandData[XWALK_SCHEDULER_SIGNAL_SIZE];
            ::ctrl::uint32 reserved[8U];
    } xSchedulerPacket;

    /** @brief Owns one stable process slot, FIFO, and retained request table. */
    typedef struct xSchedulerMailbox
    {
            ::ctrl::boolean registered;
            ::ctrl::uint32 processSlot;
            ::ctrl::uint32 mailBoxId;
            char clientAddress[XWALK_CLIENT_ADDRESS_SIZE];
            ::ctrl::uint32 moduleType;
            pid_t processId;
            ::ctrl::int32 parentSocket;
            ::ctrl::uint32 nextLocalIndex;
            xProcessState processState;
            ::ctrl::boolean dispatchEnabled;
            ::ctrl::boolean requestActive;
            xSchedulerCommand activeRequest;
            xSchedulerCommand pendingQueue[XWALK_MAX_PENDING_COMMANDS];
            ::ctrl::size queueHead;
            ::ctrl::size queueTail;
            ::ctrl::size queueCount;
            xRequestStatus requestRecords[XWALK_MAX_REQUEST_RECORDS];
            ::ctrl::uint64 requestSequence[XWALK_MAX_REQUEST_RECORDS];
            ::ctrl::uint64 nextRequestSequence;
            xProcessStatus lastStatus;
            xModuleCallbacks callbacks;
            ::ctrl::boolean shutdownRequested;
            ::ctrl::boolean cleanupComplete;
    } xSchedulerMailbox;

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_SCHEDULER_TYPES_H */
