/******************************************************************************
 * @file        xControllerSchedulerRuntime.cpp
 * @brief       Implements the hidden module-facing scheduler runtime façade.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Scheduler
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#include "xControllerSchedulerRuntimeTypes.h"

#include "xControllerSchedulerSignal.h"
#include "xHal_Rpi5CarTrace.h"

#include <cstring>
#include <new>

namespace xwalk::ctrl
{

    static xSchedulerRuntime* runtime{nullptr};

    static ::ctrl::int32 cbbStart(::ctrl::contextpointer context) noexcept
    {
        return context == nullptr ? XWALK_SCHEDULER_INVALID_ARGUMENT : XWALK_SCHEDULER_OK;
    }

    static ::ctrl::int32 cbbStop(::ctrl::contextpointer context) noexcept
    {
        return context == nullptr ? XWALK_SCHEDULER_INVALID_ARGUMENT : XWALK_SCHEDULER_OK;
    }

    static ::ctrl::size moduleIndex(xWalkModuleId moduleId) noexcept
    {
        return moduleId == XWALK_MODULE_CTRL    ? 0U
               : moduleId == XWALK_MODULE_AGENT ? 1U
               : moduleId == XWALK_MODULE_HAL   ? 2U
                                                : 3U;
    }

    static ::ctrl::uint32 moduleMailbox(xWalkModuleId moduleId) noexcept
    {
        return moduleId == XWALK_MODULE_CTRL    ? XWALK_CTRL_MAILBOX_ID
               : moduleId == XWALK_MODULE_AGENT ? XWALK_AGENT_MAILBOX_ID
               : moduleId == XWALK_MODULE_HAL   ? XWALK_HAL_MAILBOX_ID
                                                : 0U;
    }

    static ::ctrl::cstring moduleAddress(xWalkModuleId moduleId) noexcept
    {
        return moduleId == XWALK_MODULE_CTRL    ? "xwalk-controller"
               : moduleId == XWALK_MODULE_AGENT ? "xwalk-agent"
               : moduleId == XWALK_MODULE_HAL   ? "xwalk-hal"
                                                : nullptr;
    }

    static ::ctrl::int32
    dispatchModule(xWalkModuleId moduleId, ::ctrl::contextpointer context, const XWalkSignal* signal) noexcept
    {
        auto* record = static_cast<xCbbModuleRecord*>(context);
        if ((record == nullptr) || !record->registered || (record->moduleId != moduleId) ||
            (record->handler == nullptr) || (signal == nullptr) || (signal->destination != moduleId))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        return record->handler(record->context, signal);
    }

    ::ctrl::int32 cxx_xWalkCtrlHandler_LPP(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept
    {
        return dispatchModule(XWALK_MODULE_CTRL, context, signal);
    }

    ::ctrl::int32 cxx_xWalkAgentHandler_LPP(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept
    {
        return dispatchModule(XWALK_MODULE_AGENT, context, signal);
    }

    ::ctrl::int32 cxx_xWalkHalHandler_LPP(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept
    {
        return dispatchModule(XWALK_MODULE_HAL, context, signal);
    }

    ::ctrl::int32 cxx_xWalkSchedulerRegister_LPP(xWalkModuleId moduleId,
                                                 ::ctrl::contextpointer context,
                                                 xWalkSignalHandler_LPP handler,
                                                 ::ctrl::uint32 moduleType) noexcept
    {
        const ::ctrl::size index = moduleIndex(moduleId);
        if ((index >= 3U) || (context == nullptr) || (handler == nullptr))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        if (runtime == nullptr)
        {
            runtime = new (std::nothrow) xSchedulerRuntime{};
            if (runtime == nullptr)
            {
                return XWALK_SCHEDULER_PROCESS_FAILURE;
            }
            static_cast<void>(cxx_xWalkBindScheduler_LPP(&runtime->scheduler));
        }
        xCbbModuleRecord& record = runtime->modules[index];
        if (record.registered)
        {
            return XWALK_SCHEDULER_DUPLICATE_MAILBOX;
        }
        record.registered = true;
        record.moduleId = moduleId;
        record.context = context;
        record.handler = handler;
        const xWalkSignalHandler_LPP schedulerHandler = moduleId == XWALK_MODULE_CTRL    ? &cxx_xWalkCtrlHandler_LPP
                                                        : moduleId == XWALK_MODULE_AGENT ? &cxx_xWalkAgentHandler_LPP
                                                                                         : &cxx_xWalkHalHandler_LPP;
        const xModuleCallbacks callbacks{&cbbStart, &cbbStop, nullptr, nullptr, &record, schedulerHandler};
        const ::ctrl::uint32 mailBoxId = moduleMailbox(moduleId);
        ::ctrl::int32 result = runtime->scheduler.addModule(mailBoxId, moduleAddress(moduleId), moduleType, &callbacks);
        if (result == XWALK_SCHEDULER_OK)
        {
            result = runtime->scheduler.startModule(mailBoxId);
        }
        if (result != XWALK_SCHEDULER_OK)
        {
            std::memset(&record, 0, sizeof(record));
        }
        return result;
    }

    ::ctrl::int32 cxx_xWalkSchedulerUnregister_LPP(xWalkModuleId moduleId) noexcept
    {
        const ::ctrl::size index = moduleIndex(moduleId);
        if (index >= 3U)
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        if ((runtime == nullptr) || !runtime->modules[index].registered)
        {
            return XWALK_SCHEDULER_OK;
        }
        const ::ctrl::int32 result = runtime->scheduler.shutdownModule(moduleMailbox(moduleId));
        std::memset(&runtime->modules[index], 0, sizeof(runtime->modules[index]));
        return result;
    }

    ::ctrl::int32 cxx_xWalkSchedulerSend_LPP(const XWalkSignal* signal, xClientAddress* requestAddress) noexcept
    {
        if ((signal == nullptr) || (runtime == nullptr))
        {
            return XWALK_SCHEDULER_INVALID_STATE;
        }
        return runtime->scheduler.submitSignal(signal->clientInfo.mailBoxId,
                                               signal->sigNo,
                                               signal->payload,
                                               signal->payloadSize,
                                               &signal->clientInfo,
                                               requestAddress);
    }

    ::ctrl::int32 cxx_xWalkCtrlInit_LPP(::ctrl::contextpointer context,
                                        xWalkSignalHandler_LPP handler,
                                        ::ctrl::uint32 moduleType) noexcept
    {
        return cxx_xWalkSchedulerRegister_LPP(XWALK_MODULE_CTRL, context, handler, moduleType);
    }

    ::ctrl::int32 cxx_xWalkAgentInit_LPP(::ctrl::contextpointer context,
                                         xWalkSignalHandler_LPP handler,
                                         ::ctrl::uint32 moduleType) noexcept
    {
        return cxx_xWalkSchedulerRegister_LPP(XWALK_MODULE_AGENT, context, handler, moduleType);
    }

    ::ctrl::int32 cxx_xWalkHalInit_LPP(::ctrl::contextpointer context,
                                       xWalkSignalHandler_LPP handler,
                                       ::ctrl::uint32 moduleType) noexcept
    {
        return cxx_xWalkSchedulerRegister_LPP(XWALK_MODULE_HAL, context, handler, moduleType);
    }

    ::ctrl::int32 cxx_xWalkCtrlShutdown_LPP() noexcept
    {
        return cxx_xWalkSchedulerUnregister_LPP(XWALK_MODULE_CTRL);
    }

    ::ctrl::int32 cxx_xWalkAgentShutdown_LPP() noexcept
    {
        return cxx_xWalkSchedulerUnregister_LPP(XWALK_MODULE_AGENT);
    }

    ::ctrl::int32 cxx_xWalkHalShutdown_LPP() noexcept
    {
        return cxx_xWalkSchedulerUnregister_LPP(XWALK_MODULE_HAL);
    }

    ::ctrl::int32 cxx_xWalkOpen_LPP(::ctrl::uint32 mailBoxId,
                                    ::ctrl::cstring clientAddress,
                                    ::ctrl::uint32 moduleType,
                                    const xModuleCallbacks* callbacks,
                                    xSchedulerContinueCallback continueCallback,
                                    ::ctrl::contextpointer continueContext) noexcept
    {
        if ((runtime != nullptr) || (callbacks == nullptr) || (continueCallback == nullptr))
        {
            return XWALK_SCHEDULER_INVALID_STATE;
        }
        runtime = new (std::nothrow) xSchedulerRuntime{};
        if (runtime == nullptr)
        {
            return XWALK_SCHEDULER_PROCESS_FAILURE;
        }
        runtime->mailBoxId = mailBoxId;
        runtime->continueCallback = continueCallback;
        runtime->continueContext = continueContext;
        ::ctrl::int32 result = runtime->scheduler.addModule(mailBoxId, clientAddress, moduleType, callbacks);
        if (result == XWALK_SCHEDULER_OK)
        {
            result = runtime->scheduler.startModule(mailBoxId);
        }

        xProcessStatus events[XWALK_MAX_STATUS_EVENTS]{};
        for (::ctrl::int32 elapsed = 0; (result == XWALK_SCHEDULER_OK) && (elapsed < 5000);
             elapsed += XWALK_SCHEDULER_POLL_SLICE_MS)
        {
            const ::ctrl::int32 count =
                runtime->scheduler.pollStatus(events, XWALK_MAX_STATUS_EVENTS, XWALK_SCHEDULER_POLL_SLICE_MS);
            if (count < 0)
            {
                result = count;
                break;
            }
            for (::ctrl::int32 index = 0; index < count; ++index)
            {
                if ((events[index].mailBoxId == mailBoxId) && (events[index].state == XWALK_PROCESS_RUNNING))
                {
                    result = cxx_xWalkBindScheduler_LPP(&runtime->scheduler);
                    return result;
                }
                if ((events[index].mailBoxId == mailBoxId) && (events[index].state == XWALK_PROCESS_FAILED))
                {
                    result = XWALK_SCHEDULER_PROCESS_FAILURE;
                }
            }
        }
        if (result == XWALK_SCHEDULER_OK)
        {
            result = XWALK_SCHEDULER_TIMEOUT;
        }
        cxx_xWalkClose_LPP();
        return result;
    }

    ::ctrl::int32
    cxx_xWalkWait_LPP(::ctrl::uint32 mailBoxId, ::ctrl::uint32 signalNumber, ::ctrl::int32* requestResult) noexcept
    {
        const ::ctrl::uint32 ctrlMailbox = moduleMailbox(XWALK_MODULE_CTRL);
        const ::ctrl::uint32 agentMailbox = moduleMailbox(XWALK_MODULE_AGENT);
        const ::ctrl::uint32 halMailbox = moduleMailbox(XWALK_MODULE_HAL);
        if ((runtime == nullptr) || (requestResult == nullptr) ||
            ((ctrlMailbox != mailBoxId) && (agentMailbox != mailBoxId) && (halMailbox != mailBoxId)))
        {
            return XWALK_SCHEDULER_INVALID_STATE;
        }
        xProcessStatus events[XWALK_MAX_STATUS_EVENTS]{};
        while (true)
        {
            const ::ctrl::int32 count =
                runtime->scheduler.pollStatus(events, XWALK_MAX_STATUS_EVENTS, XWALK_SCHEDULER_POLL_SLICE_MS);
            if (count < 0)
            {
                return count;
            }
            for (::ctrl::int32 index = 0; index < count; ++index)
            {
                const xRequestStatus& request = events[index].request;
                if ((request.address.mailBoxId == mailBoxId) && (request.commandId == signalNumber) &&
                    ((request.state == XWALK_REQUEST_COMPLETED) || (request.state == XWALK_REQUEST_FAILED)))
                {
                    *requestResult = request.result;
                    return XWALK_SCHEDULER_OK;
                }
                if ((events[index].mailBoxId == mailBoxId) &&
                    ((events[index].state == XWALK_PROCESS_FAILED) || (events[index].state == XWALK_PROCESS_EXITED)))
                {
                    return XWALK_SCHEDULER_PROCESS_FAILURE;
                }
            }
            const ::ctrl::boolean operationContinues =
                runtime->continueCallback == nullptr || runtime->continueCallback(runtime->continueContext);
            if (operationContinues == false)
            {
                static_cast<void>(runtime->scheduler.shutdownModule(mailBoxId));
            }
            xProcessStatusMasks masks{};
            const ::ctrl::int32 statusResult = runtime->scheduler.getProcessStatusMasks(&masks);
            if ((statusResult == XWALK_SCHEDULER_OK) && (masks.aliveMask == 0U))
            {
                return XWALK_SCHEDULER_PROCESS_FAILURE;
            }
        }
    }

    void cxx_xWalkClose_LPP() noexcept
    {
        if (runtime == nullptr)
        {
            return;
        }
        cxx_xWalkUnbindScheduler_LPP(&runtime->scheduler);
        runtime->scheduler.shutdownAll();
        delete runtime;
        runtime = nullptr;
    }

} /* namespace xwalk::ctrl */
