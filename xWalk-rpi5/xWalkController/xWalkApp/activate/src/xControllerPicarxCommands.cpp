/******************************************************************************
 * @file        xControllerPicarxCommands.cpp
 * @brief       Implements PiCar-X Controller application command dispatch.
 *
 * @details
 * Routes one validated command through a mutually exclusive application-level
 * ladder while retaining individual command behavior in protected handlers.
 *
 * @project     xWalk Firmware
 * @module      xWalkApp
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

#include "xControllerPicarxCommands.h"
#include "xControllerParsing.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl {

/******************************************************************************
 * Function definitions
 ******************************************************************************/

/**
 * @brief Routes one PiCar-X command to its protected Controller handler.
 *
 * @param[in,out] controller Controller whose injected dependencies remain valid
 * for the complete call.
 * @param[in] request Parsed command and validated non-empty arguments.
 *
 * @return
 * Zero on success or three when a command-specific backend is unavailable.
 *
 * @throws std::invalid_argument
 * The argument list is empty or the selected command group is not supported.
 */
::ctrl::int32
XWALK_runPicarxControllerCommand(XWalkController &controller,
                                 const XWalkControllerCommandRequest &request) {
  const ::ctrl::boolean argumentsEmpty =
      static_cast<::ctrl::boolean>(request.arguments.empty());
  if (argumentsEmpty) {
    XWALK_CTRL_ERROR(XWALK_INVAL, "PiCar-X CLI command is required");
  } else if (request.command == XWALK_CNTRL_MOVE_REQ) {
    return controller.XWALK_handlerMove(
        XWALK_parseMoveRequest(request.arguments));
  } else if (request.command == XWALK_CNTRL_KEYBOARD_CONTROL_REQ) {
    return controller.XWALK_handlerKeyboardControl(XWALK_parseNoArgumentRequest(
        request.arguments, "keyboard-control accepts no arguments"));
  } else if (request.command == XWALK_CNTRL_AVOID_OBSTACLES_REQ) {
    return controller.XWALK_handlerObstacleAvoidance(
        XWALK_parseLifecycleRequest(
            request.arguments,
            "avoid-obstacles requires exactly start or stop"));
  } else if (request.command == XWALK_CNTRL_CLIFF_DETECTION_REQ) {
    return controller.XWALK_handlerCliffDetection(XWALK_parseLifecycleRequest(
        request.arguments, "cliff-detection requires exactly start or stop"));
  } else if (request.command == XWALK_CNTRL_STARE_AT_YOU_REQ) {
    return controller.XWALK_handlerFaceTracking(XWALK_parseLifecycleRequest(
        request.arguments, "stare-at-you requires start or stop"));
  } else if (request.command == XWALK_CNTRL_BULL_FIGHT_REQ) {
    return controller.XWALK_handlerBullFight(XWALK_parseLifecycleRequest(
        request.arguments, "bull-fight requires start or stop"));
  } else if (request.command == XWALK_CNTRL_TREASURE_HUNT_REQ) {
    return controller.XWALK_handlerTreasureHunt(XWALK_parseNoArgumentRequest(
        request.arguments, "treasure-hunt accepts no arguments"));
  } else if (request.command == XWALK_CNTRL_VIDEO_CAR_REQ) {
    return controller.XWALK_handlerVideoCar(XWALK_parseNoArgumentRequest(
        request.arguments, "video-car accepts no arguments"));
  } else if (request.command == XWALK_CNTRL_APP_CONTROL_REQ) {
    return controller.XWALK_handlerAppControl(XWALK_parseLifecycleRequest(
        request.arguments, "app-control requires start or stop"));
  } else if (request.command == XWALK_CNTRL_TURN_REQ) {
    return controller.XWALK_handlerTurn(
        XWALK_parseTurnRequest(request.arguments));
  } else if (request.command == XWALK_CNTRL_CAMERA_REQ) {
    return controller.XWALK_handlerCamera(
        XWALK_parseCameraRequest(request.arguments));
  } else if (request.command == XWALK_CNTRL_SENSOR_REQ) {
    return controller.XWALK_handlerSensor(
        XWALK_parseSensorRequest(request.arguments));
  } else if (request.command == XWALK_CNTRL_LINE_TRACK_REQ) {
    return controller.XWALK_handlerLineTracking(XWALK_parseLifecycleRequest(
        request.arguments, "line-track requires exactly start or stop"));
  } else if (request.command == XWALK_CNTRL_SELF_DRIVE_REQ) {
    return controller.XWALK_handlerSelfDrive(
        XWALK_parseSelfDriveRequest(request.arguments));
  } else if (request.command == XWALK_CNTRL_SOUND_REQ) {
    return controller.XWALK_handlerSound(
        XWALK_parseSoundRequest(request.arguments));
  } else if (request.command == XWALK_CNTRL_VOICE_CHAT_REQ) {
    return controller.XWALK_handlerVoiceChat(XWALK_parseLifecycleRequest(
        request.arguments, "voice-chat requires exactly start or stop"));
  } else if (request.command == XWALK_CNTRL_VOICE_ACTIVE_CAR_REQ) {
    return controller.XWALK_handlerVoiceActiveCar(XWALK_parseLifecycleRequest(
        request.arguments, "voice-active-car requires exactly start or stop"));
  } else if (request.command == XWALK_CNTRL_GPT_CAR_REQ) {
    return controller.XWALK_handlerGptCar(
        XWALK_parseGptCarRequest(request.arguments));
  } else if (request.command == XWALK_CNTRL_VOICE_CONTROLLED_CAR_REQ) {
    return controller.XWALK_handlerVoiceControlledCar(
        XWALK_parseLifecycleRequest(
            request.arguments,
            "voice-controlled-car requires exactly start or stop"));
  } else if (request.command == XWALK_CNTRL_VOICE_PROMPT_CAR_REQ) {
    return controller.XWALK_handlerVoicePromptCar(XWALK_parseLifecycleRequest(
        request.arguments, "voice-prompt-car requires exactly start or stop"));
  } else if (request.command == XWALK_CNTRL_STORYTELLING_ROBOT_REQ) {
    return controller.XWALK_handlerStorytellingRobot(
        XWALK_parseLifecycleRequest(
            request.arguments,
            "storytelling-robot requires exactly start or stop"));
  } else if (request.command == XWALK_CNTRL_CALIBRATE_REQ) {
    return controller.XWALK_handlerCalibration(
        XWALK_parseCalibrationRequest(request.arguments));
  } else {
    XWALK_CTRL_ERROR(XWALK_INVAL, "PiCar-X CLI command is not supported");
  }
}

} /* namespace xwalk::ctrl */
