/******************************************************************************
 * @file        xControllerBootMode.cpp
 * @brief       Implements Controller command-to-boot-mode selection.
 *
 * @details
 * Maps each supported top-level Controller command to the smallest Agent boot
 * graph that supplies the command's required services.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-08-06
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

#include "xControllerBootMode.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller application boot selection for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/**
 * @brief Selects the minimum boot graph required by one parsed command group.
 * @param[in] commandArguments Complete command arguments excluding the executable name.
 * @return Command-specific Agent boot mode, or Base when no specialized service is required.
 */
agent::uint8 XWALK_selectBootMode(
    const ::ctrl::stringvector& commandArguments) noexcept
{
    const ::ctrl::boolean commandArgumentsAvailable =
        static_cast<::ctrl::boolean>(
            !commandArguments.empty());
    if (commandArgumentsAvailable)
    {
        if (commandArguments[0U] == "line-track")
        {
            return XWALK_BOOT_LINE_TRACKING_REQ;
        }
        if (commandArguments[0U] == "computer-vision")
        {
            return XWALK_BOOT_COMPUTER_VISION_REQ;
        }
        if (commandArguments[0U] == "stare-at-you")
        {
            return XWALK_BOOT_FACE_TRACKING_REQ;
        }
        if (commandArguments[0U] == "bull-fight")
        {
            return XWALK_BOOT_BULL_FIGHT_REQ;
        }
        if (commandArguments[0U] == "treasure-hunt")
        {
            return XWALK_BOOT_TREASURE_HUNT_REQ;
        }
        if (commandArguments[0U] == "record-video")
        {
            return XWALK_BOOT_VIDEO_RECORDING_REQ;
        }
        if (commandArguments[0U] == "video-car")
        {
            return XWALK_BOOT_VIDEO_CAR_REQ;
        }
        if (commandArguments[0U] == "app-control")
        {
            return XWALK_BOOT_APP_CONTROL_REQ;
        }
        if (commandArguments[0U] == "sound-background-music")
        {
            return XWALK_BOOT_SOUND_BACKGROUND_MUSIC_REQ;
        }
        if (commandArguments[0U] == "doctor")
        {
            return XWALK_BOOT_DOCTOR_REQ;
        }
        if (commandArguments[0U] == "self-drive")
        {
            return XWALK_BOOT_SELF_DRIVE_REQ;
        }
        if (commandArguments[0U] == "sound")
        {
            return XWALK_BOOT_SOUND_REQ;
        }
        if (commandArguments[0U] == "voice-chat")
        {
            return XWALK_BOOT_VOICE_CHAT_REQ;
        }
        if (commandArguments[0U] == "voice-active-car")
        {
            return XWALK_BOOT_VOICE_ACTIVE_CAR_REQ;
        }
        if (commandArguments[0U] == "voice-active-car-gpt")
        {
            return XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ;
        }
        if (commandArguments[0U] == "gpt-car")
        {
            return XWALK_BOOT_GPT_CAR_REQ;
        }
        if (commandArguments[0U] == "voice-controlled-car")
        {
            return XWALK_BOOT_VOICE_CONTROLLED_CAR_REQ;
        }
        if (commandArguments[0U] == "voice-prompt-car")
        {
            return XWALK_BOOT_VOICE_PROMPT_CAR_REQ;
        }
        if (commandArguments[0U] == "storytelling-robot")
        {
            return XWALK_BOOT_STORYTELLING_ROBOT_REQ;
        }
        if (commandArguments[0U] == "text-vision-talk")
        {
            return XWALK_BOOT_TEXT_VISION_TALK_REQ;
        }
        if (commandArguments[0U] == "online-llm-test")
        {
            return XWALK_BOOT_ONLINE_LLM_TEST_REQ;
        }
        if (commandArguments[0U] == "servo-zeroing")
        {
            return XWALK_BOOT_SERVO_ZEROING_REQ;
        }
        if (commandArguments[0U] == "spi")
        {
            return XWALK_BOOT_SPI_TRANSFER_REQ;
        }
    }
    return XWALK_BOOT_BASE_REQ;
}

} /* namespace xwalk::ctrl */
