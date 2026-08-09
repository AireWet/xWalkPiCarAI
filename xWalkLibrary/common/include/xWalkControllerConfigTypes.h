/******************************************************************************
 * @file        xWalkControllerConfigTypes.h
 * @brief       Declares shared Controller configuration and request data.
 *
 * @details
 * Contains plain, application-owned values shared by xWalkApp and xWalkHandler.
 * Runtime services, callbacks, callback contexts, and hardware state remain in
 * their owning modules.
 *
 * @project     xWalk Firmware
 * @module      xWalkLibrary Common
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

#pragma once

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerCommand.h"
#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/** @brief Selects one audio operation requested by the Controller. */
enum class XWalkSoundOperation : ::ctrl::uint8
{
    /** @brief Plays one blocking sound-effect file. */
    Play,
    /** @brief Changes the shared audio volume without requiring a file. */
    Volume,
    /** @brief Starts one streamed music file. */
    Music,
    /** @brief Stops current music playback. */
    Stop
};

/** @brief Selects whether a long-running Controller operation starts or stops. */
enum class XWalkLifecycleAction : ::ctrl::uint8
{
    /** @brief Starts and runs the selected operation. */
    Start,
    /** @brief Stops the selected operation without starting work. */
    Stop
};

/** @brief Selects one direct vehicle movement operation. */
enum class XWalkMoveAction : ::ctrl::uint8
{
    /** @brief Moves the vehicle forward. */
    Forward,
    /** @brief Moves the vehicle backward. */
    Backward,
    /** @brief Runs the bounded movement demonstration. */
    Demo
};

/** @brief Selects one steering direction. */
enum class XWalkTurnDirection : ::ctrl::uint8
{
    /** @brief Steers toward the left side. */
    Left,
    /** @brief Steers toward the right side. */
    Right
};

/** @brief Selects one camera-servo axis. */
enum class XWalkCameraAxis : ::ctrl::uint8
{
    /** @brief Selects the horizontal camera servo. */
    Pan,
    /** @brief Selects the vertical camera servo. */
    Tilt
};

/** @brief Selects one vehicle sensor report. */
enum class XWalkSensorType : ::ctrl::uint8
{
    /** @brief Reads the ultrasonic distance in centimeters. */
    Distance,
    /** @brief Samples and reports the three grayscale channels. */
    Grayscale
};

/** @brief Selects one interactive calibration workflow. */
enum class XWalkCalibrationMode : ::ctrl::uint8
{
    /** @brief Calibrates actuators and grayscale references. */
    Complete,
    /** @brief Calibrates only grayscale references. */
    Grayscale,
    /** @brief Calibrates only servos and motors. */
    ServoMotor
};

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Stores reusable paths selected for one Controller application run. */
struct XWalkAppConfig
{
    /** @brief Absolute deployment configuration selected for hardware composition. */
    ::ctrl::string configurationFilePath{};
    /** @brief Absolute resource directory selected for file-backed services. */
    ::ctrl::string resourceDirectory{};
};

/** @brief Stores validated global paths and the remaining Controller command. */
struct XWalkControllerApplicationArguments
{
    /** @brief Command arguments after application-global options are removed. */
    ::ctrl::stringvector commandArguments{};
    /** @brief Application configuration populated from defaults and global options. */
    XWalkAppConfig appConfig{};
    /** @brief Validated trace selectors and JSON paths in command-line order. */
    ::ctrl::stringvector traceArguments{};
};

/** @brief Stores one classified top-level command and its unconverted CLI text. */
struct XWalkControllerCommandRequest
{
    /** @brief Typed command signal selected at the application boundary. */
    ::ctrl::uint16 command{XWALK_CNTRL_UNKNOWN_REQ};
    /** @brief Original command arguments consumed by command-specific request parsing. */
    ::ctrl::stringvector arguments{};
};

/** @brief Represents a command that intentionally has no payload fields. */
struct XWalkNoArgumentRequest
{
};

