/******************************************************************************
 * @file        xAgent_Rpi5CarAppControlWebSocket.cpp
 * @brief       Implements the public SunFounder WebSocket transport facade.
 * @project     xWalk Firmware
 * @module      xWalkAppControl
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarAppControlWebSocket.h"

#include "xAgent_Rpi5CarAppControlWebSocketState.h"

#include <chrono>
#include <memory>
#include <thread>

namespace xwalk::agent
{

XWalkAppControlWebSocket::XWalkAppControlWebSocket(agent::string bindAddress):
    state(std::make_unique<XWalkAppControlWebSocketState>(std::move(bindAddress)))
{
}

XWalkAppControlWebSocket::~XWalkAppControlWebSocket() noexcept
{
    stop();
}

agent::boolean XWalkAppControlWebSocket::startCallback(agent::contextpointer context,
    agent::stringview name, agent::stringview type, agent::uint16 port)
{
    return static_cast<XWalkAppControlWebSocket*>(context)->start(name, type, port);
}

void XWalkAppControlWebSocket::stopCallback(agent::contextpointer context) noexcept
{
    static_cast<XWalkAppControlWebSocket*>(context)->stop();
}

XWalkAppControlInput XWalkAppControlWebSocket::pollCallback(
    agent::contextpointer context)
{
    return static_cast<XWalkAppControlWebSocket*>(context)->poll();
}

void XWalkAppControlWebSocket::publishCallback(agent::contextpointer context,
    const XWalkAppControlTelemetry& telemetry)
{
    static_cast<XWalkAppControlWebSocket*>(context)->publish(telemetry);
}

XWalkAppControlCallbacks XWalkAppControlWebSocket::callbacks(
    const XWalkComputerVisionCallbacks& visionCallbacks) noexcept
{
    return {this, &startCallback, &stopCallback, &pollCallback, &publishCallback,
        nullptr, visionCallbacks};
}

agent::boolean XWalkAppControlWebSocket::start(agent::stringview name,
    agent::stringview type, agent::uint16 port)
{
    const agent::boolean exchangeSucceeded =
        static_cast<agent::boolean>(
            state->running.exchange(true));
    if (exchangeSucceeded)
    {
        return true;
    }
    {
        const std::lock_guard<std::mutex> lock(state->mutex);
        state->name = name;
        state->type = type;
    }
    state->startupComplete.store(false);
    state->startupSucceeded.store(false);
    const auto rollbackStart = [this](void*) noexcept
    {
        state->running.store(false);
    };
    std::unique_ptr<void, decltype(rollbackStart)> rollbackGuard(
        this, rollbackStart);
    state->worker = std::thread(&XWalkAppControlWebSocketState::run,
        state.get(), port);
    static_cast<void>(rollbackGuard.release());
    for (agent::uint32 attempt = 0U; attempt < 400U; ++attempt)
    {
        const agent::boolean startupComplete =
            static_cast<agent::boolean>(
                state->startupComplete.load());
        if (startupComplete)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    const agent::boolean startupFailed =
        static_cast<agent::boolean>(
            !state->startupSucceeded.load());
    if (startupFailed)
    {
        stop();
        return false;
    }
    return true;
}

void XWalkAppControlWebSocket::stop() noexcept
{
    state->running.store(false);
    const agent::boolean workerJoinable =
        static_cast<agent::boolean>(
            state->worker.joinable());
    if (workerJoinable)
    {
        state->worker.join();
    }
}

XWalkAppControlInput XWalkAppControlWebSocket::poll() const
{
    const std::lock_guard<std::mutex> lock(state->mutex);
    return state->input;
}

void XWalkAppControlWebSocket::publish(
    const XWalkAppControlTelemetry& telemetry)
{
    const std::lock_guard<std::mutex> lock(state->mutex);
    state->telemetry = telemetry;
}

} /* namespace xwalk::agent */
