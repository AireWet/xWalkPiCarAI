/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarTypes.h
 * @brief       Declares voice-active-car configuration and callbacks.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCar
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_TYPES_H
#define XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_TYPES_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::agent
{

using voiceactivecaroutputcallback = void (*)(hal::contextpointer, hal::stringview);
using voiceactivecarcontinuecallback = hal::boolean (*)(hal::contextpointer);
using voiceactivecardelaycallback = void (*)(hal::contextpointer, hal::uint32);
using voiceactivecarimagecallback = hal::string (*)(hal::contextpointer);

struct XWalkVoiceActiveCarCallbacks
{
    voiceactivecaroutputcallback output{nullptr}; /**< Writes one status line. */
    voiceactivecarcontinuecallback shouldContinue{nullptr}; /**< Controls foreground rounds. */
    voiceactivecardelaycallback delay{nullptr}; /**< Performs bounded LED timing. */
    voiceactivecarimagecallback captureImage{nullptr}; /**< Returns an optional image path. */
};

struct XWalkVoiceActiveCarConfiguration
{
    hal::float64 tooCloseCm{10.0}; /**< Ultrasonic trigger threshold in centimetres. */
    hal::boolean withImage{true}; /**< Enables image capture for ordinary prompts. */
    hal::uint32 listenTimeoutMs{30'000U}; /**< Bounded recognition interval. */
};

struct XWalkVoiceActiveCarResponse
{
    hal::string text{}; /**< Spoken response preceding the action delimiter. */
    hal::stringvector actions{}; /**< Parsed action names, or one `stop` fallback. */
};

/**
 * @enum XWalkVoiceControlledCarCommand
 * @brief Identifies one supported spoken command.
 */
enum class XWalkVoiceControlledCarCommand : hal::uint8
{
    /**
     * @brief Indicates that no supported command was recognized.
     */
    Unknown = 0U,
    /**
     * @brief Drives straight forward for the configured duration.
     */
    Forward,
    /**
     * @brief Drives straight backward for the configured duration.
     */
    Backward,
    /**
     * @brief Drives forward with left steering for the configured duration.
     */
    Left,
    /**
     * @brief Drives forward with right steering for the configured duration.
     */
    Right,
    /**
     * @brief Stops, centres steering, and returns to wake-word recognition.
     */
    Sleep
};

/**
 * @struct XWalkVoiceControlledCarConfiguration
 * @brief Configures local voice control.
 */
struct XWalkVoiceControlledCarConfiguration
{
    /**
     * @brief Non-empty phrase that opens one command session.
     */
    hal::string wakeWord{"hey robot"};
    /**
     * @brief Non-empty phrase that closes one command session.
     */
    hal::string sleepWord{"sleep"};
    /**
     * @brief Movement speed from zero through 100 percent.
     */
    hal::float64 speedPercent{30.0};
    /**
     * @brief Positive steering magnitude no greater than 40 degrees.
     */
    hal::float64 steeringAngle{25.0};
    /**
     * @brief Non-zero duration of each recognized movement in milliseconds.
     */
    hal::uint32 driveDurationMs{1'000U};
    /**
     * @brief Non-zero bound for each recognition request in milliseconds.
     */
    hal::uint32 listenTimeoutMs{5'000U};
};

/**
 * @struct XWalkVoicePromptCarConfiguration
 * @brief Configures the spoken movement demonstration.
 */
struct XWalkVoicePromptCarConfiguration
{
    /**
     * @brief Movement speed from zero through 100 percent.
     */
    hal::float64 speedPercent{30.0};
    /**
     * @brief Positive steering magnitude no greater than 40 degrees.
     */
    hal::float64 steeringAngle{20.0};
    /**
     * @brief Non-zero duration of each demonstration movement in milliseconds.
     */
    hal::uint32 driveDurationMs{2'000U};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_TYPES_H */
