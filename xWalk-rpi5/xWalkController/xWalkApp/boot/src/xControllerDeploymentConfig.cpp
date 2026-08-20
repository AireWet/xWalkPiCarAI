/******************************************************************************
 * @file        xControllerDeploymentConfig.cpp
 * @brief       Implements device-free deployment configuration diagnostics.
 * @project     xWalk Firmware
 * @module      xWalkController Application
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xControllerDeploymentConfig.h"

#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarFileFunctions.h"

#include <array>
#include <charconv>
#include <cctype>
#include <cmath>
#include "xControllerDeploymentConfigTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using ConfigDefault = ::xwalk::source_types::xcontrollerdeploymentconfig::ConfigDefault;

namespace
{

    constexpr std::array<ConfigDefault, 78U> knownConfiguration{
        {{"deployment_config_version", "1"},
         {"hardware_board", "robot_hat_v4"},
         {"hardware_i2c_device", "/dev/i2c-1"},
         {"hardware_gpio_device", "/dev/gpiochip4"},
         {"hardware_spi_device", "/dev/spidev0.0"},
         {"hardware_gpio_minimum_line_count", "28"},
         {"hardware_v5_left_forward_pwm_channel", "P12"},
         {"hardware_v5_left_reverse_pwm_channel", "P13"},
         {"hardware_v5_right_forward_pwm_channel", "P14"},
         {"hardware_v5_right_reverse_pwm_channel", "P15"},
         {"hardware_v4_left_pwm_channel", "P13"},
         {"hardware_v4_right_pwm_channel", "P12"},
         {"hardware_v4_left_direction_pin", "D4"},
         {"hardware_v4_right_direction_pin", "D5"},
         {"picarx_dir_motor", "[1,1]"},
         {"picarx_dir_servo", "0"},
         {"picarx_cam_pan_servo", "0"},
         {"picarx_cam_tilt_servo", "0"},
         {"picarx_max_motor_output_percent", "20"},
         {"picarx_calibration_verified", "false"},
         {"picarx_apply_persisted_servo_positions", "false"},
         {"picarx_motor_watchdog_timeout_ms", "500"},
         {"camera_connection", "csi"},
         {"computer_vision_camera_backend", "v4l2"},
         {"computer_vision_camera_device", "/dev/video0"},
         {"computer_vision_width", "640"},
         {"computer_vision_height", "480"},
         {"computer_vision_read_timeout_ms", "1000"},
         {"video_recording_camera_backend", "v4l2"},
         {"video_recording_camera_device", "/dev/video0"},
         {"video_recording_fps", "20"},
         {"video_recording_read_timeout_ms", "1000"},
         {"video_stream_camera_backend", "v4l2"},
         {"video_stream_camera_device", "/dev/video0"},
         {"video_stream_width", "640"},
         {"video_stream_height", "480"},
         {"video_stream_jpeg_quality", "80"},
         {"video_stream_read_timeout_ms", "1000"},
         {"video_stream_bind_address", "127.0.0.1"},
         {"video_stream_port", "8080"},
         {"video_stream_maximum_clients", "4"},
         {"video_stream_queue_capacity", "2"},
         {"voice_language_model_provider", "ollama"},
         {"voice_language_model_endpoint", "http://127.0.0.1:11434/api/chat"},
         {"voice_language_model_api_key_environment", ""},
         {"voice_language_model_timeout_ms", "120000"},
         {"voice_capture_device", "default"},
         {"voice_playback_device", "default"},
         {"voice_mixer_device", "default"},
         {"voice_mixer_element", "PCM"},
         {"voice_piper_executable", "piper"},
         {"voice_vosk_endpoint_start_seconds", "0.5"},
         {"voice_vosk_endpoint_end_seconds", "1.0"},
         {"voice_vosk_endpoint_max_seconds", "15.0"},
         {"voice_vosk_silence_peak_threshold", "500"},
         {"voice_vosk_trace_transcript", "false"},
         {"voice_active_car_gpt_provider", "ollama"},
         {"voice_active_car_gpt_api_key_environment", ""},
         {"voice_active_car_gpt_endpoint", "http://127.0.0.1:11434/api/chat"},
         {"voice_active_car_gpt_model", "llama3.2:3b"},
         {"voice_active_car_gpt_timeout_ms", "120000"},
         {"voice_active_car_gpt_maximum_output_tokens", "256"},
         {"voice_active_car_gpt_maximum_messages", "20"},
         {"voice_active_car_gpt_piper_model", "/usr/share/xwalk/models/piper/en_GB-alan-medium.onnx"},
         {"voice_active_car_gpt_with_image", "false"},
         {"voice_active_car_gpt_continuous_conversation", "true"},
         {"voice_active_car_gpt_conversation_idle_timeout_ms", "30000"},
         {"voice_active_car_gpt_conversation_maximum_rounds", "10"},
         {"voice_active_car_gpt_conversation_maximum_misses", "3"},
         {"voice_active_car_gpt_sleep_phrases", "goodbye jarvis,go to sleep,stop listening"},
         {"voice_active_car_gpt_sleep_acknowledgement", "Going to sleep. Say hey Jarvis when you need me, Joxy."},
         {"voice_active_car_gpt_web_search_enabled", "true"},
         {"voice_active_car_gpt_web_search_endpoint", "http://127.0.0.1:8080/search"},
         {"voice_active_car_gpt_web_search_maximum_results", "3"},
         {"voice_active_car_gpt_web_search_timeout_ms", "5000"},
         {"voice_active_car_gpt_web_search_maximum_response_bytes", "262144"},
         {"app_control_bind_address", "127.0.0.1"},
         {"app_control_port", "8765"}}};

    ::ctrl::boolean parseUnsigned(::ctrl::stringview text, ::ctrl::uint32 minimum, ::ctrl::uint32 maximum) noexcept
    {
        if (text.empty())
        {
            return false;
        }
        ::ctrl::uint64 value{};
        for (const char character : text)
        {
            if ((character < '0') || (character > '9'))
            {
                return false;
            }
            value = (value * 10U) + static_cast<::ctrl::uint64>(character - '0');
            if (value > maximum)
            {
                return false;
            }
        }
        return (value >= minimum) && (value <= maximum);
    }

    ::ctrl::boolean parseFiniteRange(::ctrl::stringview text, ::ctrl::float64 minimum, ::ctrl::float64 maximum) noexcept
    {
        ::ctrl::float64 value{};
        const char* const begin = text.data();
        const char* const end = begin + text.size();
        const std::from_chars_result result = std::from_chars(begin, end, value);
        return (result.ec == std::errc{}) && (result.ptr == end) && std::isfinite(value) && (value >= minimum) &&
               (value <= maximum);
    }

    ::ctrl::boolean validPwmChannel(::ctrl::stringview value) noexcept
    {
        return (value.size() >= 2U) && (value[0U] == 'P') && parseUnsigned(value.substr(1U), 0U, 15U);
    }

    ::ctrl::boolean validBoolean(::ctrl::stringview value) noexcept
    {
        return (value == "true") || (value == "false");
    }

    ::ctrl::boolean validEnvironmentName(::ctrl::stringview value) noexcept
    {
        const ::ctrl::boolean valueEmpty = static_cast<::ctrl::boolean>(value.empty());
        if (valueEmpty)
        {
            return false;
        }
        const unsigned char first = static_cast<unsigned char>(value.front());
        const ::ctrl::boolean firstValid =
            static_cast<::ctrl::boolean>((std::isalpha(first) != 0) || (value.front() == '_'));
        if (firstValid == false)
        {
            return false;
        }
        for (const char character : value)
        {
            const unsigned char converted = static_cast<unsigned char>(character);
            const ::ctrl::boolean characterValid =
                static_cast<::ctrl::boolean>((std::isalnum(converted) != 0) || (character == '_'));
            if (characterValid == false)
            {
                return false;
            }
        }
        return true;
    }

    ::ctrl::boolean validPhraseList(::ctrl::stringview value) noexcept
    {
        if (value.empty())
        {
            return false;
        }
        ::ctrl::stringvector normalizedPhrases;
        ::ctrl::size offset{};
        while (offset <= value.size())
        {
            const ::ctrl::size comma = value.find(',', offset);
            const ::ctrl::size end = comma == ::ctrl::stringview::npos ? value.size() : comma;
            const ::ctrl::stringview phrase = value.substr(offset, end - offset);
            const ::ctrl::size first = phrase.find_first_not_of(" \t\r\n");
            const ::ctrl::size last = phrase.find_last_not_of(" \t\r\n");
            if ((first == ::ctrl::stringview::npos) || (last == ::ctrl::stringview::npos))
            {
                return false;
            }
            ::ctrl::string normalized(phrase.substr(first, (last - first) + 1U));
            for (char& character : normalized)
            {
                character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            }
            for (const ::ctrl::string& prior : normalizedPhrases)
            {
                if (prior == normalized)
                {
                    return false;
                }
            }
            normalizedPhrases.emplace_back(normalized);
            if (comma == ::ctrl::stringview::npos)
            {
                break;
            }
            offset = comma + 1U;
        }
        return true;
    }

    ::ctrl::boolean validMotorInversion(::ctrl::stringview value) noexcept
    {
        return (value == "[1,1]") || (value == "[-1,1]") || (value == "[1,-1]") || (value == "[-1,-1]");
    }

    ::ctrl::boolean validGpioPin(::ctrl::stringview value) noexcept
    {
        if ((value.size() >= 2U) && (value[0U] == 'D') && parseUnsigned(value.substr(1U), 0U, 16U))
        {
            return true;
        }
        return (value == "SW") || (value == "USER") || (value == "MCURST") || (value == "BOARD_TYPE") ||
               (value == "BLEINT") || (value == "RST") || (value == "LED") || (value == "BLERST") || (value == "CE");
    }

    ::ctrl::boolean validCameraBackend(::ctrl::stringview value) noexcept
    {
        return (value == "automatic") || (value == "v4l2") || (value == "gstreamer") || (value == "video_file") ||
               (value == "image_sequence");
    }

    ::ctrl::boolean validProvider(::ctrl::stringview value) noexcept
    {
        return (value == "ollama") || (value == "openai") || (value == "chatgpt") || (value == "gemini") ||
               (value == "grok") || (value == "xai") || (value == "anthropic") || (value == "claude") ||
               (value == "openai_compatible");
    }

    ::ctrl::boolean validEndpoint(::ctrl::stringview value) noexcept
    {
        const ::ctrl::boolean schemeValid = value.rfind("http://", 0U) == 0U || value.rfind("https://", 0U) == 0U;
        const ::ctrl::size schemeLength = value.rfind("https://", 0U) == 0U ? 8U : 7U;
        return schemeValid && (value.size() > schemeLength) && (value.find('@') == ::ctrl::stringview::npos) &&
               (value.find('\r') == ::ctrl::stringview::npos) && (value.find('\n') == ::ctrl::stringview::npos);
    }

    void appendCheck(xwalk::ctrl::XWalkDeploymentConfigReport& report,
                     ::ctrl::boolean passed,
                     ::ctrl::stringview name,
                     ::ctrl::stringview detail)
    {
        report.valid = report.valid && passed;
        report.lines.emplace_back(::ctrl::string(passed ? "[PASS] " : "[FAIL] ") + ::ctrl::string(name) + ": " +
                                  ::ctrl::string(detail));
    }

    ::ctrl::string value(const xwalk::hal::XWalkConfigStore& store, ::ctrl::stringview name)
    {
        for (const ConfigDefault& entry : knownConfiguration)
        {
            if (name == entry.name)
            {
                return store.get(entry.name, entry.value);
            }
        }
        return store.get(name, "");
    }

} /* namespace */

