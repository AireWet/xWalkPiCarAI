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
#include "xHal_Rpi5CarTrace.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <json-c/json.h>

#include <chrono>
#include <cmath>
#include <memory>
#include <sstream>

namespace
{

    using JsonObject = std::unique_ptr<json_object, decltype(&json_object_put)>;

    bool jsonBoolean(json_object* root, const char* name, bool defaultValue) noexcept
    {
        json_object* value{nullptr};
        if (!json_object_object_get_ex(root, name, &value) || (json_object_get_type(value) != json_type_boolean))
        {
            return defaultValue;
        }
        return json_object_get_boolean(value) != 0;
    }

    bool jsonPair(json_object* root, const char* name, double& first, double& second) noexcept
    {
        json_object* value{nullptr};
        if (!json_object_object_get_ex(root, name, &value) || (json_object_get_type(value) != json_type_array) ||
            (json_object_array_length(value) < 2U))
        {
            return false;
        }
        json_object* firstValue = json_object_array_get_idx(value, 0U);
        json_object* secondValue = json_object_array_get_idx(value, 1U);
        const bool valuesNumeric = (firstValue != nullptr) && (secondValue != nullptr) &&
                                   ((json_object_get_type(firstValue) == json_type_double) ||
                                    (json_object_get_type(firstValue) == json_type_int)) &&
                                   ((json_object_get_type(secondValue) == json_type_double) ||
                                    (json_object_get_type(secondValue) == json_type_int));
        if (!valuesNumeric)
        {
            return false;
        }
        first = json_object_get_double(firstValue);
        second = json_object_get_double(secondValue);
        return std::isfinite(first) && std::isfinite(second);
    }

} /* namespace */

namespace xwalk::agent
{

    XWalkAppControlWebSocketState::XWalkAppControlWebSocketState(agent::string address)
        : bindAddress(std::move(address))
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
        if (message.size() > 65'536U)
        {
            return;
        }
        json_tokener* const tokener = json_tokener_new();
        if (tokener == nullptr)
        {
            return;
        }
        json_object* parsed = json_tokener_parse_ex(tokener, message.data(), static_cast<int>(message.size()));
        const json_tokener_error parseError = json_tokener_get_error(tokener);
        json_tokener_free(tokener);
        JsonObject root(parsed, &json_object_put);
        if ((parseError != json_tokener_success) || (root == nullptr) ||
            (json_object_get_type(root.get()) != json_type_object))
        {
            return;
        }
        XWalkAppControlInput next;
        {
            const std::lock_guard<std::mutex> lock(mutex);
            next = input;
        }
        next.hornRequested = jsonBoolean(root.get(), "M", false);
        next.lineTrackingEnabled = jsonBoolean(root.get(), "I", false);
        next.obstacleAvoidanceEnabled = jsonBoolean(root.get(), "E", false);
        next.colorDetectionEnabled = jsonBoolean(root.get(), "N", false);
        next.faceDetectionEnabled = jsonBoolean(root.get(), "O", false);
        next.objectDetectionEnabled = jsonBoolean(root.get(), "P", false);
        next.driveJoystickAvailable = false;
        next.cameraJoystickAvailable = false;
        json_object* speech{nullptr};
        if (json_object_object_get_ex(root.get(), "J", &speech) && (json_object_get_type(speech) == json_type_string))
        {
            next.spokenCommand = json_object_get_string(speech);
        }
        next.driveJoystickAvailable = jsonPair(root.get(), "K", next.driveX, next.driveY);
        next.cameraJoystickAvailable = jsonPair(root.get(), "Q", next.cameraPanDegrees, next.cameraTiltDegrees);
        const std::lock_guard<std::mutex> lock(mutex);
        input = next;
    }

    agent::string XWalkAppControlWebSocketState::response() const
    {
        const std::lock_guard<std::mutex> lock(mutex);
        std::ostringstream stream;
        stream << "{\"Name\":\"" << escaped(name) << "\",\"Type\":\"" << escaped(type)
               << "\",\"Check\":\"SunFounder Controller\",\"A\":" << telemetry.speedPercent << ",\"D\":["
               << telemetry.grayscale[0U] << ',' << telemetry.grayscale[1U] << ',' << telemetry.grayscale[2U]
               << "],\"F\":" << telemetry.distanceCm << ",\"video\":\"" << escaped(telemetry.videoUrl)
               << "\",\"Heart\":\"pong\"}";
        return stream.str();
    }

    void XWalkAppControlWebSocketState::run(agent::uint16 port) noexcept
    {
        XWALK_RPIAGENT_TRACE_UID1(RPIAGENT .047, "App-control WebSocket event loop entered on port %u", port);
        namespace asio = boost::asio;
        namespace beast = boost::beast;
        namespace websocket = beast::websocket;
        using tcp = asio::ip::tcp;
        asio::io_context context;
        boost::system::error_code error;
        const asio::ip::address address = asio::ip::make_address(bindAddress, error);
        if (error)
        {
            startupComplete.store(true);
            running.store(false);
            return;
        }
        const tcp::endpoint endpoint(address, port);
        tcp::acceptor acceptor(context);
        acceptor.open(endpoint.protocol(), error);
        if (!error)
        {
            acceptor.bind(endpoint, error);
        }
        if (!error)
        {
            acceptor.listen(asio::socket_base::max_listen_connections, error);
        }
        if (!error)
        {
            acceptor.non_blocking(true, error);
        }
        if (error)
        {
            startupComplete.store(true);
            running.store(false);
            return;
        }
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
            acceptor.accept(socket, error);
            if (error == asio::error::would_block)
            {
                error.clear();
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
                if ((error == asio::error::would_block) || (error == asio::error::try_again))
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
        running.store(false);
    }

} /* namespace xwalk::agent */
