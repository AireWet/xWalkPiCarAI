/******************************************************************************
 * @file        xAgent_Rpi5CarBootTypes.h
 * @brief       Declares xWalk process-boot modes, services, and callbacks.
 *
 * @details
 * Defines the hardware-independent boundary between a platform composition
 * owner and one application operation executed while that composition lives.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot
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

#ifndef XAGENT_RPI5CAR_BOOT_TYPES_H
#define XAGENT_RPI5CAR_BOOT_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "xAgent_Rpi5CarLineTracking.h"
#include "xAgent_Rpi5CarLocalVoiceChatbot.h"
#include "xAgent_Rpi5CarCameraCapture.h"
#include "xAgent_Rpi5CarComputerVision.h"
#include "xAgent_Rpi5CarFaceTracking.h"
#include "xAgent_Rpi5CarBullFight.h"
#include "xAgent_Rpi5CarTreasureHunt.h"
#include "xAgent_Rpi5CarVideoRecording.h"
#include "xAgent_Rpi5CarVideoCar.h"
#include "xAgent_Rpi5CarAppControl.h"
#include "xAgent_Rpi5CarSoundBackgroundMusic.h"
#include "xAgent_Rpi5CarSelfDrive.h"
#include "xAgent_Rpi5CarSpiTransfer.h"
#include "xAgent_Rpi5CarVoiceActiveCar.h"
#include "xAgent_Rpi5CarVoiceControlledCar.h"
#include "xAgent_Rpi5CarVoicePromptCar.h"
#include "xAgent_Rpi5CarStorytellingRobot.h"
#include "xAgent_Rpi5CarTextVisionTalk.h"
#include "xAgent_Rpi5CarOnlineLlmTest.h"
#include "xAgent_Rpi5CarGptCar.h"
#include "xAgent_Rpi5CarServoZeroing.h"

/******************************************************************************
 * Boot request definitions
 ******************************************************************************/

/** @brief Creates the base PiCar-X controller graph. */
#define XWALK_BOOT_BASE_REQ (::agent::uint8{0U})
/** @brief Performs passive deployment inspection without claiming outputs. */
#define XWALK_BOOT_DOCTOR_REQ (::agent::uint8{1U})
/** @brief Selects only the configured OpenCV camera provider. */
#define XWALK_BOOT_COMPUTER_VISION_REQ (::agent::uint8{2U})
/** @brief Adds OpenCV face detection and camera-servo tracking. */
#define XWALK_BOOT_FACE_TRACKING_REQ (::agent::uint8{3U})
/** @brief Adds OpenCV red detection and PiCar-X target pursuit. */
#define XWALK_BOOT_BULL_FIGHT_REQ (::agent::uint8{4U})
/** @brief Adds the color treasure hunt with Pico2Wave speech. */
#define XWALK_BOOT_TREASURE_HUNT_REQ (::agent::uint8{5U})
/** @brief Selects continuous OpenCV video recording. */
#define XWALK_BOOT_VIDEO_RECORDING_REQ (::agent::uint8{6U})
/** @brief Adds interactive camera-assisted PiCar-X driving. */
#define XWALK_BOOT_VIDEO_CAR_REQ (::agent::uint8{7U})
/** @brief Adds explicitly bound SunFounder mobile-app control. */
#define XWALK_BOOT_APP_CONTROL_REQ (::agent::uint8{8U})
/** @brief Adds interactive foreground effects and background music. */
#define XWALK_BOOT_SOUND_BACKGROUND_MUSIC_REQ (::agent::uint8{9U})
/** @brief Adds the foreground line-tracking coordinator. */
#define XWALK_BOOT_LINE_TRACKING_REQ (::agent::uint8{10U})
/** @brief Adds Music and the preset-action coordinator. */
#define XWALK_BOOT_SELF_DRIVE_REQ (::agent::uint8{11U})
/** @brief Adds Music for standalone audio commands. */
#define XWALK_BOOT_SOUND_REQ (::agent::uint8{12U})
/** @brief Selects the local voice-chatbot composition. */
#define XWALK_BOOT_VOICE_CHAT_REQ (::agent::uint8{13U})
/** @brief Selects the base voice-active-car composition. */
#define XWALK_BOOT_VOICE_ACTIVE_CAR_REQ (::agent::uint8{14U})
/** @brief Selects the English GPT voice-active-car profile. */
#define XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ (::agent::uint8{15U})
/** @brief Selects the upstream GPT PiCar-X assistant profile. */
#define XWALK_BOOT_GPT_CAR_REQ (::agent::uint8{16U})
/** @brief Selects wake-word movement control. */
#define XWALK_BOOT_VOICE_CONTROLLED_CAR_REQ (::agent::uint8{17U})
/** @brief Selects the spoken movement demonstration. */
#define XWALK_BOOT_VOICE_PROMPT_CAR_REQ (::agent::uint8{18U})
/** @brief Selects the Piper storytelling movement demonstration. */
#define XWALK_BOOT_STORYTELLING_ROBOT_REQ (::agent::uint8{19U})
/** @brief Selects image-grounded typed conversation through Ollama. */
#define XWALK_BOOT_TEXT_VISION_TALK_REQ (::agent::uint8{20U})
/** @brief Selects the OpenAI text-only conversation from example 18. */
#define XWALK_BOOT_ONLINE_LLM_TEST_REQ (::agent::uint8{21U})
/** @brief Selects MCU reset and all twelve Robot HAT servo channels. */
#define XWALK_BOOT_SERVO_ZEROING_REQ (::agent::uint8{22U})
/** @brief Selects only one configured Linux SPI device. */
#define XWALK_BOOT_SPI_TRANSFER_REQ (::agent::uint8{23U})

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @struct XWalkBootServices
 * @brief Publishes non-owning services valid only during one boot callback.
 */
