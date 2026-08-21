/******************************************************************************
 * @file        xControllerCommandRunner.cpp
 * @brief       Implements configured Controller command execution.
 *
 * @details
 * Parses one command, selects the typed handler request, and preserves command-scope safety cleanup.
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

#include "xControllerCommands.h"

#include "xControllerParsing.h"
#include "xControllerPicarxCommands.h"
#include "xAgent_Rpi5CarPicarxSafetyGuard.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

    /**
     * @brief Executes one application command through a configured controller.
     *
     * @param[in,out] controller Controller whose non-owning dependencies and
     * callbacks remain valid for the complete call.
     * @param[in] arguments Command arguments excluding the executable name.
     *
     * @return
     * Zero on success, two for a command-level failure, or three when the selected
     * backend is unavailable.
     *
     * @throws std::invalid_argument
     * The argument list is empty or the selected command syntax is invalid.
     *
     * @post
     * A PiCar-X command has performed command-scope emergency-stop cleanup.
     */
    ::ctrl::int32 XWALK_runControllerCommand(XWalkController& controller, const ::ctrl::stringvector& arguments)
    {
        const XWalkControllerCommandRequest request = XWALK_parseControllerCommand(arguments);
        if (request.command == XWALK_CNTRL_HELP_REQ)
        {
            controller.output(XWALK_controllerUsage());
            return 0;
        }
        else if (request.command == XWALK_CNTRL_SPI_REQ)
        {
            return controller.XWALK_handlerSpi(XWALK_parseSpiRequest(request.arguments));
        }
        else if (request.command == XWALK_CNTRL_DOCTOR_REQ)
        {
            return controller.XWALK_handlerDoctor(
                XWALK_parseNoArgumentRequest(request.arguments, "doctor accepts no additional arguments"));
        }
        else if (request.command == XWALK_CNTRL_SERVO_ZEROING_REQ)
        {
            return controller.XWALK_handlerServoZeroing(
                XWALK_parseNoArgumentRequest(request.arguments, "servo-zeroing does not accept arguments"));
        }
        else if (request.command == XWALK_CNTRL_COMPUTER_VISION_REQ)
        {
            return controller.XWALK_handlerComputerVision(
                XWALK_parseNoArgumentRequest(request.arguments, "computer-vision accepts no arguments"));
        }
        else if (request.command == XWALK_CNTRL_RECORD_VIDEO_REQ)
        {
            return controller.XWALK_handlerVideoRecording(
                XWALK_parseNoArgumentRequest(request.arguments, "record-video accepts no arguments"));
        }
        else if (request.command == XWALK_CNTRL_VIDEO_STREAM_REQ)
        {
            return controller.XWALK_handlerVideoStreaming(
                XWALK_parseNoArgumentRequest(request.arguments, "video-stream accepts no arguments"));
        }
        else if (request.command == XWALK_CNTRL_SOUND_BACKGROUND_MUSIC_REQ)
        {
            return controller.XWALK_handlerSoundBackgroundMusic(
                XWALK_parseNoArgumentRequest(request.arguments, "sound-background-music accepts no arguments"));
        }
        else if (request.command == XWALK_CNTRL_TEXT_VISION_TALK_REQ)
        {
            return controller.XWALK_handlerTextVisionTalk(
                XWALK_parseLifecycleRequest(request.arguments, "text-vision-talk requires exactly start or stop"));
        }
        else if (request.command == XWALK_CNTRL_ONLINE_LLM_TEST_REQ)
        {
            return controller.XWALK_handlerOnlineLlmTest(
                XWALK_parseLifecycleRequest(request.arguments, "online-llm-test requires exactly start or stop"));
        }
        else if (controller.picarxObject == nullptr)
        {
            controller.output("PiCar-X backend unavailable");
            return 3;
        }
        else
        {
            controller.picarxObject->clearEmergencyStop();
            agent::XWalkPicarxSafetyGuard safetyGuard(*controller.picarxObject);
            return XWALK_runPicarxControllerCommand(controller, request);
        }
    }

} /* namespace xwalk::ctrl */