namespace xwalk::ctrl
{

    XWalkDeploymentConfigReport XWALK_validateDeploymentConfig(::ctrl::stringview configurationFilePath)
    {
        XWalkDeploymentConfigReport report{true, {"=== xWalk No-Hardware Configuration Validation ==="}};
        const ::ctrl::filesystempath path(configurationFilePath);
        const ::ctrl::boolean readable = path.is_absolute() && hal::isReadableRegularFile(path);
        appendCheck(report,
                    readable,
                    "Configuration manifest",
                    readable ? "absolute readable regular file" : "must be an absolute readable regular file");
        if (readable == false)
        {
            report.lines.emplace_back("[SIMULATED] No hardware device or external service was accessed");
            return report;
        }

        const hal::XWalkConfigStore store(configurationFilePath);
        appendCheck(
            report, value(store, "deployment_config_version") == "1", "Configuration schema", "supported version is 1");
        const ::ctrl::string board = value(store, "hardware_board");
        appendCheck(report,
                    (board == "auto") || (board == "robot_hat_v4") || (board == "robot_hat_v5"),
                    "Robot HAT revision",
                    board);

        for (const ::ctrl::cstring key : {"hardware_i2c_device", "hardware_gpio_device", "hardware_spi_device"})
        {
            const ::ctrl::string device = value(store, key);
            appendCheck(report,
                        ::ctrl::filesystempath(device).is_absolute(),
                        key,
                        ::ctrl::filesystempath(device).is_absolute() ? "absolute path" : "must be an absolute path");
        }
        appendCheck(report,
                    parseUnsigned(value(store, "hardware_gpio_minimum_line_count"), 1U, 1'024U),
                    "GPIO line count",
                    "range 1 through 1024");

        const std::array<::ctrl::cstring, 4U> v5Keys{{"hardware_v5_left_forward_pwm_channel",
                                                      "hardware_v5_left_reverse_pwm_channel",
                                                      "hardware_v5_right_forward_pwm_channel",
                                                      "hardware_v5_right_reverse_pwm_channel"}};
        std::array<::ctrl::string, 4U> v5Channels{};
        ::ctrl::boolean v5Valid = true;
        for (::ctrl::size index = 0U; index < v5Keys.size(); ++index)
        {
            v5Channels[index] = value(store, v5Keys[index]);
            v5Valid = v5Valid && validPwmChannel(v5Channels[index]);
            for (::ctrl::size prior = 0U; prior < index; ++prior)
            {
                v5Valid = v5Valid && (v5Channels[index] != v5Channels[prior]);
            }
        }
        appendCheck(report,
                    v5Valid,
                    "Robot HAT v5 motor mapping",
                    v5Valid ? "four unique P0-P15 channels" : "channels must be valid and unique");

        const ::ctrl::string v4Left = value(store, "hardware_v4_left_pwm_channel");
        const ::ctrl::string v4Right = value(store, "hardware_v4_right_pwm_channel");
        appendCheck(report,
                    validPwmChannel(v4Left) && validPwmChannel(v4Right) && (v4Left != v4Right),
                    "Robot HAT v4 motor mapping",
                    "two distinct P0-P15 channels");
        const ::ctrl::string v4LeftDirection = value(store, "hardware_v4_left_direction_pin");
        const ::ctrl::string v4RightDirection = value(store, "hardware_v4_right_direction_pin");
        appendCheck(report,
                    validGpioPin(v4LeftDirection) && validGpioPin(v4RightDirection) &&
                        (v4LeftDirection != v4RightDirection),
                    "Robot HAT v4 direction mapping",
                    "two distinct supported Robot HAT GPIO names");
        appendCheck(report,
                    validMotorInversion(value(store, "picarx_dir_motor")),
                    "Motor inversion",
                    "exactly two values, each -1 or 1");
        appendCheck(report,
                    parseUnsigned(value(store, "picarx_max_motor_output_percent"), 0U, 100U),
                    "Motor output limit",
                    "range 0 through 100 percent");
        appendCheck(report,
                    parseUnsigned(value(store, "picarx_motor_watchdog_timeout_ms"), 1U, 60'000U),
                    "Motor watchdog",
                    "range 1 through 60000 ms");
        for (const ::ctrl::cstring key : {"picarx_dir_servo", "picarx_cam_pan_servo", "picarx_cam_tilt_servo"})
        {
            appendCheck(report,
                        parseFiniteRange(value(store, key), -180.0, 180.0),
                        key,
                        "finite range -180 through 180 degrees");
        }
        appendCheck(report, validBoolean(value(store, "picarx_calibration_verified")), "Calibration gate", "boolean");
        appendCheck(report,
                    validBoolean(value(store, "picarx_apply_persisted_servo_positions")),
                    "Persisted servo commissioning gate",
                    "boolean");

        const ::ctrl::string connection = value(store, "camera_connection");
        appendCheck(report, (connection == "csi") || (connection == "usb"), "Still-camera connection", connection);
        for (const ::ctrl::cstring prefix : {"computer_vision", "video_recording", "video_stream"})
        {
            const ::ctrl::string backendKey = ::ctrl::string(prefix) + "_camera_backend";
            const ::ctrl::string deviceKey = ::ctrl::string(prefix) + "_camera_device";
            const ::ctrl::string backend = value(store, backendKey);
            const ::ctrl::string device = value(store, deviceKey);
            const ::ctrl::boolean sourceValid =
                backend == "gstreamer" ? !device.empty()
                                       : ((backend == "automatic") || ::ctrl::filesystempath(device).is_absolute());
            const ::ctrl::boolean streamingBackend = ::ctrl::stringview(prefix) == "video_stream";
            const ::ctrl::boolean streamingBackendValid = (streamingBackend == false) || (backend == "v4l2") ||
                                                          (backend == "gstreamer") || (backend == "automatic");
            appendCheck(report,
                        validCameraBackend(backend) && sourceValid && streamingBackendValid,
                        backendKey,
                        "supported backend with compatible source");
        }
        appendCheck(report,
                    parseUnsigned(value(store, "computer_vision_width"), 16U, 7'680U) &&
                        parseUnsigned(value(store, "computer_vision_height"), 16U, 4'320U),
                    "Vision resolution",
                    "bounded width and height");
        appendCheck(report,
                    parseUnsigned(value(store, "video_stream_width"), 16U, 7'680U) &&
                        parseUnsigned(value(store, "video_stream_height"), 16U, 4'320U),
                    "Streaming resolution",
                    "bounded width and height");
        appendCheck(report,
                    parseUnsigned(value(store, "video_stream_jpeg_quality"), 1U, 100U),
                    "Streaming JPEG quality",
                    "range 1 through 100");
        appendCheck(report,
                    value(store, "video_stream_bind_address") == "127.0.0.1" &&
                        parseUnsigned(value(store, "video_stream_port"), 1U, 65'535U) &&
                        parseUnsigned(value(store, "video_stream_maximum_clients"), 1U, 32U) &&
                        parseUnsigned(value(store, "video_stream_queue_capacity"), 1U, 16U),
                    "Streaming listener",
                    "loopback address and bounded listener settings");
        appendCheck(report,
                    parseUnsigned(value(store, "computer_vision_read_timeout_ms"), 1U, 60'000U) &&
                        parseUnsigned(value(store, "video_recording_read_timeout_ms"), 1U, 60'000U) &&
                        parseUnsigned(value(store, "video_stream_read_timeout_ms"), 1U, 60'000U),
                    "Camera read timeouts",
                    "range 1 through 60000 ms");
        appendCheck(report,
                    parseFiniteRange(value(store, "video_recording_fps"), 1.0, 120.0),
                    "Recording frame rate",
                    "finite range 1 through 120 fps");

        appendCheck(report,
                    validProvider(value(store, "voice_language_model_provider")),
                    "Language-model provider",
                    "supported local or OpenAI-compatible provider");
        appendCheck(report,
                    validEndpoint(value(store, "voice_language_model_endpoint")),
                    "Language-model endpoint",
                    "HTTP(S) URL without embedded credentials");
        appendCheck(report,
                    parseUnsigned(value(store, "voice_language_model_timeout_ms"), 1U, 600'000U),
                    "Language-model timeout",
                    "range 1 through 600000 ms");
        const ::ctrl::string captureDevice = value(store, "voice_capture_device");
        const ::ctrl::string playbackDevice = value(store, "voice_playback_device");
        const ::ctrl::string mixerDevice = value(store, "voice_mixer_device");
        const ::ctrl::string mixerElement = value(store, "voice_mixer_element");
        const ::ctrl::string piperExecutable = value(store, "voice_piper_executable");
        appendCheck(report,
                    !captureDevice.empty() && !playbackDevice.empty() && !mixerDevice.empty() &&
                        !mixerElement.empty() && !piperExecutable.empty(),
                    "Voice audio deployment",
                    "non-empty capture, playback, mixer, element, and Piper selections");
        const ::ctrl::string endpointStart = value(store, "voice_vosk_endpoint_start_seconds");
        const ::ctrl::string endpointEnd = value(store, "voice_vosk_endpoint_end_seconds");
        const ::ctrl::string endpointMaximum = value(store, "voice_vosk_endpoint_max_seconds");
        ::ctrl::float64 endpointStartValue{};
        ::ctrl::float64 endpointEndValue{};
        ::ctrl::float64 endpointMaximumValue{};
        const auto parseEndpoint = [](::ctrl::stringview text, ::ctrl::float64& result) noexcept -> ::ctrl::boolean
        {
            const char* const begin = text.data();
            const char* const end = begin + text.size();
            const std::from_chars_result conversion = std::from_chars(begin, end, result);
            return (conversion.ec == std::errc{}) && (conversion.ptr == end) && std::isfinite(result) &&
                   (result > 0.0) && (result <= 30.0);
        };
        const ::ctrl::boolean endpointValuesValid = parseEndpoint(endpointStart, endpointStartValue) &&
                                                    parseEndpoint(endpointEnd, endpointEndValue) &&
                                                    parseEndpoint(endpointMaximum, endpointMaximumValue);
        appendCheck(report,
                    endpointValuesValid && (endpointStartValue <= endpointMaximumValue) &&
                        (endpointEndValue <= endpointMaximumValue),
                    "Vosk streaming endpoint timing",
                    "positive ordered seconds bounded by the 30-second hard timeout");
        appendCheck(report,
                    parseUnsigned(value(store, "voice_vosk_silence_peak_threshold"), 1U, 32'767U),
                    "Vosk fallback silence threshold",
                    "signed 16-bit PCM peak range 1 through 32767");
        appendCheck(report,
                    validBoolean(value(store, "voice_vosk_trace_transcript")),
                    "Vosk transcript diagnostic",
                    "strict boolean; disabled by default for privacy");
        appendCheck(report,
                    parseUnsigned(value(store, "voice_active_car_gpt_maximum_output_tokens"), 1U, 16'384U),
                    "Jarvis spoken-output token bound",
                    "range 1 through 16384 tokens");
        const ::ctrl::string jarvisEnvironment = value(store, "voice_active_car_gpt_api_key_environment");
        const ::ctrl::string jarvisProvider = value(store, "voice_active_car_gpt_provider");
        const ::ctrl::string jarvisEndpoint = value(store, "voice_active_car_gpt_endpoint");
        const ::ctrl::string jarvisModel = value(store, "voice_active_car_gpt_model");
        const ::ctrl::boolean jarvisEndpointHttps = jarvisEndpoint.rfind("https://", 0U) == 0U;
        const ::ctrl::boolean jarvisOllama = jarvisProvider == "ollama";
        const ::ctrl::boolean jarvisGemini = jarvisProvider == "gemini";
        const ::ctrl::boolean jarvisCloud = jarvisGemini || (jarvisProvider == "openai") ||
                                            (jarvisProvider == "chatgpt") || (jarvisProvider == "grok") ||
                                            (jarvisProvider == "xai") || (jarvisProvider == "claude") ||
                                            (jarvisProvider == "anthropic") || (jarvisProvider == "openai_compatible");
        const ::ctrl::boolean jarvisLoopback = (jarvisEndpoint.rfind("http://127.0.0.1:", 0U) == 0U) ||
                                               (jarvisEndpoint.rfind("http://localhost:", 0U) == 0U) ||
                                               (jarvisEndpoint.rfind("http://[::1]:", 0U) == 0U);
        const ::ctrl::boolean jarvisEndpointSuffix =
            jarvisOllama
                ? (jarvisEndpoint.size() >= 9U && jarvisEndpoint.substr(jarvisEndpoint.size() - 9U) == "/api/chat")
                : (jarvisEndpoint.size() >= 17U &&
                   jarvisEndpoint.substr(jarvisEndpoint.size() - 17U) == "/chat/completions");
        appendCheck(report,
                    (jarvisOllama || jarvisCloud) && !jarvisModel.empty() && validEndpoint(jarvisEndpoint) &&
                        jarvisEndpointSuffix &&
                        ((jarvisOllama && jarvisLoopback && jarvisEnvironment.empty()) ||
                         (jarvisCloud && jarvisEndpointHttps && validEnvironmentName(jarvisEnvironment) &&
                          (!jarvisGemini || (jarvisEnvironment == "GEMINI_API_KEY")))),
                    "Jarvis language-model provider",
                    "loopback credential-free Ollama or credentialed HTTPS Gemini");
        appendCheck(report,
                    parseUnsigned(value(store, "voice_active_car_gpt_timeout_ms"), 1U, 300'000U) &&
                        parseUnsigned(value(store, "voice_active_car_gpt_maximum_messages"), 1U, 200U),
                    "Jarvis model bounds",
                    "timeout 1-300000 ms and retained messages 1-200");
        appendCheck(report,
                    validBoolean(value(store, "voice_active_car_gpt_with_image")) &&
                        (value(store, "voice_active_car_gpt_with_image") == "false") &&
                        validBoolean(value(store, "voice_active_car_gpt_continuous_conversation")),
                    "Jarvis feature gates",
                    "camera must be false and continuous conversation must be a strict boolean");
        appendCheck(report,
                    parseUnsigned(value(store, "voice_active_car_gpt_conversation_idle_timeout_ms"), 1U, 300'000U),
                    "Jarvis conversation idle timeout",
                    "range 1 through 300000 ms");
        appendCheck(report,
                    parseUnsigned(value(store, "voice_active_car_gpt_conversation_maximum_rounds"), 1U, 100U) &&
                        parseUnsigned(value(store, "voice_active_car_gpt_conversation_maximum_misses"), 1U, 10U),
                    "Jarvis conversation limits",
                    "1-100 rounds and 1-10 consecutive misses");
        appendCheck(report,
                    validPhraseList(value(store, "voice_active_car_gpt_sleep_phrases")),
                    "Jarvis sleep phrases",
                    "comma-separated trimmed non-empty phrases");
        const ::ctrl::string searchEndpoint = value(store, "voice_active_car_gpt_web_search_endpoint");
        const ::ctrl::boolean searchLoopback = (searchEndpoint.rfind("http://127.0.0.1:", 0U) == 0U) ||
                                               (searchEndpoint.rfind("http://localhost:", 0U) == 0U) ||
                                               (searchEndpoint.rfind("http://[::1]:", 0U) == 0U);
        const ::ctrl::boolean searchSuffix =
            searchEndpoint.size() >= 7U && searchEndpoint.substr(searchEndpoint.size() - 7U) == "/search";
        appendCheck(report,
                    validBoolean(value(store, "voice_active_car_gpt_web_search_enabled")) && searchLoopback &&
                        searchSuffix &&
                        parseUnsigned(value(store, "voice_active_car_gpt_web_search_maximum_results"), 1U, 10U) &&
                        parseUnsigned(value(store, "voice_active_car_gpt_web_search_timeout_ms"), 1U, 30'000U) &&
                        parseUnsigned(
                            value(store, "voice_active_car_gpt_web_search_maximum_response_bytes"), 1'024U, 1'048'576U),
                    "Jarvis local web retrieval",
                    "strict gate, loopback /search endpoint, and bounded result, timeout, and response limits");
        appendCheck(report,
                    parseUnsigned(value(store, "app_control_port"), 1U, 65'535U),
                    "App-control port",
                    "range 1 through 65535");
        appendCheck(report,
                    !value(store, "app_control_bind_address").empty(),
                    "App-control bind address",
                    "non-empty; localhost is recommended");
        report.lines.emplace_back("[SIMULATED] No hardware device or external service was accessed");
        return report;
    }

    ::ctrl::stringvector XWALK_effectiveDeploymentConfig(::ctrl::stringview configurationFilePath)
    {
        const hal::XWalkConfigStore store(configurationFilePath);
        ::ctrl::stringvector lines{"=== xWalk Sanitized Effective Configuration ==="};
        lines.reserve(knownConfiguration.size() + 2U);
        for (const ConfigDefault& entry : knownConfiguration)
        {
            const ::ctrl::string effectiveValue = store.get(entry.name, entry.value);
            const ::ctrl::stringview key(entry.name);
            const ::ctrl::boolean secretShaped = key.find("api_key") != ::ctrl::stringview::npos ||
                                                 key.find("secret") != ::ctrl::stringview::npos ||
                                                 ((key.find("token") != ::ctrl::stringview::npos) &&
                                                  (key.find("maximum_output_tokens") == ::ctrl::stringview::npos));
            lines.emplace_back(::ctrl::string(entry.name) + " = " +
                               (secretShaped && !effectiveValue.empty() ? "<redacted>" : effectiveValue));
        }
        lines.emplace_back("[SIMULATED] Known layered values only; no hardware was accessed");
        return lines;
    }

} /* namespace xwalk::ctrl */
