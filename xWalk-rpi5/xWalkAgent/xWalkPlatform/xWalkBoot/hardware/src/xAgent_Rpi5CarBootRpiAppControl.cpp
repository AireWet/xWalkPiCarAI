/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiAppControl.cpp
 * @brief       Composes Raspberry Pi mobile-application control.
 * @details     Binds configured WebSocket, camera, and audio providers to PiCar-X.
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

#include "xAgent_Rpi5CarAppControlWebSocket.h"
#include "xAgent_Rpi5CarComputerVisionOpenCv.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarMusicAlsa.h"
#include "xHal_Rpi5CarMusicSndFileDecoder.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured mobile-application control.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runAppControl(agent::contextpointer context,
                                             bootapplicationcallback callback,
                                             hal::XWalkConfigStore& config,
                                             XWalkPicarx& picarx)
    {
        XWalkComputerVisionOpenCvConfiguration visionConfiguration;
        visionConfiguration.cameraBackend =
            XWalkComputerVisionOpenCv::backendFromString(config.get("computer_vision_camera_backend", "v4l2"));
        visionConfiguration.cameraDevice = config.get("computer_vision_camera_device", "");
        visionConfiguration.photoDirectory = config.get("computer_vision_photo_directory", "/tmp/xwalk-pictures");
        visionConfiguration.faceCascadePath = config.get(
            "computer_vision_face_cascade", "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml");
        visionConfiguration.widthPixels =
            parseUnsigned(config.get("computer_vision_width", "640"), "computer_vision_width", 7'680U);
        visionConfiguration.heightPixels =
            parseUnsigned(config.get("computer_vision_height", "480"), "computer_vision_height", 4'320U);
        visionConfiguration.readTimeoutMilliseconds = parseUnsigned(
            config.get("computer_vision_read_timeout_ms", "1000"), "computer_vision_read_timeout_ms", 60'000U);
        XWalkComputerVisionOpenCv visionBackend(visionConfiguration);
        XWalkComputerVisionCallbacks visionCallbacks = visionBackend.callbacks();
        visionCallbacks.delay = &delayMilliseconds;
        visionCallbacks.continueOperation = &continueComputerVision;
        XWalkAppControlWebSocket transport(config.get("app_control_bind_address", "127.0.0.1"));
        XWalkAppControlCallbacks appCallbacks = transport.callbacks(visionCallbacks);
        appCallbacks.visionContext = &visionBackend;
        XWalkAppControlConfiguration appConfiguration;
        appConfiguration.controllerName = config.get("app_control_name", "Picarx-001");
        appConfiguration.controllerType = config.get("app_control_type", "Picarx");
        appConfiguration.videoUrl = config.get("app_control_video_url", "");
        appConfiguration.controllerPort = static_cast<agent::uint16>(
            parseUnsigned(config.get("app_control_port", "8765"), "app_control_port", 65'535U));
        XWalkAppControl appControl(picarx, appCallbacks, appConfiguration);
        hal::XWalkAudioAlsa audioBackend(config.get("voice_playback_device", "default"),
                                         config.get("voice_mixer_device", "default"),
                                         config.get("voice_mixer_element", "PCM"));
        hal::XWalkMusicAlsa musicBackend(audioBackend, nullptr, hal::XWalkMusicSndFileDecoder::operations());
        hal::XWalkMusic music(&musicBackend, musicBackend.callbacks());
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.appControl = &appControl;
        services.music = &music;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
