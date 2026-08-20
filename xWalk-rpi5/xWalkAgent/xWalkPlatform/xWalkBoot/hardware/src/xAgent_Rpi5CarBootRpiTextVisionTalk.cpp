/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiTextVisionTalk.cpp
 * @brief       Composes the Raspberry Pi text-and-vision mode.
 *
 * @details
 * Owns the configured still camera and local Ollama model for one synchronous
 * application callback.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"
#include "xHal_Rpi5CarTrace.h"

#include "xAgent_Rpi5CarCameraCapture.h"
#include "xHal_Rpi5CarCameraLinux.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"

namespace xwalk::agent
{

    /**
     * @brief Runs image-grounded text conversation with configured providers.
     * @param[in] parameters Non-owning application callback and configuration
     * dependency valid through this synchronous composition.
     * @return Status returned by the configured callback.
     * @pre `parameters.callback` and `parameters.config` are non-null.
     */
    agent::int32 XWalkBootRpi::runTextVisionTalk(const xAgentContext& parameters)
    {
        hal::XWalkConfigStore& config = *parameters.config;
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .062, "Boot composing text-vision conversation services");
        const hal::XWalkCameraConnection cameraConnection =
            hal::XWalkCamera::connectionFromString(config.get("camera_connection", "csi"));
        const agent::boolean csiSelected =
            static_cast<agent::boolean>(cameraConnection == hal::XWalkCameraConnection::Csi);
        const agent::string cameraExecutable = csiSelected ? config.get("camera_csi_executable", "rpicam-still")
                                                           : config.get("camera_usb_executable", "ffmpeg");
        hal::XWalkCameraLinux cameraBackend(
            cameraConnection, cameraExecutable, config.get("camera_usb_device", "/dev/video0"));
        hal::XWalkCameraConfiguration cameraConfiguration;
        cameraConfiguration.widthPixels =
            parseUnsigned(config.get("text_vision_width", "1280"), "text_vision_width", 7'680U);
        cameraConfiguration.heightPixels =
            parseUnsigned(config.get("text_vision_height", "720"), "text_vision_height", 4'320U);
        hal::XWalkCamera camera(&cameraBackend, cameraBackend.callback(), cameraConfiguration);
        XWalkCameraCapture cameraCapture(camera, config.get("text_vision_image_path", "/tmp/llm-img.jpg"));
        hal::XWalkLanguageModelOllama modelBackend(
            config.get("text_vision_ollama_endpoint", "http://127.0.0.1:11434/api/chat"),
            config.get("text_vision_ollama_model", "llava:7b"));
        hal::XWalkLanguageModel languageModel(&modelBackend, modelBackend.callbacks());
        XWalkBootServices services{};
        services.cameraCapture = &cameraCapture;
        services.languageModel = &languageModel;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
