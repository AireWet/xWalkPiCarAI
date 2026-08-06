/******************************************************************************
 * @file        xAgent_Rpi5CarAppControlWebSocketState.cpp
 * @brief       Implements bounded SunFounder WebSocket transport state.
 * @project     xWalk Firmware
 * @module      xWalkAppControl
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarAppControlWebSocketState.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <chrono>
#include <sstream>

namespace xwalk::agent
{

XWalkAppControlWebSocketState::XWalkAppControlWebSocketState(
    agent::string address): bindAddress(std::move(address))
{
}

XWalkAppControlWebSocketState::~XWalkAppControlWebSocketState() noexcept
{
    running.store(false);
    const agent::boolean workerJoinable = worker.joinable();
    if (workerJoinable)
    {
        worker.join();
    }
}

agent::string XWalkAppControlWebSocketState::escaped(agent::stringview value)
{
    agent::string result;
    result.reserve(value.size());
    for (const char character : value)
    {
        if ((character == '\\') || (character == '"'))
        {
            result.push_back('\\');
        }
        if ((character >= 0x20) && (character != 0x7F))
        {
            result.push_back(character);
        }
    }
    return result;
}

void XWalkAppControlWebSocketState::parse(const agent::string& message)
{
    try
    {
        std::istringstream stream(message);
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(stream, tree);
        XWalkAppControlInput next;
        {
            const std::lock_guard<std::mutex> lock(mutex);
            next = input;
        }
        next.hornRequested = tree.get("M", false);
        next.lineTrackingEnabled = tree.get("I", false);
        next.obstacleAvoidanceEnabled = tree.get("E", false);
        next.colorDetectionEnabled = tree.get("N", false);
        next.faceDetectionEnabled = tree.get("O", false);
        next.objectDetectionEnabled = tree.get("P", false);
        next.driveJoystickAvailable = false;
        next.cameraJoystickAvailable = false;
        const auto speech = tree.get_optional<agent::string>("J");
        if (speech)
        {
            next.spokenCommand = *speech;
        }
        const auto driveJoystick = tree.get_child_optional("K");
        if (driveJoystick)
        {
            auto value = driveJoystick->begin();
            const agent::boolean driveXAvailable = value != driveJoystick->end();
            if (driveXAvailable)
            {
                next.driveX = value->second.get_value<agent::float64>();
                ++value;
                const agent::boolean driveYAvailable = value != driveJoystick->end();
                if (driveYAvailable)
                {
                    next.driveY = value->second.get_value<agent::float64>();
                    next.driveJoystickAvailable = true;
                }
            }
        }
        const auto cameraJoystick = tree.get_child_optional("Q");
        if (cameraJoystick)
        {
            auto value = cameraJoystick->begin();
            const agent::boolean cameraPanAvailable = value != cameraJoystick->end();
            if (cameraPanAvailable)
            {
                next.cameraPanDegrees = value->second.get_value<agent::float64>();
                ++value;
                const agent::boolean cameraTiltAvailable = value != cameraJoystick->end();
                if (cameraTiltAvailable)
                {
                    next.cameraTiltDegrees = value->second.get_value<agent::float64>();
                    next.cameraJoystickAvailable = true;
                }
            }
        }
        const std::lock_guard<std::mutex> lock(mutex);
        input = next;
    }
    catch (...)
    {
        /* Malformed or oversized-type input leaves the last valid state intact. */
    }
}

agent::string XWalkAppControlWebSocketState::response() const
{
    const std::lock_guard<std::mutex> lock(mutex);
    std::ostringstream stream;
    stream << "{\"Name\":\"" << escaped(name) << "\",\"Type\":\""
        << escaped(type) << "\",\"Check\":\"SunFounder Controller\",\"A\":"
        << telemetry.speedPercent << ",\"D\":[" << telemetry.grayscale[0U]
        << ',' << telemetry.grayscale[1U] << ',' << telemetry.grayscale[2U]
        << "],\"F\":" << telemetry.distanceCm << ",\"video\":\""
        << escaped(telemetry.videoUrl) << "\",\"Heart\":\"pong\"}";
    return stream.str();
}

void XWalkAppControlWebSocketState::run(agent::uint16 port) noexcept
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    using tcp = asio::ip::tcp;
    try
    {
        asio::io_context context;
        const asio::ip::address address = asio::ip::make_address(bindAddress);
        tcp::acceptor acceptor(context, tcp::endpoint(address, port));
        acceptor.non_blocking(true);
        startupSucceeded.store(true);
        startupComplete.store(true);
        const agent::boolean serverLoopRequested{true};
        while (serverLoopRequested)
        {
            const agent::boolean serverRunning = running.load();
            if (serverRunning == false)
            {
                break;
            }
            tcp::socket socket(context);
            boost::system::error_code error;
            acceptor.accept(socket, error);
            if (error == asio::error::would_block)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }
            if (error)
            {
                break;
            }
            beast::tcp_stream stream(std::move(socket));
            stream.expires_after(std::chrono::seconds(2));
            websocket::stream<beast::tcp_stream> connection(std::move(stream));
            websocket::stream_base::timeout timeout =
                websocket::stream_base::timeout::suggested(beast::role_type::server);
            timeout.handshake_timeout = std::chrono::seconds(2);
            timeout.idle_timeout = std::chrono::milliseconds(250);
            connection.set_option(timeout);
            connection.read_message_max(65'536U);
            connection.accept(error);
            if (!error)
            {
                connection.next_layer().expires_never();
                connection.next_layer().socket().non_blocking(true, error);
            }
            beast::flat_buffer buffer;
            const agent::boolean connectionLoopRequested{true};
            while (connectionLoopRequested)
            {
                const agent::boolean connectionRunning = running.load() && !error;
                if (connectionRunning == false)
                {
                    break;
                }
                connection.read(buffer, error);
                if ((error == asio::error::would_block) ||
                    (error == asio::error::try_again))
                {
                    error.clear();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                else if (!error)
                {
                    parse(beast::buffers_to_string(buffer.data()));
                    buffer.consume(buffer.size());
                    connection.text(true);
                    connection.write(asio::buffer(response()), error);
                }
            }
        }
    }
    catch (...)
    {
        /* The foreground coordinator observes shutdown through cancellation. */
        startupComplete.store(true);
    }
    running.store(false);
}

} /* namespace xwalk::agent */
