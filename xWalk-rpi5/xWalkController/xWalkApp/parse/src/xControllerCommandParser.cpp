/******************************************************************************
 * @file        xControllerCommandParser.cpp
 * @brief       Implements top-level Controller command parsing.
 *
 * @details
 * Maps one validated top-level command name to the shared typed command
 *request.
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

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl {

/**
 * @brief Parses one non-empty top-level command into the shared request type.
 * @param[in] arguments Command arguments excluding the executable name.
 * @return Owned arguments and the recognized command, or `Unknown`.
 */
XWalkControllerCommandRequest
XWALK_parseControllerCommand(const ::ctrl::stringvector &arguments) {
  const ::ctrl::boolean argumentsEmpty =
      static_cast<::ctrl::boolean>(arguments.empty());
  if (argumentsEmpty) {
    XWALK_CTRL_ERROR(XWALK_INVAL, "PiCar-X CLI command is required");
  }

  ::ctrl::uint16 command{XWALK_CNTRL_UNKNOWN_REQ};
  if ((arguments[0U] == "-h") || (arguments[0U] == "--help") ||
      (arguments[0U] == "help")) {
    command = XWALK_CNTRL_HELP_REQ;
  } else if (arguments[0U] == "spi") {
    command = XWALK_CNTRL_SPI_REQ;
  } else if (arguments[0U] == "doctor") {
    command = XWALK_CNTRL_DOCTOR_REQ;
  } else if (arguments[0U] == "servo-zeroing") {
    command = XWALK_CNTRL_SERVO_ZEROING_REQ;
  } else if (arguments[0U] == "computer-vision") {
    command = XWALK_CNTRL_COMPUTER_VISION_REQ;
  } else if (arguments[0U] == "record-video") {
    command = XWALK_CNTRL_RECORD_VIDEO_REQ;
  } else if (arguments[0U] == "sound-background-music") {
    command = XWALK_CNTRL_SOUND_BACKGROUND_MUSIC_REQ;
  } else if (arguments[0U] == "text-vision-talk") {
    command = XWALK_CNTRL_TEXT_VISION_TALK_REQ;
  } else if (arguments[0U] == "online-llm-test") {
    command = XWALK_CNTRL_ONLINE_LLM_TEST_REQ;
  } else if (arguments[0U] == "move") {
    command = XWALK_CNTRL_MOVE_REQ;
  } else if (arguments[0U] == "keyboard-control") {
    command = XWALK_CNTRL_KEYBOARD_CONTROL_REQ;
  } else if (arguments[0U] == "avoid-obstacles") {
    command = XWALK_CNTRL_AVOID_OBSTACLES_REQ;
  } else if (arguments[0U] == "cliff-detection") {
    command = XWALK_CNTRL_CLIFF_DETECTION_REQ;
  } else if (arguments[0U] == "stare-at-you") {
    command = XWALK_CNTRL_STARE_AT_YOU_REQ;
  } else if (arguments[0U] == "bull-fight") {
    command = XWALK_CNTRL_BULL_FIGHT_REQ;
  } else if (arguments[0U] == "treasure-hunt") {
    command = XWALK_CNTRL_TREASURE_HUNT_REQ;
  } else if (arguments[0U] == "video-car") {
    command = XWALK_CNTRL_VIDEO_CAR_REQ;
  } else if (arguments[0U] == "app-control") {
    command = XWALK_CNTRL_APP_CONTROL_REQ;
  } else if (arguments[0U] == "turn") {
    command = XWALK_CNTRL_TURN_REQ;
  } else if (arguments[0U] == "cam") {
    command = XWALK_CNTRL_CAMERA_REQ;
  } else if (arguments[0U] == "sensor") {
    command = XWALK_CNTRL_SENSOR_REQ;
  } else if (arguments[0U] == "line-track") {
    command = XWALK_CNTRL_LINE_TRACK_REQ;
  } else if (arguments[0U] == "self-drive") {
    command = XWALK_CNTRL_SELF_DRIVE_REQ;
  } else if (arguments[0U] == "sound") {
    command = XWALK_CNTRL_SOUND_REQ;
  } else if (arguments[0U] == "voice-chat") {
    command = XWALK_CNTRL_VOICE_CHAT_REQ;
  } else if ((arguments[0U] == "voice-active-car") ||
             (arguments[0U] == "voice-active-car-gpt")) {
    command = XWALK_CNTRL_VOICE_ACTIVE_CAR_REQ;
  } else if (arguments[0U] == "gpt-car") {
    command = XWALK_CNTRL_GPT_CAR_REQ;
  } else if (arguments[0U] == "voice-controlled-car") {
    command = XWALK_CNTRL_VOICE_CONTROLLED_CAR_REQ;
  } else if (arguments[0U] == "voice-prompt-car") {
    command = XWALK_CNTRL_VOICE_PROMPT_CAR_REQ;
  } else if (arguments[0U] == "storytelling-robot") {
    command = XWALK_CNTRL_STORYTELLING_ROBOT_REQ;
  } else if (arguments[0U] == "calibrate") {
    command = XWALK_CNTRL_CALIBRATE_REQ;
  } else {
    /* Unsupported commands retain the explicit Unknown value. */
  }
  return {command, arguments};
}

} /* namespace xwalk::ctrl */
