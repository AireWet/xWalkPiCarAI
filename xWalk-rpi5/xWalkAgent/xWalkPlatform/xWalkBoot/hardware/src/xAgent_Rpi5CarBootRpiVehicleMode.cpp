/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVehicleMode.cpp
 * @brief       Dispatches one configured PiCar-X boot mode.
 * @details     Delegates the shared vehicle graph to one mode-specific owner.
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::agent
{

    /**
     * @brief Selects one mode after the common PiCar-X graph is available.
     * @param[in] parameters Non-owning application, vehicle, configuration, and
     * GPIO dependencies valid through the synchronous dispatch.
     * @return Status returned by the selected mode callback.
     * @pre Every required pointer in `parameters` is non-null.
     */
    agent::int32 XWalkBootRpi::runVehicleMode(const xAgentContext& parameters)
    {
        if (selectedMode == XWALK_BOOT_FACE_TRACKING_REQ)
        {
            return runFaceTracking(parameters);
        }
        else if (selectedMode == XWALK_BOOT_TREASURE_HUNT_REQ)
        {
            return runTreasureHunt(parameters);
        }
        else if (selectedMode == XWALK_BOOT_BULL_FIGHT_REQ)
        {
            return runBullFight(parameters);
        }
        else if (selectedMode == XWALK_BOOT_VIDEO_CAR_REQ)
        {
            return runVideoCar(parameters);
        }
        else if (selectedMode == XWALK_BOOT_APP_CONTROL_REQ)
        {
            return runAppControl(parameters);
        }
        else if (selectedMode == XWALK_BOOT_LINE_TRACKING_REQ)
        {
            return runLineTracking(parameters);
        }
        else if (selectedMode == XWALK_BOOT_SELF_DRIVE_REQ)
        {
            return runSelfDrive(parameters);
        }
        else if (selectedMode == XWALK_BOOT_SOUND_REQ)
        {
            return runSound(parameters);
        }
        else if (selectedMode == XWALK_BOOT_SOUND_BACKGROUND_MUSIC_REQ)
        {
            return runSoundBackgroundMusic(parameters);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_CONTROLLED_CAR_REQ)
        {
            return runVoiceControlledCar(parameters);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_PROMPT_CAR_REQ)
        {
            return runVoicePromptCar(parameters);
        }
        else if (selectedMode == XWALK_BOOT_STORYTELLING_ROBOT_REQ)
        {
            return runStorytellingRobot(parameters);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_CHAT_REQ)
        {
            return runVoiceChat(parameters);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_ACTIVE_CAR_REQ)
        {
            return runVoiceActiveCar(parameters);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ)
        {
            return runVoiceActiveCarGpt(parameters);
        }
        else if (selectedMode == XWALK_BOOT_GPT_CAR_REQ)
        {
            return runGptCar(parameters);
        }
        else if (selectedMode == XWALK_BOOT_BASE_REQ)
        {
            return runBase(parameters);
        }
        XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "xWalkBoot vehicle mode is not supported");
    }

} /* namespace xwalk::agent */