/** @brief Describes one validated start-or-stop request. */
struct XWalkLifecycleRequest
{
    /** @brief Lifecycle action selected at the application boundary. */
    XWalkLifecycleAction action{XWalkLifecycleAction::Start};
};

/** @brief Describes one validated direct vehicle movement request. */
struct XWalkMoveRequest
{
    /** @brief Movement action selected at the application boundary. */
    XWalkMoveAction action{XWalkMoveAction::Forward};
    /** @brief Requested motor power in the inclusive range zero through one hundred percent. */
    ::ctrl::float64 speedPercent{50.0};
    /** @brief Requested movement duration in milliseconds. */
    ::ctrl::uint32 durationMs{1'000U};
};

/** @brief Describes one validated steering request. */
struct XWalkTurnRequest
{
    /** @brief Direction selected at the application boundary. */
    XWalkTurnDirection direction{XWalkTurnDirection::Left};
    /** @brief Unsigned steering magnitude in the inclusive range zero through thirty degrees. */
    ::ctrl::float64 angleDegrees{30.0};
};

/** @brief Describes one validated camera-servo request. */
struct XWalkCameraRequest
{
    /** @brief Camera axis selected at the application boundary. */
    XWalkCameraAxis axis{XWalkCameraAxis::Pan};
    /** @brief Requested axis angle in degrees within the selected servo's supported range. */
    ::ctrl::float64 angleDegrees{};
};

/** @brief Describes one validated sensor request. */
struct XWalkSensorRequest
{
    /** @brief Sensor report selected at the application boundary. */
    XWalkSensorType type{XWalkSensorType::Distance};
};

/** @brief Describes one validated named self-drive request. */
struct XWalkSelfDriveRequest
{
    /** @brief Canonical action text with words separated by spaces. */
    ::ctrl::string action{};
};

/** @brief Describes one validated full-duplex SPI transfer request. */
struct XWalkSpiRequest
{
    /** @brief Non-empty transmit bytes containing at most 256 bytes. */
    ::ctrl::bytevector transmitData{};
};

/** @brief Describes one validated GPT-car lifecycle and input-source request. */
struct XWalkGptCarRequest
{
    /** @brief Whether the GPT-car operation starts or stops. */
    XWalkLifecycleAction action{XWalkLifecycleAction::Start};
    /** @brief Whether prompts are read from the keyboard instead of speech input. */
    ::ctrl::boolean keyboardInput{false};
    /** @brief Whether captured images are included with prompts. */
    ::ctrl::boolean withImage{true};
};

/** @brief Describes one validated calibration workflow request. */
struct XWalkCalibrationRequest
{
    /** @brief Calibration workflow selected at the application boundary. */
    XWalkCalibrationMode mode{XWalkCalibrationMode::Complete};
};

/** @brief Describes one validated platform audio request. */
struct XWalkSoundRequest
{
    /** @brief Requested sound action. */
    XWalkSoundOperation operation{XWalkSoundOperation::Stop};
    /** @brief File path for play or music, or empty text otherwise. */
    ::ctrl::string filePath{};
    /** @brief Optional volume in the inclusive range zero through one hundred percent. */
    ::ctrl::optionalfloat64 volumePercent{};
};

/** @brief Describes one interactive servo-offset calibration operation. */
struct XWalkServoCalibrationConfig
{
    /** @brief Section title written before prompting. */
    ::ctrl::string title{};
    /** @brief Input prompt including the supported range. */
    ::ctrl::string prompt{};
    /** @brief Inclusive calibration minimum in degrees. */
    ::ctrl::float64 minimumAngleDegrees{};
    /** @brief Inclusive calibration maximum in degrees. */
    ::ctrl::float64 maximumAngleDegrees{};
    /** @brief Zero for steering, one for pan, or two for tilt. */
    ::ctrl::uint8 servoId{};
};

} /* namespace xwalk::ctrl */
