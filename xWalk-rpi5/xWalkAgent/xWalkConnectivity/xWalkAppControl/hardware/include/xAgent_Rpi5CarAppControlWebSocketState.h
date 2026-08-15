/******************************************************************************
 * @file        xAgent_Rpi5CarAppControlWebSocketState.h
 * @brief       Declares private WebSocket transport state.
 * @project     xWalk Firmware
 * @module      xWalkAppControl
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_APP_CONTROL_WEB_SOCKET_STATE_H
#define XAGENT_RPI5CAR_APP_CONTROL_WEB_SOCKET_STATE_H

#include "xAgent_Rpi5CarAppControlTypes.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace xwalk::agent
{

    /** @brief Stores synchronized state owned by one WebSocket provider. */
    class XWalkAppControlWebSocketState final
    {
        public:
            agent::string bindAddress;
            agent::string name;
            agent::string type;
            XWalkAppControlInput input{};
            XWalkAppControlTelemetry telemetry{};
            mutable std::mutex mutex{};
            std::atomic<agent::boolean> running{false};
            std::atomic<agent::boolean> startupComplete{false};
            std::atomic<agent::boolean> startupSucceeded{false};
            std::thread worker{};

            explicit XWalkAppControlWebSocketState(agent::string address);
            ~XWalkAppControlWebSocketState() noexcept;
            void run(agent::uint16 port) noexcept;
            void parse(const agent::string& message);
            agent::string response() const;
            static agent::string escaped(agent::stringview value);
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_APP_CONTROL_WEB_SOCKET_STATE_H */
