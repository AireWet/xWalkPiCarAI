/******************************************************************************
 * @file        xControllerRunner.cpp
 * @brief       Implements boot-service Controller command composition.
 *
 * @details
 * Builds the command-specific Controller and temporary Agent adapters while
 * the selected boot graph retains every required hardware service.
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
#include "xControllerRunner.h"
#include "xAgent_Rpi5CarBootTypes.h"
#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"
#include "xController.h"
#include "xControllerApplicationSupport.h"
#include "xControllerCommands.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller application composition for the xWalk firmware.
 */
namespace xwalk::ctrl
{

    /**
     * @brief Executes one CLI command through services retained by xWalkBoot.
     * @param[in,out] context Non-null Controller boot context valid throughout the
     * call.
     * @param[in,out] services Command-specific non-owning services retained by
     * xWalkBoot.
     * @return Command-specific status, or three when the required base service is
     * absent.
     */
    ::ctrl::int32 XWALK_runController(::ctrl::contextpointer context, agent::XWalkBootServices& services)
    {
        const ::ctrl::boolean signalHandlingActivated = XWALK_activateOperationSignalHandling();
        if (signalHandlingActivated == false)
        {
            return 2;
        }
        XWALK_CTRL_TRACE_UID0(CTRL .001, "Controller command execution started");
        const XWalkControllerBootContext& bootContext = *static_cast<XWalkControllerBootContext*>(context);
        const ::ctrl::stringvector& commandArguments = *bootContext.commandArguments;
        const XWalkControllerCallbacks callbacks{&XWALK_outputLine,
                                                 &XWALK_inputLine,
                                                 &XWALK_delayMilliseconds,
                                                 &XWALK_monotonicMilliseconds,
                                                 &XWALK_continueOperation,
                                                 &XWALK_performSound};

        XWalkControllerApplicationContext applicationContext{services.music, bootContext.resourceDirectory};

        if (services.doctorLines != nullptr)
        {
            XWalkController cli(*services.doctorLines, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.spiTransfer != nullptr)
        {
            XWalkController cli(*services.spiTransfer, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.servoZeroing != nullptr)
        {
            XWalkController cli(*services.servoZeroing, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.computerVision != nullptr)
        {
            XWalkController cli(*services.computerVision, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if ((services.languageModel != nullptr) && (services.cameraCapture != nullptr))
        {
            const xwalk::agent::XWalkTextVisionTalkCallbacks talkCallbacks{
                &XWALK_outputLine, &XWALK_inputLine, &XWALK_delayMilliseconds, &XWALK_continueOperation};
            xwalk::agent::XWalkTextVisionTalk textVisionTalk(
                *services.languageModel, *services.cameraCapture, &applicationContext, talkCallbacks);
            XWalkController cli(textVisionTalk, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.languageModel != nullptr)
        {
            const xwalk::agent::XWalkOnlineLlmTestCallbacks onlineCallbacks{
                &XWALK_outputLine, &XWALK_inputLine, &XWALK_continueOperation};
            xwalk::agent::XWalkOnlineLlmTest onlineLlmTest(
                *services.languageModel, &applicationContext, onlineCallbacks);
            XWalkController cli(onlineLlmTest, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.videoRecording != nullptr)
        {
            XWalkController cli(*services.videoRecording, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.videoStreaming != nullptr)
        {
            XWalkController cli(*services.videoStreaming, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.soundBackgroundMusic != nullptr)
        {
            XWalkController cli(*services.soundBackgroundMusic, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.faceTracking != nullptr)
        {
            if (services.picarx == nullptr)
            {
                XWALK_CTRL_ERROR(XWALK_LOGIC, "xWalkBoot did not provide PiCar-X for face tracking");
            }

            XWalkController cli(*services.picarx, *services.faceTracking, &applicationContext, callbacks);

            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.bullFight != nullptr)
        {
            if (services.picarx == nullptr)
            {
                XWALK_CTRL_ERROR(XWALK_LOGIC, "xWalkBoot did not provide PiCar-X for bull fight");
            }

            XWalkController cli(*services.picarx, *services.bullFight, &applicationContext, callbacks);

            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.treasureHunt != nullptr)
        {
            if (services.picarx == nullptr)
            {
                XWALK_CTRL_ERROR(XWALK_LOGIC, "xWalkBoot did not provide PiCar-X for treasure hunt");
            }

            XWalkController cli(*services.picarx, *services.treasureHunt, &applicationContext, callbacks);

            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.videoCar != nullptr)
        {
            if (services.picarx == nullptr)
            {
                XWALK_CTRL_ERROR(XWALK_LOGIC, "xWalkBoot did not provide PiCar-X for video car");
            }

            XWalkController cli(*services.picarx, *services.videoCar, &applicationContext, callbacks);

            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.appControl != nullptr)
        {
            if (services.picarx == nullptr)
            {
                XWALK_CTRL_ERROR(XWALK_LOGIC, "xWalkBoot did not provide PiCar-X for app control");
            }

            XWalkController cli(*services.picarx, *services.appControl, &applicationContext, callbacks);

            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.picarx == nullptr)
        {
            return 3;
        }

        if (services.selfDrive != nullptr)
        {
            services.selfDrive->setCancellation(&applicationContext, &XWALK_continueOperation);
        }

        if (services.lineTracking != nullptr)
        {
            XWalkController cli(*services.picarx, *services.lineTracking, &applicationContext, callbacks);

            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.voiceAssistant != nullptr)
        {
            const ::ctrl::boolean commandArgumentsValid =
                static_cast<::ctrl::boolean>(!commandArguments.empty() && (commandArguments[0U] == "voice-chat"));

            if (commandArgumentsValid)
            {
                const xwalk::agent::XWalkLocalVoiceChatbotCallbacks voiceCallbacks{
                    &XWALK_outputLine, &XWALK_continueOperation, &XWALK_delayMilliseconds};
                xwalk::agent::XWalkLocalVoiceChatbot chatbot(*services.voiceAssistant, nullptr, voiceCallbacks);

                XWalkController cli(*services.picarx, chatbot, &applicationContext, callbacks);

                return XWALK_runControllerCommand(cli, commandArguments);
            }

            if ((services.selfDrive == nullptr) || (services.voiceStatusLed == nullptr))
            {
                XWALK_CTRL_ERROR(XWALK_LOGIC, "xWalkBoot did not provide the voice-active-car hardware graph");
            }

            const xwalk::agent::XWalkVoiceActiveCarCallbacks voiceCallbacks{&XWALK_outputLine,
                                                                            &XWALK_continueOperation,
                                                                            &XWALK_delayMilliseconds,
                                                                            &XWALK_monotonicMilliseconds,
                                                                            nullptr,
                                                                            &XWALK_inputLine};

            xwalk::agent::XWalkVoiceActiveCarConfiguration voiceConfiguration =
                xwalk::agent::XWalkVoiceActiveCar::carConfiguration();

            const ::ctrl::boolean voiceActiveCarGptRequested = static_cast<::ctrl::boolean>(
                !commandArguments.empty() && (commandArguments[0U] == "voice-active-car-gpt"));

            if (voiceActiveCarGptRequested)
            {
                voiceConfiguration = xwalk::agent::XWalkVoiceActiveCarGpt::carConfiguration();
            }
            else
            {
                const ::ctrl::boolean gptCarRequested =
                    static_cast<::ctrl::boolean>(!commandArguments.empty() && (commandArguments[0U] == "gpt-car"));
                if (gptCarRequested)
                {
                    voiceConfiguration = xwalk::agent::XWalkGptCar::carConfiguration();
                }
            }

            if (services.voiceActiveCarConfiguration != nullptr)
            {
                voiceConfiguration = *services.voiceActiveCarConfiguration;
            }
            if (voiceActiveCarGptRequested)
            {
                voiceConfiguration.withImage = false;
            }
            const ::ctrl::boolean imageCaptureMissing =
                static_cast<::ctrl::boolean>(voiceConfiguration.withImage && (services.cameraCapture == nullptr));
            if (imageCaptureMissing)
            {
                XWALK_CTRL_ERROR(XWALK_LOGIC, "xWalkBoot did not provide enabled voice-active camera capture");
            }
            xwalk::agent::voiceactivecarimagecallback imageCallback = nullptr;
            ::ctrl::contextpointer voiceContext = &applicationContext;
            if (voiceConfiguration.withImage && (voiceActiveCarGptRequested == false))
            {
                imageCallback = &xwalk::agent::XWalkCameraCapture::captureImage;
                voiceContext = services.cameraCapture;
            }
            xwalk::agent::XWalkVoiceActiveCarCallbacks configuredCallbacks = voiceCallbacks;
            configuredCallbacks.captureImage = imageCallback;
            xwalk::agent::XWalkVoiceActiveCar voiceActiveCar(*services.picarx,
                                                             *services.selfDrive,
                                                             *services.voiceAssistant,
                                                             *services.voiceStatusLed,
                                                             voiceContext,
                                                             configuredCallbacks,
                                                             voiceConfiguration);

            const ::ctrl::boolean gptCarRequested =
                static_cast<::ctrl::boolean>(!commandArguments.empty() && (commandArguments[0U] == "gpt-car"));

            if (gptCarRequested)
            {
                xwalk::agent::XWalkGptCar gptCar(voiceActiveCar);
                XWalkController cli(*services.picarx, gptCar, &applicationContext, callbacks);
                return XWALK_runControllerCommand(cli, commandArguments);
            }

            XWalkController cli(*services.picarx, voiceActiveCar, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.selfDrive != nullptr)
        {
            XWalkController cli(*services.picarx, *services.selfDrive, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.localVoiceChatbot != nullptr)
        {
            XWalkController cli(*services.picarx, *services.localVoiceChatbot, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.voiceActiveCar != nullptr)
        {
            XWalkController cli(*services.picarx, *services.voiceActiveCar, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.voiceControlledCar != nullptr)
        {
            XWalkController cli(*services.picarx, *services.voiceControlledCar, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.speechToText != nullptr)
        {
            const xwalk::agent::XWalkVoiceControlledCarCallbacks voiceCallbacks{
                &XWALK_outputLine, &XWALK_continueOperation, &XWALK_delayMilliseconds};
            xwalk::agent::XWalkVoiceControlledCar voiceControlledCar(
                *services.picarx, *services.speechToText, nullptr, voiceCallbacks);
            XWalkController cli(*services.picarx, voiceControlledCar, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.voicePromptCar != nullptr)
        {
            XWalkController cli(*services.picarx, *services.voicePromptCar, &applicationContext, callbacks);
            return XWALK_runControllerCommand(cli, commandArguments);
        }

        if (services.textToSpeech != nullptr)
        {
            const ::ctrl::boolean storytellingRobotRequested = static_cast<::ctrl::boolean>(
                !commandArguments.empty() && (commandArguments[0U] == "storytelling-robot"));

            if (storytellingRobotRequested)
            {
                const xwalk::agent::XWalkStorytellingRobotCallbacks storyCallbacks{&XWALK_delayMilliseconds,
                                                                                   &XWALK_continueOperation};

                xwalk::agent::XWalkStorytellingRobot storytellingRobot(
                    *services.picarx, *services.textToSpeech, nullptr, storyCallbacks);
                XWalkController cli(*services.picarx, storytellingRobot, &applicationContext, callbacks);

                return XWALK_runControllerCommand(cli, commandArguments);
            }

            const xwalk::agent::XWalkVoicePromptCarCallbacks voiceCallbacks{
                &XWALK_outputLine, &XWALK_continueOperation, &XWALK_delayMilliseconds};

            xwalk::agent::XWalkVoicePromptCar voicePromptCar(
                *services.picarx, *services.textToSpeech, nullptr, voiceCallbacks);

            XWalkController cli(*services.picarx, voicePromptCar, &applicationContext, callbacks);

            return XWALK_runControllerCommand(cli, commandArguments);
        }
        xwalk::agent::XWalkGrayscaleCalibration grayscaleCalibration(
            *services.picarx, &applicationContext, &XWALK_delayMilliseconds, &XWALK_continueOperation);
        xwalk::agent::XWalkServoMotorCalibration servoMotorCalibration(
            *services.picarx, &applicationContext, &XWALK_delayMilliseconds, &XWALK_continueOperation);
        xwalk::agent::XWalkMoveExample moveExample(
            *services.picarx, &applicationContext, &XWALK_delayMilliseconds, &XWALK_continueOperation);
        xwalk::agent::XWalkKeyboardControl keyboardControl(
            *services.picarx, &applicationContext, &XWALK_delayMilliseconds, &XWALK_continueOperation);
        xwalk::agent::XWalkObstacleAvoidance obstacleAvoidance(
            *services.picarx, &applicationContext, &XWALK_delayMilliseconds, &XWALK_continueOperation);
        xwalk::agent::XWalkCliffDetection cliffDetection(
            *services.picarx, &applicationContext, &XWALK_delayMilliseconds, &XWALK_continueOperation);
        XWalkController cli(*services.picarx,
                            grayscaleCalibration,
                            servoMotorCalibration,
                            moveExample,
                            keyboardControl,
                            obstacleAvoidance,
                            cliffDetection,
                            &applicationContext,
                            callbacks);
        return XWALK_runControllerCommand(cli, commandArguments);
    }

} /* namespace xwalk::ctrl */
