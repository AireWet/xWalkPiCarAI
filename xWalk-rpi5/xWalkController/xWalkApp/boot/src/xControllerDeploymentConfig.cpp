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
#include <cmath>

namespace
{

struct ConfigDefault
{
    ::ctrl::cstring name;
    ::ctrl::cstring value;
};

constexpr std::array<ConfigDefault, 38U> knownConfiguration{{
    {"deployment_config_version", "1"},
    {"hardware_board", "auto"},
    {"hardware_i2c_device", "/dev/i2c-1"},
    {"hardware_gpio_device", "/dev/gpiochip0"},
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
    {"voice_language_model_provider", "ollama"},
    {"voice_language_model_endpoint", "http://127.0.0.1:11434/api/chat"},
    {"voice_language_model_api_key_environment", ""},
    {"voice_language_model_timeout_ms", "120000"},
    {"app_control_bind_address", "127.0.0.1"},
    {"app_control_port", "8765"}
}};

::ctrl::boolean parseUnsigned(::ctrl::stringview text, ::ctrl::uint32 minimum,
    ::ctrl::uint32 maximum) noexcept
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

::ctrl::boolean parseFiniteRange(::ctrl::stringview text, ::ctrl::float64 minimum,
    ::ctrl::float64 maximum) noexcept
{
    ::ctrl::float64 value{};
    const char* const begin = text.data();
    const char* const end = begin + text.size();
    const std::from_chars_result result = std::from_chars(begin, end, value);
    return (result.ec == std::errc{}) && (result.ptr == end) &&
        std::isfinite(value) && (value >= minimum) && (value <= maximum);
}

::ctrl::boolean validPwmChannel(::ctrl::stringview value) noexcept
{
    return (value.size() >= 2U) && (value[0U] == 'P') &&
        parseUnsigned(value.substr(1U), 0U, 15U);
}

::ctrl::boolean validBoolean(::ctrl::stringview value) noexcept
{
    return (value == "true") || (value == "false");
}

::ctrl::boolean validMotorInversion(::ctrl::stringview value) noexcept
{
    return (value == "[1,1]") || (value == "[-1,1]") ||
        (value == "[1,-1]") || (value == "[-1,-1]");
}

::ctrl::boolean validGpioPin(::ctrl::stringview value) noexcept
{
    if ((value.size() >= 2U) && (value[0U] == 'D') &&
        parseUnsigned(value.substr(1U), 0U, 16U))
    {
        return true;
    }
    return (value == "SW") || (value == "USER") || (value == "MCURST") ||
        (value == "BOARD_TYPE") || (value == "BLEINT") || (value == "RST") ||
        (value == "LED") || (value == "BLERST") || (value == "CE");
}

::ctrl::boolean validCameraBackend(::ctrl::stringview value) noexcept
{
    return (value == "automatic") || (value == "v4l2") ||
        (value == "gstreamer") || (value == "video_file") ||
        (value == "image_sequence");
}

::ctrl::boolean validProvider(::ctrl::stringview value) noexcept
{
    return (value == "ollama") || (value == "openai") ||
        (value == "chatgpt") || (value == "gemini") ||
        (value == "grok") || (value == "xai") ||
        (value == "anthropic") || (value == "claude") ||
        (value == "openai_compatible");
}

::ctrl::boolean validEndpoint(::ctrl::stringview value) noexcept
{
    const ::ctrl::boolean schemeValid = value.rfind("http://", 0U) == 0U ||
        value.rfind("https://", 0U) == 0U;
    const ::ctrl::size schemeLength = value.rfind("https://", 0U) == 0U ? 8U : 7U;
    return schemeValid && (value.size() > schemeLength) &&
        (value.find('@') == ::ctrl::stringview::npos) &&
        (value.find('\r') == ::ctrl::stringview::npos) &&
        (value.find('\n') == ::ctrl::stringview::npos);
}

void appendCheck(xwalk::ctrl::XWalkDeploymentConfigReport& report,
    ::ctrl::boolean passed, ::ctrl::stringview name, ::ctrl::stringview detail)
{
    report.valid = report.valid && passed;
    report.lines.emplace_back(::ctrl::string(passed ? "[PASS] " : "[FAIL] ") +
        ::ctrl::string(name) + ": " + ::ctrl::string(detail));
}

::ctrl::string value(const xwalk::hal::XWalkConfigStore& store,
    ::ctrl::stringview name)
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

XWalkDeploymentConfigReport XWALK_validateDeploymentConfig(
    ::ctrl::stringview configurationFilePath)
{
    XWalkDeploymentConfigReport report{true,
        {"=== xWalk No-Hardware Configuration Validation ==="}};
    const ::ctrl::filesystempath path(configurationFilePath);
    const ::ctrl::boolean readable = path.is_absolute() &&
        hal::isReadableRegularFile(path);
    appendCheck(report, readable, "Configuration manifest",
        readable ? "absolute readable regular file" :
            "must be an absolute readable regular file");
    if (readable == false)
    {
        report.lines.emplace_back(
            "[SIMULATED] No hardware device or external service was accessed");
        return report;
    }

    const hal::XWalkConfigStore store(configurationFilePath);
        appendCheck(report, value(store, "deployment_config_version") == "1",
            "Configuration schema", "supported version is 1");
        const ::ctrl::string board = value(store, "hardware_board");
        appendCheck(report, (board == "auto") || (board == "robot_hat_v4") ||
            (board == "robot_hat_v5"), "Robot HAT revision", board);

        for (const ::ctrl::cstring key : {"hardware_i2c_device",
            "hardware_gpio_device", "hardware_spi_device"})
        {
            const ::ctrl::string device = value(store, key);
            appendCheck(report, ::ctrl::filesystempath(device).is_absolute(), key,
                ::ctrl::filesystempath(device).is_absolute() ? "absolute path" :
                    "must be an absolute path");
        }
        appendCheck(report, parseUnsigned(value(store,
            "hardware_gpio_minimum_line_count"), 1U, 1'024U),
            "GPIO line count", "range 1 through 1024");

        const std::array<::ctrl::cstring, 4U> v5Keys{{
            "hardware_v5_left_forward_pwm_channel",
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
        appendCheck(report, v5Valid, "Robot HAT v5 motor mapping",
            v5Valid ? "four unique P0-P15 channels" : "channels must be valid and unique");

        const ::ctrl::string v4Left = value(store, "hardware_v4_left_pwm_channel");
        const ::ctrl::string v4Right = value(store, "hardware_v4_right_pwm_channel");
        appendCheck(report, validPwmChannel(v4Left) && validPwmChannel(v4Right) &&
            (v4Left != v4Right), "Robot HAT v4 motor mapping",
            "two distinct P0-P15 channels");
        const ::ctrl::string v4LeftDirection = value(store,
            "hardware_v4_left_direction_pin");
        const ::ctrl::string v4RightDirection = value(store,
            "hardware_v4_right_direction_pin");
        appendCheck(report, validGpioPin(v4LeftDirection) &&
            validGpioPin(v4RightDirection) &&
            (v4LeftDirection != v4RightDirection),
            "Robot HAT v4 direction mapping",
            "two distinct supported Robot HAT GPIO names");
        appendCheck(report, validMotorInversion(value(store, "picarx_dir_motor")),
            "Motor inversion", "exactly two values, each -1 or 1");
        appendCheck(report, parseUnsigned(value(store,
            "picarx_max_motor_output_percent"), 0U, 100U),
            "Motor output limit", "range 0 through 100 percent");
        appendCheck(report, parseUnsigned(value(store,
            "picarx_motor_watchdog_timeout_ms"), 1U, 60'000U),
            "Motor watchdog", "range 1 through 60000 ms");
        for (const ::ctrl::cstring key : {"picarx_dir_servo",
            "picarx_cam_pan_servo", "picarx_cam_tilt_servo"})
        {
            appendCheck(report, parseFiniteRange(value(store, key), -180.0, 180.0),
                key, "finite range -180 through 180 degrees");
        }
        appendCheck(report, validBoolean(value(store, "picarx_calibration_verified")),
            "Calibration gate", "boolean");
        appendCheck(report, validBoolean(value(store,
            "picarx_apply_persisted_servo_positions")),
            "Persisted servo commissioning gate", "boolean");

        const ::ctrl::string connection = value(store, "camera_connection");
        appendCheck(report, (connection == "csi") || (connection == "usb"),
            "Still-camera connection", connection);
        for (const ::ctrl::cstring prefix : {"computer_vision", "video_recording"})
        {
            const ::ctrl::string backendKey = ::ctrl::string(prefix) + "_camera_backend";
            const ::ctrl::string deviceKey = ::ctrl::string(prefix) + "_camera_device";
            const ::ctrl::string backend = value(store, backendKey);
            const ::ctrl::string device = value(store, deviceKey);
            const ::ctrl::boolean sourceValid = backend == "gstreamer" ? !device.empty() :
                ((backend == "automatic") || ::ctrl::filesystempath(device).is_absolute());
            appendCheck(report, validCameraBackend(backend) && sourceValid,
                backendKey, "supported backend with compatible source");
        }
        appendCheck(report, parseUnsigned(value(store, "computer_vision_width"),
            16U, 7'680U) && parseUnsigned(value(store, "computer_vision_height"),
            16U, 4'320U), "Vision resolution", "bounded width and height");
        appendCheck(report, parseUnsigned(value(store,
            "computer_vision_read_timeout_ms"), 1U, 60'000U) &&
            parseUnsigned(value(store, "video_recording_read_timeout_ms"),
                1U, 60'000U), "Camera read timeouts", "range 1 through 60000 ms");
        appendCheck(report, parseFiniteRange(value(store, "video_recording_fps"),
            1.0, 120.0), "Recording frame rate", "finite range 1 through 120 fps");

        appendCheck(report, validProvider(value(store,
            "voice_language_model_provider")), "Language-model provider",
            "supported local or OpenAI-compatible provider");
        appendCheck(report, validEndpoint(value(store,
            "voice_language_model_endpoint")), "Language-model endpoint",
            "HTTP(S) URL without embedded credentials");
        appendCheck(report, parseUnsigned(value(store,
            "voice_language_model_timeout_ms"), 1U, 600'000U),
            "Language-model timeout", "range 1 through 600000 ms");
        appendCheck(report, parseUnsigned(value(store, "app_control_port"),
            1U, 65'535U), "App-control port", "range 1 through 65535");
    appendCheck(report, !value(store, "app_control_bind_address").empty(),
        "App-control bind address", "non-empty; localhost is recommended");
    report.lines.emplace_back("[SIMULATED] No hardware device or external service was accessed");
    return report;
}

::ctrl::stringvector XWALK_effectiveDeploymentConfig(
    ::ctrl::stringview configurationFilePath)
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
            key.find("token") != ::ctrl::stringview::npos;
        lines.emplace_back(::ctrl::string(entry.name) + " = " +
            (secretShaped && !effectiveValue.empty() ? "<redacted>" : effectiveValue));
    }
    lines.emplace_back("[SIMULATED] Known layered values only; no hardware was accessed");
    return lines;
}

} /* namespace xwalk::ctrl */