struct XWalkBootServices
{
    /** @brief Optional passive preflight lines selected by Doctor boot mode. */
    const agent::stringvector* doctorLines{nullptr};
    /** @brief Base PiCar-X coordinator, null for the SPI-only boot mode. */
    XWalkPicarx* picarx{nullptr};
    /** @brief Optional SPI transaction Agent selected by SPI-only boot mode. */
    XWalkSpiTransfer* spiTransfer{nullptr};
    /** @brief Optional twelve-channel servo-zeroing Agent. */
    XWalkServoZeroing* servoZeroing{nullptr};
    /** @brief Optional line-tracking coordinator selected by the boot mode. */
    XWalkLineTracking* lineTracking{nullptr};
    /** @brief Optional self-drive coordinator selected by the boot mode. */
    XWalkSelfDrive* selfDrive{nullptr};
    /** @brief Optional Music controller selected by an audio boot mode. */
    hal::XWalkMusic* music{nullptr};
    /** @brief Optional local voice-chatbot coordinator selected by voice-chat mode. */
    XWalkLocalVoiceChatbot* localVoiceChatbot{nullptr};
    /** @brief Optional sensor-aware voice-active-car coordinator. */
    XWalkVoiceActiveCar* voiceActiveCar{nullptr};
    /** @brief Optional wake-word voice-controlled-car coordinator. */
    XWalkVoiceControlledCar* voiceControlledCar{nullptr};
    /** @brief Optional spoken movement-demonstration coordinator. */
    XWalkVoicePromptCar* voicePromptCar{nullptr};
    /** @brief Optional boot-owned complete voice-assistant pipeline. */
    hal::XWalkVoiceAssistant* voiceAssistant{nullptr};
    /** @brief Optional boot-owned status LED used by the voice-active car. */
    hal::XWalkLed* voiceStatusLed{nullptr};
    /** @brief Optional boot-owned still-image capture Agent. */
    XWalkCameraCapture* cameraCapture{nullptr};
    /** @brief Optional interactive computer-vision Agent. */
    XWalkComputerVision* computerVision{nullptr};
    /** @brief Optional bounded camera-servo face-tracking Agent. */
    XWalkFaceTracking* faceTracking{nullptr};
    /** @brief Optional bounded red-target pursuit Agent. */
    XWalkBullFight* bullFight{nullptr};
    /** @brief Optional interactive color treasure-hunt Agent. */
    XWalkTreasureHunt* treasureHunt{nullptr};
    /** @brief Optional interactive video-recording Agent. */
    XWalkVideoRecording* videoRecording{nullptr};
    /** @brief Optional interactive camera-assisted driving Agent. */
    XWalkVideoCar* videoCar{nullptr};
    /** @brief Optional mobile-app vehicle-control Agent. */
    XWalkAppControl* appControl{nullptr};
    /** @brief Optional interactive sound-and-music Agent. */
    XWalkSoundBackgroundMusic* soundBackgroundMusic{nullptr};
    /** @brief Optional boot-owned speech-recognition coordinator. */
    hal::XWalkSpeechToText* speechToText{nullptr};
    /** @brief Optional boot-owned speech-synthesis coordinator. */
    hal::XWalkTextToSpeech* textToSpeech{nullptr};
    /** @brief Optional boot-owned language model for text-vision talk. */
    hal::XWalkLanguageModel* languageModel{nullptr};
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Executes one application operation while boot services remain alive.
 * @param[in,out] context Nullable caller-owned application context.
 * @param[in,out] services Non-owning services valid only for this callback.
 * @return Application-defined process status.
 */
using bootapplicationcallback = agent::int32 (*)(agent::contextpointer context,
    XWalkBootServices& services);

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_BOOT_TYPES_H */
