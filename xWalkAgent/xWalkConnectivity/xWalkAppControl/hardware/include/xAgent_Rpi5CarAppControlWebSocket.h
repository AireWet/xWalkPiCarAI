/******************************************************************************
 * @file        xAgent_Rpi5CarAppControlWebSocket.h
 * @brief       Declares the SunFounder-compatible WebSocket transport.
 * @project     xWalk Firmware
 * @module      xWalkAppControl
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_APP_CONTROL_WEB_SOCKET_H
#define XAGENT_RPI5CAR_APP_CONTROL_WEB_SOCKET_H

#include "xAgent_Rpi5CarAppControlTypes.h"

#include <memory>

namespace xwalk::agent
{

class XWalkAppControlWebSocketState;

/** @brief Owns one explicitly configured SunFounder WebSocket listener. */
class XWalkAppControlWebSocket final
{
private:
    std::unique_ptr<XWalkAppControlWebSocketState> state;

    static agent::boolean startCallback(agent::contextpointer context,
        agent::stringview name, agent::stringview type, agent::uint16 port);
    static void stopCallback(agent::contextpointer context) noexcept;
    static XWalkAppControlInput pollCallback(agent::contextpointer context);
    static void publishCallback(agent::contextpointer context,
        const XWalkAppControlTelemetry& telemetry);

public:
    explicit XWalkAppControlWebSocket(agent::string bindAddress);
    ~XWalkAppControlWebSocket() noexcept;

    XWalkAppControlWebSocket(const XWalkAppControlWebSocket&) = delete;
    XWalkAppControlWebSocket(XWalkAppControlWebSocket&&) = delete;
    XWalkAppControlWebSocket& operator=(const XWalkAppControlWebSocket&) = delete;
    XWalkAppControlWebSocket& operator=(XWalkAppControlWebSocket&&) = delete;

    XWalkAppControlCallbacks callbacks(
        const XWalkComputerVisionCallbacks& visionCallbacks) noexcept;
    agent::boolean start(agent::stringview name, agent::stringview type,
        agent::uint16 port);
    void stop() noexcept;
    XWalkAppControlInput poll() const;
    void publish(const XWalkAppControlTelemetry& telemetry);
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_APP_CONTROL_WEB_SOCKET_H */
