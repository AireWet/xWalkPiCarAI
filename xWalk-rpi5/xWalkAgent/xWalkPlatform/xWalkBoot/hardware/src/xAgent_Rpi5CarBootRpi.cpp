/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpi.cpp
 * @brief       Implements Raspberry Pi boot lifecycle and mode dispatch.
 *
 * @details
 * Validates one boot selection, starts the one-shot lifecycle, loads the
 * deployment configuration, and delegates composition to one mode owner.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xAgent_Rpi5CarBootRpi.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarConfigStore.h"

#include "xHal_Rpi5CarTrace.h"
#include <charconv>
#include <cmath>
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Stores one boot selection without claiming hardware.
     * @param[in] mode Minimum hardware graph required by the application command.
     * @param[in] configFilePath Non-empty writable PiCar-X configuration path.
     * @throws std::invalid_argument If the path or mode is invalid.
     */
    XWalkBootRpi::XWalkBootRpi(agent::uint8 mode, agent::stringview configFilePath)
        : selectedMode(mode), configurationFilePath(configFilePath)
    {
        const agent::boolean configurationFilePathEmpty = static_cast<agent::boolean>(configurationFilePath.empty());
        if (configurationFilePathEmpty)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "xWalkBoot configuration file path is required");
        }
        switch (selectedMode)
        {
            case XWALK_BOOT_BASE_REQ:
            case XWALK_BOOT_DOCTOR_REQ:
            case XWALK_BOOT_COMPUTER_VISION_REQ:
            case XWALK_BOOT_FACE_TRACKING_REQ:
            case XWALK_BOOT_BULL_FIGHT_REQ:
            case XWALK_BOOT_TREASURE_HUNT_REQ:
            case XWALK_BOOT_VIDEO_RECORDING_REQ:
            case XWALK_BOOT_VIDEO_CAR_REQ:
            case XWALK_BOOT_APP_CONTROL_REQ:
            case XWALK_BOOT_SOUND_BACKGROUND_MUSIC_REQ:
            case XWALK_BOOT_LINE_TRACKING_REQ:
            case XWALK_BOOT_SELF_DRIVE_REQ:
            case XWALK_BOOT_SOUND_REQ:
            case XWALK_BOOT_VOICE_CHAT_REQ:
            case XWALK_BOOT_VOICE_ACTIVE_CAR_REQ:
            case XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ:
            case XWALK_BOOT_GPT_CAR_REQ:
            case XWALK_BOOT_VOICE_CONTROLLED_CAR_REQ:
            case XWALK_BOOT_VOICE_PROMPT_CAR_REQ:
            case XWALK_BOOT_STORYTELLING_ROBOT_REQ:
            case XWALK_BOOT_TEXT_VISION_TALK_REQ:
            case XWALK_BOOT_ONLINE_LLM_TEST_REQ:
            case XWALK_BOOT_SERVO_ZEROING_REQ:
            case XWALK_BOOT_SPI_TRANSFER_REQ:
            case XWALK_BOOT_VIDEO_STREAMING_REQ:
                break;
            default:
                XWALK_RPIAGENT_ERROR(XWALK_INVAL, "xWalkBoot mode is invalid");
        }
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Suspends one Agent action on the calling thread.
     * @param[in] context Optional context; unused.
     * @param[in] durationMs Requested duration in milliseconds.
     */
    void XWalkBootRpi::delayMilliseconds(agent::contextpointer context, agent::uint32 durationMs)
    {
        static_cast<void>(context);
        hal::common::sleepMilliseconds(durationMs);
    }

    /**
     * @brief Writes one angle through a boot-owned twelve-servo table.
     * @param[in,out] context Non-null pointer to twelve boot-owned Servo pointers.
     * @param[in] servoId Servo channel from zero through eleven.
     * @param[in] angleDegrees Logical angle in degrees.
     */
    void
    XWalkBootRpi::setServoZeroingAngle(agent::contextpointer context, agent::uint8 servoId, agent::float64 angleDegrees)
    {
        hal::XWalkServo** servos = static_cast<hal::XWalkServo**>(context);
        servos[servoId]->setAngle(angleDegrees);
    }

    /**
     * @brief Allows provider-local waits while application cancellation is checked
     * externally.
     * @param[in] context Optional context; unused.
     * @return Always `true`.
     */
    agent::boolean XWalkBootRpi::continueComputerVision(agent::contextpointer context) noexcept
    {
        static_cast<void>(context);
        return true;
    }

    /**
     * @brief Suspends one self-drive action and reports completion.
     * @param[in] context Optional context; unused.
     * @param[in] durationMs Requested duration in milliseconds.
     * @return Always `true` after the requested delay completes.
     */
    agent::boolean XWalkBootRpi::selfDriveDelayMilliseconds(agent::contextpointer context,
                                                            agent::uint32 durationMs) noexcept
    {
        static_cast<void>(context);
        hal::common::sleepMilliseconds(durationMs);
        return true;
    }

    /**
     * @brief Accepts the bounded speaker-prime request without emitting PCM.
     * @param[in] context Optional context; unused.
     * @param[in] durationMs Requested duration; unused by this silent callback.
     */
    void XWalkBootRpi::primeSpeaker(agent::contextpointer context, agent::uint32 durationMs)
    {
        static_cast<void>(context);
        static_cast<void>(durationMs);
    }

    /**
     * @brief Parses one bounded unsigned decimal deployment value.
     * @param[in] value Non-empty decimal digits without a sign or separator.
     * @param[in] optionName Non-empty option name included in validation failures.
     * @param[in] maximum Inclusive maximum accepted value.
     * @return Parsed unsigned value.
     * @throws std::invalid_argument If the value is empty or contains a non-digit.
     * @throws std::out_of_range If the value exceeds `maximum`.
     */
    agent::uint32
    XWalkBootRpi::parseUnsigned(agent::stringview value, agent::stringview optionName, agent::uint32 maximum)
    {
        const agent::boolean valueEmpty = static_cast<agent::boolean>(value.empty());
        if (valueEmpty)
        {
            const std::string exceptionMessage = std::string(optionName).append(" must not be empty");
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, exceptionMessage);
        }
        agent::uint32 result{};
        for (const char character : value)
        {
            const agent::boolean characterInvalid = static_cast<agent::boolean>((character < '0') || (character > '9'));
            if (characterInvalid)
            {
                const std::string exceptionMessage =
                    std::string(optionName).append(" must contain decimal digits only");
                XWALK_RPIAGENT_ERROR(XWALK_INVAL, exceptionMessage);
            }
            const agent::uint32 digit = static_cast<agent::uint32>(character - '0');
            const agent::uint32 maximumPrefix = maximum / 10U;
            const agent::uint32 maximumDigit = maximum % 10U;
            const agent::boolean valueExceedsMaximum = static_cast<agent::boolean>(
                (result > maximumPrefix) || ((result == maximumPrefix) && (digit > maximumDigit)));
            if (valueExceedsMaximum)
            {
                const std::string exceptionMessage = std::string(optionName).append(" exceeds its range");
                XWALK_RPIAGENT_ERROR(XWALK_RANGE, exceptionMessage);
            }
            result = (result * 10U) + digit;
        }
        return result;
    }

    /**
     * @brief Parses one strict lowercase deployment boolean.
     * @param[in] value Exact `true` or `false` text.
     * @param[in] optionName Configuration key included in failures.
     * @return Parsed boolean.
     * @throws std::invalid_argument If the value is not a supported boolean.
     */
    agent::boolean XWalkBootRpi::parseBoolean(agent::stringview value, agent::stringview optionName)
    {
        if (value == "true")
        {
            return true;
        }
        if (value == "false")
        {
            return false;
        }
        const std::string exceptionMessage = std::string(optionName).append(" must be true or false");
        XWALK_RPIAGENT_ERROR(XWALK_INVAL, exceptionMessage);
    }

    /**
     * @brief Parses finite positive seconds and converts them to milliseconds.
     * @param[in] value Decimal seconds without surrounding whitespace.
     * @param[in] optionName Configuration key included in failures.
     * @param[in] maximumMilliseconds Inclusive converted upper bound.
     * @return Rounded-up duration in milliseconds.
     * @throws std::invalid_argument If the value is malformed or not positive.
     * @throws std::out_of_range If the converted duration exceeds the bound.
     */
    agent::uint32
    XWalkBootRpi::parseSeconds(agent::stringview value, agent::stringview optionName, agent::uint32 maximumMilliseconds)
    {
        agent::float64 seconds{};
        const char* const begin = value.data();
        const char* const end = begin + value.size();
        const std::from_chars_result result = std::from_chars(begin, end, seconds);
        const agent::boolean invalid =
            static_cast<agent::boolean>(value.empty() || (result.ec != std::errc{}) || (result.ptr != end) ||
                                        !std::isfinite(seconds) || (seconds <= 0.0));
        if (invalid)
        {
            const std::string exceptionMessage = std::string(optionName).append(" must be finite positive seconds");
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, exceptionMessage);
        }
        const agent::float64 milliseconds = std::ceil(seconds * 1000.0);
        if (milliseconds > static_cast<agent::float64>(maximumMilliseconds))
        {
            const std::string exceptionMessage = std::string(optionName).append(" exceeds its range");
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, exceptionMessage);
        }
        return static_cast<agent::uint32>(milliseconds);
    }

    /**
     * @brief Parses a comma-separated list of trimmed non-empty phrases.
     * @param[in] value Comma-separated phrase text.
     * @param[in] optionName Configuration key included in failures.
     * @return Trimmed phrases in source order.
     * @throws std::invalid_argument If the list or one phrase is empty.
     */
    agent::stringvector XWalkBootRpi::parsePhraseList(agent::stringview value, agent::stringview optionName)
    {
        agent::stringvector phrases;
        agent::size offset{};
        while (offset <= value.size())
        {
            const agent::size comma = value.find(',', offset);
            const agent::size end = comma == agent::stringview::npos ? value.size() : comma;
            agent::string phrase(value.substr(offset, end - offset));
            const agent::size first = phrase.find_first_not_of(" \t\r\n");
            const agent::size last = phrase.find_last_not_of(" \t\r\n");
            if ((first == agent::string::npos) || (last == agent::string::npos))
            {
                const std::string exceptionMessage = std::string(optionName).append(" contains an empty phrase");
                XWALK_RPIAGENT_ERROR(XWALK_INVAL, exceptionMessage);
            }
            phrases.emplace_back(phrase.substr(first, (last - first) + 1U));
            if (comma == agent::stringview::npos)
            {
                break;
            }
            offset = comma + 1U;
        }
        if (phrases.empty())
        {
            const std::string exceptionMessage = std::string(optionName).append(" must not be empty");
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, exceptionMessage);
        }
        return phrases;
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Claims hardware and executes one application callback.
     * @param[in] parameters Application context and non-null callback retained
     * through the selected synchronous composition.
     * @return Status returned by the configured callback.
     * @throws std::invalid_argument If the configured callback is null.
     * @throws std::logic_error If this object already started once.
     * @warning Claims only the physical resources required by the selected mode.
     */
    agent::int32 XWalkBootRpi::run(const xAgentContext& parameters)
    {
        begin(parameters.callback);
        XWALK_RPIAGENT_TRACE_UID1(
            RPIAGENT .019, "Raspberry Pi boot composition selected mode %u", static_cast<agent::uint32>(selectedMode));
        if (selectedMode == XWALK_BOOT_DOCTOR_REQ)
        {
            return runDoctor(parameters);
        }

        hal::XWalkConfigStore config(configurationFilePath);
        xAgentContext configuredParameters = parameters;
        configuredParameters.config = &config;
        if (selectedMode == XWALK_BOOT_TEXT_VISION_TALK_REQ)
        {
            return runTextVisionTalk(configuredParameters);
        }
        else if (selectedMode == XWALK_BOOT_ONLINE_LLM_TEST_REQ)
        {
            return runOnlineLlmTest(configuredParameters);
        }
        else if (selectedMode == XWALK_BOOT_COMPUTER_VISION_REQ)
        {
            return runComputerVision(configuredParameters);
        }
        else if (selectedMode == XWALK_BOOT_VIDEO_RECORDING_REQ)
        {
            return runVideoRecording(configuredParameters);
        }
        else if (selectedMode == XWALK_BOOT_VIDEO_STREAMING_REQ)
        {
            return runVideoStreaming(configuredParameters);
        }
        else if (selectedMode == XWALK_BOOT_SPI_TRANSFER_REQ)
        {
            return runSpiTransfer(configuredParameters);
        }
        return runVehicle(configuredParameters);
    }

} /* namespace xwalk::agent */
