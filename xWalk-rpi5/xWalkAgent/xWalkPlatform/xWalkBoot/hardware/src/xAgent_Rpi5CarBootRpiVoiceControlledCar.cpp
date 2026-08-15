/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVoiceControlledCar.cpp
 * @brief       Composes Raspberry Pi wake-word vehicle control.
 * @details     Binds configured Vosk and ALSA capture providers to PiCar-X.
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

#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarSpeechRecognizerVosk.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured wake-word vehicle control.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runVoiceControlledCar(agent::contextpointer context,
                                                     bootapplicationcallback callback,
                                                     hal::XWalkConfigStore& config,
                                                     XWalkPicarx& picarx)
    {
        hal::XWalkSpeechRecognizerVosk recognizer(
            config.get("voice_vosk_library", "/usr/lib/xwalk/libvosk.so"),
            config.get("voice_vosk_model", "/usr/share/xwalk/models/vosk/vosk-model-small-en-us-0.15"));
        hal::XWalkSpeechToTextAlsa speechBackend(
            config.get("voice_capture_device", "default"), &recognizer, recognizer.operations());
        hal::XWalkSpeechToText speechToText(&speechBackend, speechBackend.callbacks());
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.speechToText = &speechToText;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
