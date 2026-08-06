/******************************************************************************
 * @file        xControllerParsing.h
 * @brief       Declares Controller command parsing and result formatting.
 *
 * @details
 * Exposes application-boundary conversion from CLI text to typed command
 * requests and stable formatting used by Controller handlers.
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

#ifndef XCONTROLLER_PARSING_H
#define XCONTROLLER_PARSING_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerTypes.h"

#include "xAgent_Rpi5CarComputerVisionTypes.h"
#include "xAgent_Rpi5CarLineTrackingTypes.h"

#include "xHal_Rpi5CarLineTrackerTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command parsing and formatting for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Function declarations
 ******************************************************************************/

/**
 * @brief Parses one contiguous hexadecimal SPI payload.
 * @param[in] text Even hexadecimal digits with an optional leading `0x`.
 * @return Parsed non-empty payload containing at most 256 bytes.
 * @throws std::invalid_argument If the text is empty, odd, or non-hexadecimal.
 * @throws std::out_of_range If the payload exceeds 256 bytes.
 */
::ctrl::bytevector XWALK_parseHexBytes(::ctrl::stringview text);
/**
 * @brief Formats bytes as uppercase space-separated hexadecimal text.
 * @param[in] bytes Non-empty received payload.
 * @return Owned uppercase text containing two digits for every byte.
 */
::ctrl::string XWALK_formatHexBytes(const ::ctrl::bytevector& bytes);
/**
 * @brief Validates one command that intentionally carries no payload.
 * @param[in] arguments Complete command arguments excluding the executable name.
 * @param[in] errorMessage Non-null command-specific validation message.
 * @return Empty typed request after validation succeeds.
 */
XWalkNoArgumentRequest XWALK_parseNoArgumentRequest(
    const ::ctrl::stringvector& arguments, ::ctrl::cstring errorMessage);
/**
 * @brief Parses one command containing exactly a start or stop action.
 * @param[in] arguments Complete command arguments excluding the executable name.
 * @param[in] errorMessage Non-null command-specific validation message.
 * @return Strongly typed lifecycle request.
 */
XWalkLifecycleRequest XWALK_parseLifecycleRequest(
    const ::ctrl::stringvector& arguments, ::ctrl::cstring errorMessage);
/**
 * @brief Parses one direct vehicle movement request.
 * @param[in] arguments Move action followed by optional speed and duration options.
 * @return Validated movement request with duration represented in milliseconds.
 */
XWalkMoveRequest XWALK_parseMoveRequest(const ::ctrl::stringvector& arguments);
/**
 * @brief Parses one steering request.
 * @param[in] arguments Turn direction followed by an optional angle.
 * @return Validated direction and unsigned angle magnitude.
 */
XWalkTurnRequest XWALK_parseTurnRequest(const ::ctrl::stringvector& arguments);
/**
 * @brief Parses one camera-servo request.
 * @param[in] arguments Camera axis followed by its required angle option.
 * @return Validated axis and angle in degrees.
 */
XWalkCameraRequest XWALK_parseCameraRequest(const ::ctrl::stringvector& arguments);
/**
 * @brief Parses one sensor report request.
 * @param[in] arguments Sensor command followed by exactly one supported type.
 * @return Strongly typed sensor selection.
 */
XWalkSensorRequest XWALK_parseSensorRequest(const ::ctrl::stringvector& arguments);
/**
 * @brief Parses one named self-drive request.
 * @param[in] arguments Command followed by one hyphenated action or separate action words.
 * @return Request containing canonical space-separated action text.
 */
XWalkSelfDriveRequest XWALK_parseSelfDriveRequest(
    const ::ctrl::stringvector& arguments);
/**
 * @brief Parses one sound operation request.
 * @param[in] arguments Sound operation, payload, and optional volume.
 * @return Validated sound request.
 */
XWalkSoundRequest XWALK_parseSoundRequest(const ::ctrl::stringvector& arguments);
/**
 * @brief Parses one bounded full-duplex SPI transfer request.
 * @param[in] arguments Exact transfer action followed by hexadecimal bytes.
 * @return Parsed transmit bytes.
 */
