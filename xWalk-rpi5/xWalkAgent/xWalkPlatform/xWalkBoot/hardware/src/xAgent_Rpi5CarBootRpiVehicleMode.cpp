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
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] boardControl Caller-owned board controller valid through this
     * call.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator valid through this
     * call.
     * @param[in] gpioDevice Configured GPIO character-device path.
     * @param[in] gpioChipName Optional exact kernel GPIO chip name.
     * @param[in] gpioChipLabel Optional exact kernel GPIO chip label.
     * @param[in] minimumGpioLineCount Required minimum GPIO line count.
     * @param[in] gpioCallbacks Linux GPIO operation table.
     * @return Status returned by the selected mode callback.
     */
    agent::int32 XWalkBootRpi::runVehicleMode(agent::contextpointer context,
                                              bootapplicationcallback callback,
                                              hal::XWalkConfigStore& config,
                                              hal::XWalkBoardControl& boardControl,
                                              XWalkPicarx& picarx,
                                              agent::stringview gpioDevice,
                                              agent::stringview gpioChipName,
                                              agent::stringview gpioChipLabel,
                                              agent::uint32 minimumGpioLineCount,
                                              const hal::XWalkGpioCallbacks& gpioCallbacks)
    {
        if (selectedMode == XWALK_BOOT_FACE_TRACKING_REQ)
        {
            return runFaceTracking(context, callback, config, picarx);
        }
        else if (selectedMode == XWALK_BOOT_TREASURE_HUNT_REQ)
        {
            return runTreasureHunt(context, callback, config, boardControl, picarx);
        }
        else if (selectedMode == XWALK_BOOT_BULL_FIGHT_REQ)
        {
            return runBullFight(context, callback, config, picarx);
        }
        else if (selectedMode == XWALK_BOOT_VIDEO_CAR_REQ)
        {
            return runVideoCar(context, callback, config, picarx);
        }
        else if (selectedMode == XWALK_BOOT_APP_CONTROL_REQ)
        {
            return runAppControl(context, callback, config, picarx);
        }
        else if (selectedMode == XWALK_BOOT_LINE_TRACKING_REQ)
        {
            return runLineTracking(context, callback, picarx);
        }
        else if (selectedMode == XWALK_BOOT_SELF_DRIVE_REQ)
        {
            return runSelfDrive(context, callback, config, picarx);
        }
        else if (selectedMode == XWALK_BOOT_SOUND_REQ)
        {
            return runSound(context, callback, config, picarx);
        }
        else if (selectedMode == XWALK_BOOT_SOUND_BACKGROUND_MUSIC_REQ)
        {
            return runSoundBackgroundMusic(context, callback, config, picarx);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_CONTROLLED_CAR_REQ)
        {
            return runVoiceControlledCar(context, callback, config, picarx);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_PROMPT_CAR_REQ)
        {
            return runVoicePromptCar(context, callback, config, boardControl, picarx);
        }
        else if (selectedMode == XWALK_BOOT_STORYTELLING_ROBOT_REQ)
        {
            return runStorytellingRobot(context, callback, config, boardControl, picarx);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_CHAT_REQ)
        {
            return runVoiceChat(context, callback, config, boardControl, picarx);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_ACTIVE_CAR_REQ)
        {
            return runVoiceActiveCar(context,
                                     callback,
                                     config,
                                     boardControl,
                                     picarx,
                                     gpioDevice,
                                     gpioChipName,
                                     gpioChipLabel,
                                     minimumGpioLineCount,
                                     gpioCallbacks);
        }
        else if (selectedMode == XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ)
        {
            return runVoiceActiveCarGpt(context,
                                        callback,
                                        config,
                                        boardControl,
                                        picarx,
                                        gpioDevice,
                                        gpioChipName,
                                        gpioChipLabel,
                                        minimumGpioLineCount,
                                        gpioCallbacks);
        }
        else if (selectedMode == XWALK_BOOT_GPT_CAR_REQ)
        {
            return runGptCar(context,
                             callback,
                             config,
                             boardControl,
                             picarx,
                             gpioDevice,
                             gpioChipName,
                             gpioChipLabel,
                             minimumGpioLineCount,
                             gpioCallbacks);
        }
        else if (selectedMode == XWALK_BOOT_BASE_REQ)
        {
            return runBase(context, callback, picarx);
        }
        XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "xWalkBoot vehicle mode is not supported");
    }

} /* namespace xwalk::agent */
