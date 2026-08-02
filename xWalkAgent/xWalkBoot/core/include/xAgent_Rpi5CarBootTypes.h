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
#include "xAgent_Rpi5CarSelfDrive.h"
#include "xAgent_Rpi5CarSpiTransfer.h"
#include "xAgent_Rpi5CarVoiceActiveCar.h"
#include "xAgent_Rpi5CarVoiceControlledCar.h"
#include "xAgent_Rpi5CarVoicePromptCar.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent
{

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/**
 * @enum XWalkBootMode
 * @brief Selects the minimum process hardware composition.
 */
enum class XWalkBootMode : hal::uint8
{
    /**
     * @brief Creates the base PiCar-X controller graph.
     */
    Base = 0U,
    /**
     * @brief Performs passive deployment inspection without claiming outputs.
     */
    Doctor,
    /**
     * @brief Adds the foreground line-tracking coordinator.
     */
    LineTracking,
    /**
     * @brief Adds Music and the preset-action coordinator.
     */
    SelfDrive,
    /**
     * @brief Adds Music for standalone audio commands.
     */
    Sound,
    /**
     * @brief Selects the local voice-chatbot composition.
     */
    VoiceChat,
    /**
     * @brief Selects the base voice-active-car composition.
     */
    VoiceActiveCar,
    /**
     * @brief Selects the English GPT voice-active-car profile.
     */
    VoiceActiveCarGpt,
    /**
     * @brief Selects wake-word movement control.
     */
    VoiceControlledCar,
    /**
     * @brief Selects the spoken movement demonstration.
     */
    VoicePromptCar,
    /**
     * @brief Selects only one configured Linux SPI device.
     */
    SpiTransfer
};

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
    const hal::stringvector* doctorLines{nullptr};
    /** @brief Base PiCar-X coordinator, null for the SPI-only boot mode. */
    XWalkPicarx* picarx{nullptr};
    /** @brief Optional SPI transaction Agent selected by SPI-only boot mode. */
    XWalkSpiTransfer* spiTransfer{nullptr};
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
    /** @brief Optional boot-owned speech-recognition coordinator. */
    hal::XWalkSpeechToText* speechToText{nullptr};
    /** @brief Optional boot-owned speech-synthesis coordinator. */
    hal::XWalkTextToSpeech* textToSpeech{nullptr};
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
using bootapplicationcallback = hal::int32 (*)(hal::contextpointer context,
    XWalkBootServices& services);

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_BOOT_TYPES_H */