XWalkSpiRequest XWALK_parseSpiRequest(const ::ctrl::stringvector& arguments);
/**
 * @brief Parses one GPT-car lifecycle and input-source request.
 * @param[in] arguments Start or stop followed by optional keyboard and no-image flags.
 * @return Validated GPT-car request.
 */
XWalkGptCarRequest XWALK_parseGptCarRequest(const ::ctrl::stringvector& arguments);
/**
 * @brief Parses one calibration workflow request.
 * @param[in] arguments Command with an optional supported calibration mode.
 * @return Strongly typed calibration selection.
 */
XWalkCalibrationRequest XWALK_parseCalibrationRequest(
    const ::ctrl::stringvector& arguments);
/**
 * @brief Parses named options beginning at one argument index.
 * @param[in] arguments Complete command arguments excluding the executable name.
 * @param[in] startIndex First option index.
 * @return Owned option names without `--` and their values.
 */
controlleroptions XWALK_parseOptions(const ::ctrl::stringvector& arguments,
    ::ctrl::size startIndex);
/**
 * @brief Retrieves one option or a caller-supplied default.
 * @param[in] options Parsed option map.
 * @param[in] name Option name without `--`.
 * @param[in] defaultValue Value returned when the option is absent.
 * @param[in] required Whether absence is invalid.
 * @return Owned option or default text.
 */
::ctrl::string XWALK_optionValue(const controlleroptions& options,
    ::ctrl::stringview name, ::ctrl::stringview defaultValue,
    ::ctrl::boolean required);
/**
 * @brief Rejects options outside a command-specific allow list.
 * @param[in] options Parsed option map.
 * @param[in] allowed Valid option names without `--`.
 */
void XWALK_validateOptions(const controlleroptions& options,
    const ::ctrl::stringvector& allowed);
/**
 * @brief Parses one complete finite number within inclusive limits.
 * @param[in] text Numeric text.
 * @param[in] name Non-null field name used in errors.
 * @param[in] minimum Inclusive minimum.
 * @param[in] maximum Inclusive maximum.
 * @return Validated numeric value.
 */
::ctrl::float64 XWALK_parseNumber(::ctrl::stringview text,
    ::ctrl::cstring name, ::ctrl::float64 minimum, ::ctrl::float64 maximum);
/**
 * @brief Converts non-negative seconds to a bounded millisecond delay.
 * @param[in] durationSeconds Finite duration from zero through 4,294,967.295 seconds.
 * @return Rounded duration in milliseconds.
 */
::ctrl::uint32 XWALK_durationMilliseconds(::ctrl::float64 durationSeconds);
/**
 * @brief Formats one sensor value with one fractional decimal digit.
 * @param[in] value Finite value whose tenths fit the signed 32-bit range.
 * @return Owned decimal text with exactly one fractional digit.
 */
::ctrl::string XWALK_formatOneDecimal(::ctrl::float64 value);
/**
 * @brief Formats three signed sensor counts in bracketed list form.
 * @param[in] values Left, middle, and right values.
 * @return Owned bracketed list.
 */
::ctrl::string XWALK_formatValues(const hal::linetrackervalues& values);
/**
 * @brief Formats three binary statuses in bracketed list form.
 * @param[in] status Left, middle, and right statuses.
 * @return Owned bracketed list.
 */
::ctrl::string XWALK_formatStatus(const hal::linetrackerstatus& status);
/**
 * @brief Formats one line-tracking decision using the upstream example names.
 * @param[in] state Classified line-tracking state.
 * @return Lowercase stop, forward, left, or right text.
 */
::ctrl::string XWALK_formatLineTrackingState(
    agent::XWalkLineTrackingState state);
/**
 * @brief Formats one detected object's center and size.
 * @param[in] detection Non-empty detection geometry.
 * @return Source-compatible coordinate and size text.
 */
::ctrl::string XWALK_formatDetection(
    const agent::XWalkComputerVisionDetection& detection);

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_PARSING_H */
