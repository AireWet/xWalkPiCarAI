/******************************************************************************
 * @file        xControllerHelp.h
 * @brief       Declares source-visible PiCar-X CLI help text.
 *
 * @details
 * Selects the JSON-generated help declaration for official CMake targets and
 * provides an equivalent fallback for editors and direct source parsing.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-07-31
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XCONTROLLER_HELP_H
#define XCONTROLLER_HELP_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

#if defined(XWALK_CONTROLLER_USE_GENERATED_HELP)
#include "xControllerHelpGenerated.h"
#else

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::ctrl
{

/**
 * @brief Linux-style command help available without a configured build tree.
 *
 * @details
 * Official CMake targets replace this fallback with the declaration generated
 * from `xWalkApp/activate/resources/help.json`.
 */
inline constexpr ::ctrl::cstring XCONTROLLER_HELP =
    "Usage:\n"
    "  xwalk-picarx-control [GLOBAL-OPTIONS] [COMMAND]\n"
    "  xwalk-picarx-control <command> [options]\n"
    "  xwalk-picarx-control {-h|--help|help}\n"
    "\n"
    "Commands:\n"
    "  doctor                   Inspect deployment without claiming actuator outputs.\n"
    "  move <forward|backward>  Drive for a bounded duration, then stop.\n"
    "  turn <left|right>        Perform a fixed turn sequence.\n"
    "  cam <pan|tilt>           Set one camera-servo angle.\n"
    "  sensor <type>            Read distance or grayscale sensors.\n"
    "  line-track <start|stop>  Track a line in foreground or stop the motors.\n"
    "  self-drive <action-name> Run one named preset gesture, movement, or sound.\n"
    "  sound <operation>        Play audio, set volume, play music, or stop audio.\n"
    "  calibrate                Interactively calibrate steering and camera servos.\n"
    "\n"
    "Options:\n"
    "  --deployment-config PATH Absolute Raspberry Pi deployment configuration.\n"
    "  --resource-directory PATH Absolute packaged-resource directory.\n"
    "  --trace VALUE            Apply one ordered trace setting before Controller boot.\n"
    "                           RPI, CTRL, RPIAGENT, or LIB numeric trace selector\n"
    "                           Example: RPIAGENT.<digits>.enable or LIB.<digits>.disable\n"
    "                           Module selectors accept the same tags and matching state\n"
    "                           all.enable or all.disable\n"
    "                           FILE.json for grouped global, module, and tag states\n"
    "                           Trace IDs must be unique across the compiled project.\n"
    "                           New traces are disabled; saved XML states load automatically.\n"
    "  -h, --help               Show this help text and exit.\n"
    "  --speed N                Movement speed from 0 through 100 percent; default 50.\n"
    "  --duration S             Movement duration in seconds; default 1.0.\n"
    "  --angle N                Turn, pan, or tilt angle in degrees.\n"
    "  --volume N               Audio volume from 0 through 100 percent.\n"
    "\n"
    "Self-drive actions:\n"
    "  shake-head               Run the decreasing head-shake gesture.\n"
    "  nod                      Run the repeated camera-tilt nod gesture.\n"
    "  wave-hands               Run the steering wave gesture.\n"
    "  resist                   Run the steering and camera resistance gesture.\n"
    "  act-cute                 Run the low-speed cute-shaking gesture.\n"
    "  rub-hands                Run the small steering-angle rubbing gesture.\n"
    "  think                    Run the thinking pose and return to center.\n"
    "  twist-body               Run the motor, steering, and camera twist gesture.\n"
    "  celebrate                Run the mirrored celebration gesture.\n"
    "  depressed                Run the downward camera-tilt gesture.\n"
    "  forward                  Drive forward briefly and stop.\n"
    "  backward                 Drive backward briefly and stop.\n"
    "  honking                  Play the horn sound in the background.\n"
    "  start-engine             Play the engine-start sound in the background.\n"
    "\n"
    "Examples:\n"
    "  xwalk-picarx-control --trace RPI.001.enable\n"
    "  xwalk-picarx-control --trace CTRL.001.disable doctor\n"
    "  xwalk-picarx-control --trace RPI.enable\n"
    "  xwalk-picarx-control --trace CTRL.disable\n"
    "  xwalk-picarx-control --trace RPIAGENT.enable\n"
    "  xwalk-picarx-control --trace LIB.disable\n"
    "  xwalk-picarx-control --trace all.enable\n"
    "  xwalk-picarx-control --trace all.disable\n"
    "  xwalk-picarx-control --trace xwalk-traces.json\n"
    "  xwalk-picarx-control doctor\n"
    "  xwalk-picarx-control move forward --speed 40 --duration 2.5\n"
    "  xwalk-picarx-control move backward --speed 25\n"
    "  xwalk-picarx-control turn left --angle 20\n"
    "  xwalk-picarx-control cam pan --angle 45\n"
    "  xwalk-picarx-control cam tilt --angle -15\n"
    "  xwalk-picarx-control sensor distance\n"
    "  xwalk-picarx-control sensor grayscale\n"
    "  xwalk-picarx-control line-track start\n"
    "  xwalk-picarx-control line-track stop\n"
    "  xwalk-picarx-control self-drive shake-head\n"
    "  xwalk-picarx-control self-drive wave-hands\n"
    "  xwalk-picarx-control self-drive act-cute\n"
    "  xwalk-picarx-control self-drive twist-body\n"
    "  xwalk-picarx-control self-drive honking\n"
    "  xwalk-picarx-control self-drive start-engine\n"
    "  xwalk-picarx-control sound play sounds/car-double-horn.wav --volume 80\n"
    "  xwalk-picarx-control sound volume 60\n"
    "  xwalk-picarx-control sound music music/slow-trail-Ahjay_Stelino.mp3 --volume 20\n"
    "  xwalk-picarx-control sound stop\n"
    "  xwalk-picarx-control calibrate";

} /* namespace xwalk::ctrl */

#endif /* XWALK_CONTROLLER_USE_GENERATED_HELP */

#endif /* XCONTROLLER_HELP_H */
