/******************************************************************************
 * @file        xControllerParsing.cpp
 * @brief       Implements PiCar-X CLI parsing and formatting.
 *
 * @details
 * Converts bounded command text into validated numeric values and stable output representations.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerParsing.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarMath.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Function definitions
 ******************************************************************************/

/**
 * @brief Parses one contiguous hexadecimal SPI payload.
 * @param[in] text Even hexadecimal digits with an optional leading `0x`.
 * @return Parsed non-empty payload containing at most 256 bytes.
 * @throws std::invalid_argument If the text is empty, odd, or non-hexadecimal.
 * @throws std::out_of_range If the payload exceeds 256 bytes.
 */
::ctrl::bytevector XWALK_parseHexBytes(::ctrl::stringview text)
{
    ::ctrl::size startIndex{};
    const ::ctrl::boolean textXInvalid =
        static_cast<::ctrl::boolean>(
            (text.size() >= 2U) && (text[0U] == '0') &&
        ((text[1U] == 'x') || (text[1U] == 'X')));
    if (textXInvalid)
    {
        startIndex = 2U;
    }
    const ::ctrl::size digitCount = text.size() - startIndex;
    if ((digitCount == 0U) || ((digitCount % 2U) != 0U))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "SPI data must contain a non-empty even number of hexadecimal digits");
    }
    const ::ctrl::size byteCount = digitCount / 2U;
    if (byteCount > XHAL_RPI5CAR_SPI_MAXIMUM_TRANSFER_BYTES)
    {
        XHAL_THROW_OUT_OF_RANGE("SPI data exceeds 256 bytes");
    }

    ::ctrl::bytevector bytes;
    bytes.reserve(byteCount);
    for (::ctrl::size index = startIndex; index < text.size(); index += 2U)
    {
        ::ctrl::uint8 value{};
        for (::ctrl::size digitIndex = index; digitIndex < (index + 2U); ++digitIndex)
        {
            const char character = text[digitIndex];
            ::ctrl::uint8 digit{};
            if ((character >= '0') && (character <= '9'))
            {
                digit = static_cast<::ctrl::uint8>(character - '0');
            }
            else if ((character >= 'a') && (character <= 'f'))
            {
                digit = static_cast<::ctrl::uint8>((character - 'a') + 10);
            }
            else if ((character >= 'A') && (character <= 'F'))
            {
                digit = static_cast<::ctrl::uint8>((character - 'A') + 10);
            }
            else
            {
                XHAL_THROW_INVALID_ARGUMENT("SPI data contains a non-hexadecimal digit");
            }
            value = static_cast<::ctrl::uint8>((static_cast<::ctrl::uint32>(value) * 16U) + digit);
        }
        bytes.push_back(value);
    }
    return bytes;
}

/**
 * @brief Formats bytes as uppercase space-separated hexadecimal text.
 * @param[in] bytes Non-empty received payload.
 * @return Owned uppercase text containing two digits for every byte.
 */
::ctrl::string XWALK_formatHexBytes(const ::ctrl::bytevector& bytes)
{
    constexpr char digits[] = "0123456789ABCDEF";
    ::ctrl::string outputText;
    outputText.reserve((bytes.size() * 3U) - 1U);
    for (::ctrl::size index = 0U; index < bytes.size(); ++index)
    {
        if (index > 0U)
        {
            outputText.push_back(' ');
        }
        const ::ctrl::uint8 value = bytes[index];
        outputText.push_back(digits[value >> 4U]);
        outputText.push_back(digits[value & 0x0FU]);
    }
    return outputText;
}

/**
 * @brief Validates one command that intentionally carries no payload.
 * @param[in] arguments Complete command arguments excluding the executable name.
 * @param[in] errorMessage Non-null command-specific validation message.
 * @return Empty typed request after validation succeeds.
 */
XWalkNoArgumentRequest XWALK_parseNoArgumentRequest(
    const ::ctrl::stringvector& arguments, ::ctrl::cstring errorMessage)
{
    const ::ctrl::boolean argumentsDifferent =
        static_cast<::ctrl::boolean>(
            arguments.size() != 1U);
    if (argumentsDifferent)
    {
        XHAL_THROW_INVALID_ARGUMENT(errorMessage);
    }
    return {};
}

/**
 * @brief Parses one command containing exactly a start or stop action.
 * @param[in] arguments Complete command arguments excluding the executable name.
 * @param[in] errorMessage Non-null command-specific validation message.
 * @return Strongly typed lifecycle request.
 */
XWalkLifecycleRequest XWALK_parseLifecycleRequest(
    const ::ctrl::stringvector& arguments, ::ctrl::cstring errorMessage)
{
    const ::ctrl::boolean argumentsInvalid =
        static_cast<::ctrl::boolean>(
            (arguments.size() != 2U) ||
        ((arguments[1U] != "start") && (arguments[1U] != "stop")));
    if (argumentsInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT(errorMessage);
    }
    const XWalkLifecycleAction action = (arguments[1U] == "start") ?
        XWalkLifecycleAction::Start : XWalkLifecycleAction::Stop;
    return {action};
}

/**
 * @brief Parses one direct vehicle movement request.
 * @param[in] arguments Move action followed by optional speed and duration options.
 * @return Validated movement request with duration represented in milliseconds.
 */
XWalkMoveRequest XWALK_parseMoveRequest(const ::ctrl::stringvector& arguments)
{
    const ::ctrl::boolean argumentsTooSmall =
        static_cast<::ctrl::boolean>(
            arguments.size() < 2U);
    if (argumentsTooSmall)
    {
        XHAL_THROW_INVALID_ARGUMENT("move requires forward, backward, or demo");
    }
    if (arguments[1U] == "demo")
    {
        const ::ctrl::boolean unexpectedDemoArguments =
            static_cast<::ctrl::boolean>(
                arguments.size() != 2U);
        if (unexpectedDemoArguments)
        {
            XHAL_THROW_INVALID_ARGUMENT("move demo accepts no options");
        }
        return {XWalkMoveAction::Demo, 50.0, 1'000U};
    }

    XWalkMoveAction action{XWalkMoveAction::Forward};
    if (arguments[1U] == "forward")
    {
        action = XWalkMoveAction::Forward;
    }
    else if (arguments[1U] == "backward")
    {
        action = XWalkMoveAction::Backward;
    }
    else
    {
        XHAL_THROW_INVALID_ARGUMENT("move action must be forward, backward, or demo");
    }
    const controlleroptions options = XWALK_parseOptions(arguments, 2U);
    XWALK_validateOptions(options, {"speed", "duration"});
    const ::ctrl::float64 speedPercent = XWALK_parseNumber(
        XWALK_optionValue(options, "speed", "50", false), "move speed", 0.0, 100.0);
    const ::ctrl::float64 durationSeconds = XWALK_parseNumber(
        XWALK_optionValue(options, "duration", "1.0", false),
        "move duration", 0.0, 4'294'967.295);
    return {action, speedPercent, XWALK_durationMilliseconds(durationSeconds)};
}

/**
 * @brief Parses one steering request.
 * @param[in] arguments Turn direction followed by an optional angle.
 * @return Validated direction and unsigned angle magnitude.
 */
XWalkTurnRequest XWALK_parseTurnRequest(const ::ctrl::stringvector& arguments)
{
    const ::ctrl::boolean turnDirectionMissing =
        static_cast<::ctrl::boolean>(
            arguments.size() < 2U);
    if (turnDirectionMissing)
    {
        XHAL_THROW_INVALID_ARGUMENT("turn requires left or right");
    }
    XWalkTurnDirection direction{XWalkTurnDirection::Left};
    if (arguments[1U] == "left")
    {
        direction = XWalkTurnDirection::Left;
    }
    else if (arguments[1U] == "right")
    {
        direction = XWalkTurnDirection::Right;
    }
    else
    {
        XHAL_THROW_INVALID_ARGUMENT("turn direction must be left or right");
    }
    const controlleroptions options = XWALK_parseOptions(arguments, 2U);
    XWALK_validateOptions(options, {"angle"});
    const ::ctrl::float64 angleDegrees = XWALK_parseNumber(
        XWALK_optionValue(options, "angle", "30", false), "turn angle", 0.0, 30.0);
    return {direction, angleDegrees};
}

/**
 * @brief Parses one camera-servo request.
 * @param[in] arguments Camera axis followed by its required angle option.
 * @return Validated axis and angle in degrees.
 */
XWalkCameraRequest XWALK_parseCameraRequest(const ::ctrl::stringvector& arguments)
{
    const ::ctrl::boolean cameraAxisMissing =
        static_cast<::ctrl::boolean>(
            arguments.size() < 2U);
    if (cameraAxisMissing)
    {
        XHAL_THROW_INVALID_ARGUMENT("cam requires pan or tilt");
    }
    const controlleroptions options = XWALK_parseOptions(arguments, 2U);
    XWALK_validateOptions(options, {"angle"});
    const ::ctrl::string angleText = XWALK_optionValue(options, "angle", {}, true);
    if (arguments[1U] == "pan")
    {
        return {XWalkCameraAxis::Pan,
            XWALK_parseNumber(angleText, "camera pan angle", -90.0, 90.0)};
    }
    if (arguments[1U] == "tilt")
    {
        return {XWalkCameraAxis::Tilt,
            XWALK_parseNumber(angleText, "camera tilt angle", -35.0, 65.0)};
    }
    XHAL_THROW_INVALID_ARGUMENT("camera action must be pan or tilt");
}

/**
 * @brief Parses one sensor report request.
 * @param[in] arguments Sensor command followed by exactly one supported type.
 * @return Strongly typed sensor selection.
 */
XWalkSensorRequest XWALK_parseSensorRequest(const ::ctrl::stringvector& arguments)
{
    const ::ctrl::boolean sensorArgumentCountInvalid =
        static_cast<::ctrl::boolean>(
            arguments.size() != 2U);
    if (sensorArgumentCountInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("sensor requires exactly one type");
    }
    if (arguments[1U] == "distance")
    {
        return {XWalkSensorType::Distance};
    }
    if (arguments[1U] == "grayscale")
    {
        return {XWalkSensorType::Grayscale};
    }
    XHAL_THROW_INVALID_ARGUMENT("sensor type must be distance or grayscale");
}

/**
 * @brief Parses one named self-drive action.
 * @param[in] arguments Command followed by one hyphenated action or separate action words.
 * @return Request containing canonical space-separated action text.
 */
XWalkSelfDriveRequest XWALK_parseSelfDriveRequest(
    const ::ctrl::stringvector& arguments)
{
    const ::ctrl::boolean selfDriveActionMissing =
        static_cast<::ctrl::boolean>(
            arguments.size() < 2U);
    if (selfDriveActionMissing)
    {
        XHAL_THROW_INVALID_ARGUMENT("self-drive requires an action");
    }
    ::ctrl::string action = arguments[1U];
    for (::ctrl::size index = 2U; index < arguments.size(); ++index)
    {
        action += " ";
        action += arguments[index];
    }
    std::replace(action.begin(), action.end(), '-', ' ');
    return {action};
}

/**
 * @brief Parses one sound operation request.
 * @param[in] arguments Sound operation, payload, and optional volume.
 * @return Validated sound request.
 */
XWalkSoundRequest XWALK_parseSoundRequest(const ::ctrl::stringvector& arguments)
{
    const ::ctrl::boolean soundOperationMissing =
        static_cast<::ctrl::boolean>(
            arguments.size() < 2U);
    if (soundOperationMissing)
    {
        XHAL_THROW_INVALID_ARGUMENT("sound requires an operation");
    }
    XWalkSoundRequest request;
    ::ctrl::size optionIndex{2U};
    if ((arguments[1U] == "play") || (arguments[1U] == "music"))
    {
        const ::ctrl::boolean soundFileMissing =
            static_cast<::ctrl::boolean>(
                (arguments.size() < 3U) || (arguments[2U].rfind("--", 0U) == 0U));
        if (soundFileMissing)
        {
            XHAL_THROW_INVALID_ARGUMENT("sound play and music require a file");
        }
        request.operation = (arguments[1U] == "play") ?
            XWalkSoundOperation::Play : XWalkSoundOperation::Music;
        request.filePath = arguments[2U];
        optionIndex = 3U;
    }
    else if (arguments[1U] == "volume")
    {
        const ::ctrl::boolean volumeArgumentCountInvalid =
            static_cast<::ctrl::boolean>(
                arguments.size() != 3U);
        if (volumeArgumentCountInvalid)
        {
            XHAL_THROW_INVALID_ARGUMENT("sound volume requires one value");
        }
        request.operation = XWalkSoundOperation::Volume;
        request.volumePercent = XWALK_parseNumber(arguments[2U], "sound volume", 0.0, 100.0);
        optionIndex = arguments.size();
    }
    else if (arguments[1U] != "stop")
    {
        XHAL_THROW_INVALID_ARGUMENT("sound operation is not supported");
    }

    const controlleroptions options = XWALK_parseOptions(arguments, optionIndex);
    XWALK_validateOptions(options, {"volume"});
    const ::ctrl::boolean optionsCountVolumeDifferent =
        static_cast<::ctrl::boolean>(
            options.count("volume") != 0U);
    if (optionsCountVolumeDifferent)
    {
        request.volumePercent = XWALK_parseNumber(
            XWALK_optionValue(options, "volume", {}, true), "sound volume", 0.0, 100.0);
    }
    else {
        const ::ctrl::boolean requestValid =
            static_cast<::ctrl::boolean>(
                (request.operation == XWalkSoundOperation::Music) &&
            !request.volumePercent.has_value());
            if (requestValid)
    {
        request.volumePercent = 20.0;
    }
    }
    return request;
}

/**
 * @brief Parses one bounded full-duplex SPI transfer request.
 * @param[in] arguments Exact transfer action followed by hexadecimal bytes.
 * @return Parsed transmit bytes.
 */
XWalkSpiRequest XWALK_parseSpiRequest(const ::ctrl::stringvector& arguments)
{
    const ::ctrl::boolean argumentsTransferInvalid =
        static_cast<::ctrl::boolean>(
            (arguments.size() != 3U) || (arguments[1U] != "transfer"));
    if (argumentsTransferInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "spi requires transfer followed by one hexadecimal payload");
    }
    return {XWALK_parseHexBytes(arguments[2U])};
}

/**
 * @brief Parses one GPT-car lifecycle and input-source request.
 * @param[in] arguments Start or stop followed by optional keyboard and no-image flags.
 * @return Validated GPT-car request.
 */
XWalkGptCarRequest XWALK_parseGptCarRequest(const ::ctrl::stringvector& arguments)
{
    const ::ctrl::boolean gptArgumentCountInvalid =
        static_cast<::ctrl::boolean>(
            (arguments.size() < 2U) || (arguments.size() > 4U));
    if (gptArgumentCountInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "gpt-car requires start or stop and optional --keyboard or --no-img");
    }
    if ((arguments[1U] != "start") && (arguments[1U] != "stop"))
    {
        XHAL_THROW_INVALID_ARGUMENT("gpt-car action must be start or stop");
    }
    const ::ctrl::boolean argumentsValid =
        static_cast<::ctrl::boolean>(
            (arguments[1U] == "stop") && (arguments.size() != 2U));
    if (argumentsValid)
    {
        XHAL_THROW_INVALID_ARGUMENT("gpt-car stop does not accept options");
    }
    XWalkGptCarRequest request;
    request.action = (arguments[1U] == "start") ?
        XWalkLifecycleAction::Start : XWalkLifecycleAction::Stop;
    for (::ctrl::size index = 2U; index < arguments.size(); ++index)
    {
        if (arguments[index] == "--keyboard")
        {
            request.keyboardInput = true;
        }
        else if (arguments[index] == "--no-img")
        {
            request.withImage = false;
        }
        else
        {
            XHAL_THROW_INVALID_ARGUMENT("gpt-car option is not supported");
        }
    }
    return request;
}

/**
 * @brief Parses one calibration workflow request.
 * @param[in] arguments Command with an optional supported calibration mode.
 * @return Strongly typed calibration selection.
 */
XWalkCalibrationRequest XWALK_parseCalibrationRequest(
    const ::ctrl::stringvector& arguments)
{
    const ::ctrl::boolean argumentsMatched =
        static_cast<::ctrl::boolean>(
            arguments.size() == 1U);
    if (argumentsMatched)
    {
        return {XWalkCalibrationMode::Complete};
    }
    const ::ctrl::boolean grayscaleCalibrationRequested =
        static_cast<::ctrl::boolean>(
            (arguments.size() == 2U) && (arguments[1U] == "grayscale"));
    if (grayscaleCalibrationRequested)
    {
        return {XWalkCalibrationMode::Grayscale};
    }
    const ::ctrl::boolean servoMotorCalibrationRequested =
        static_cast<::ctrl::boolean>(
            (arguments.size() == 2U) && (arguments[1U] == "servo-motor"));
    if (servoMotorCalibrationRequested)
    {
        return {XWalkCalibrationMode::ServoMotor};
    }
    XHAL_THROW_INVALID_ARGUMENT("calibrate mode must be grayscale or servo-motor");
}

/**
 * @brief Parses named options beginning at one argument index.
 * @param[in] arguments Complete command arguments excluding the executable name.
 * @param[in] startIndex First option index.
 * @return Owned option names without `--` and their values.
 */
controlleroptions XWALK_parseOptions(const ::ctrl::stringvector& arguments,
    ::ctrl::size startIndex)
{
    controlleroptions options;
    ::ctrl::size index = startIndex;
    const ::ctrl::boolean processingLoopRequested{true};
    while (processingLoopRequested)
    {
        const ::ctrl::boolean argumentAvailable =
            static_cast<::ctrl::boolean>(
                index < arguments.size());
        if (argumentAvailable == false)
        {
            break;
        }
        const ::ctrl::string& token = arguments[index];
        const ::ctrl::boolean tokenInvalid =
            static_cast<::ctrl::boolean>(
                (token.size() < 3U) || (token[0U] != '-') || (token[1U] != '-'));
        if (tokenInvalid)
        {
            XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI expected a named option");
        }
        const ::ctrl::size equals = token.find('=');
        const ::ctrl::string name = token.substr(2U, equals == ::ctrl::string::npos ? equals : equals - 2U);
        ::ctrl::string value;
        if (equals != ::ctrl::string::npos)
        {
            value = token.substr(equals + 1U);
            ++index;
        }
        else
        {
            const ::ctrl::boolean optionValueMissing =
                static_cast<::ctrl::boolean>(
                    (index + 1U) >= arguments.size());
            if (optionValueMissing)
            {
                XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI option requires a value");
            }
            value = arguments[index + 1U];
            index += 2U;
        }
        const ::ctrl::boolean nameValueOptionsInvalid =
            static_cast<::ctrl::boolean>(
                name.empty() || value.empty() || (options.count(name) != 0U));
        if (nameValueOptionsInvalid)
        {
            XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI option is empty or duplicated");
        }
        options.emplace(name, value);
    }
    return options;
}

/**
 * @brief Retrieves one option or a caller-supplied default.
 * @param[in] options Parsed option map.
 * @param[in] name Option name without `--`.
 * @param[in] defaultValue Value returned when the option is absent.
 * @param[in] required Whether absence is invalid.
 * @return Owned option or default text.
 */
::ctrl::string XWALK_optionValue(const controlleroptions& options,
    ::ctrl::stringview name, ::ctrl::stringview defaultValue, ::ctrl::boolean required)
{
    const auto option = options.find(::ctrl::string(name));
    const ::ctrl::boolean optionOptionsDifferent =
        static_cast<::ctrl::boolean>(
            option != options.end());
    if (optionOptionsDifferent)
    {
        return option->second;
    }
    if (required)
    {
        XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI required option is missing");
    }
    return ::ctrl::string(defaultValue);
}

/**
 * @brief Rejects options outside a command-specific allow list.
 * @param[in] options Parsed option map.
 * @param[in] allowed Valid option names without `--`.
 */
void XWALK_validateOptions(const controlleroptions& options,
    const ::ctrl::stringvector& allowed)
{
    for (const auto& option : options)
    {
        const ::ctrl::boolean allowedOptionFirstMatched =
            static_cast<::ctrl::boolean>(
                std::find(allowed.begin(), allowed.end(), option.first) == allowed.end());
        if (allowedOptionFirstMatched)
        {
            XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI option is not supported by this command");
        }
    }
}

/**
 * @brief Parses one complete finite number within inclusive limits.
 * @param[in] text Numeric text.
 * @param[in] name Non-null field name used in errors.
 * @param[in] minimum Inclusive minimum.
 * @param[in] maximum Inclusive maximum.
 * @return Validated numeric value.
 */
::ctrl::float64 XWALK_parseNumber(::ctrl::stringview text, ::ctrl::cstring name,
    ::ctrl::float64 minimum, ::ctrl::float64 maximum)
{
    ::ctrl::size parsedLength{};
    const ::ctrl::float64 value = hal::common::parseFloat64(text, parsedLength);
    const ::ctrl::boolean parsedLengthTextValueInvalid =
        static_cast<::ctrl::boolean>(
            (parsedLength != text.size()) || !XHAL_IS_FINITE(value));
    if (parsedLengthTextValueInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " must be one finite number");
    }
    if ((value < minimum) || (value > maximum))
    {
        XHAL_THROW_OUT_OF_RANGE_DETAIL(name, " is outside its range");
    }
    return value;
}

/**
 * @brief Converts non-negative seconds to a bounded millisecond delay.
 * @param[in] durationSeconds Finite duration from zero through 4,294,967.295 seconds.
 * @return Rounded duration in milliseconds.
 */
::ctrl::uint32 XWALK_durationMilliseconds(::ctrl::float64 durationSeconds)
{
    const ::ctrl::float64 milliseconds = durationSeconds * 1'000.0;
    return hal::common::roundedValue(milliseconds, "duration milliseconds", 0U,
        hal::common::UINT32_MAXIMUM);
}

/**
 * @brief Formats one sensor value with one fractional decimal digit.
 * @param[in] value Finite value whose tenths fit the signed 32-bit range.
 * @return Owned decimal text with exactly one fractional digit.
 */
::ctrl::string XWALK_formatOneDecimal(::ctrl::float64 value)
{
    const ::ctrl::float64 tenthsValue = XHAL_ROUND_NEAREST(value * 10.0);
    const ::ctrl::boolean tenthsInvalid =
        static_cast<::ctrl::boolean>(
            !XHAL_IS_FINITE(tenthsValue) ||
        (tenthsValue < static_cast<::ctrl::float64>(hal::common::INT32_MINIMUM)) ||
        (tenthsValue > static_cast<::ctrl::float64>(hal::common::INT32_MAXIMUM)));
    if (tenthsInvalid)
    {
        XHAL_THROW_OUT_OF_RANGE("PiCar-X CLI sensor value cannot be formatted");
    }
    const ::ctrl::int32 tenths = static_cast<::ctrl::int32>(tenthsValue);
    const ::ctrl::int32 whole = tenths / 10;
    const ::ctrl::int32 digit = static_cast<::ctrl::int32>(XHAL_ABSOLUTE_VALUE(tenths % 10));
    return hal::common::int32ToString(whole) + "." + hal::common::int32ToString(digit);
}

/**
 * @brief Formats three signed sensor counts in bracketed list form.
 * @param[in] values Left, middle, and right values.
 * @return Owned bracketed list.
 */
::ctrl::string XWALK_formatValues(const hal::linetrackervalues& values)
{
    return ::ctrl::string("[") + hal::common::int32ToString(values[0U]) + ", " +
        hal::common::int32ToString(values[1U]) + ", " + hal::common::int32ToString(values[2U]) + "]";
}

/**
 * @brief Formats three binary statuses in bracketed list form.
 * @param[in] status Left, middle, and right statuses.
 * @return Owned bracketed list.
 */
::ctrl::string XWALK_formatStatus(const hal::linetrackerstatus& status)
{
    return ::ctrl::string("[") + hal::common::uint32ToString(status[0U]) + ", " +
        hal::common::uint32ToString(status[1U]) + ", " + hal::common::uint32ToString(status[2U]) + "]";
}

/**
 * @brief Formats one line-tracking decision using the upstream example names.
 * @param[in] state Classified line-tracking state.
 * @return Lowercase stop, forward, left, or right text.
 */
::ctrl::string XWALK_formatLineTrackingState(
    agent::XWalkLineTrackingState state)
{
    ::ctrl::string result{"stop"};
    if (state == agent::XWalkLineTrackingState::Forward)
    {
        result = "forward";
    }
    else if (state == agent::XWalkLineTrackingState::Left)
    {
        result = "left";
    }
    else if (state == agent::XWalkLineTrackingState::Right)
    {
        result = "right";
    }
    return result;
}

/**
 * @brief Formats one detected object's center and size.
 * @param[in] detection Non-empty detection geometry.
 * @return Source-compatible coordinate and size text.
 */
::ctrl::string XWALK_formatDetection(
    const agent::XWalkComputerVisionDetection& detection)
{
    return "Coordinate:(" + hal::common::int32ToString(detection.centerX) +
        ", " + hal::common::int32ToString(detection.centerY) + ") Size (" +
        hal::common::uint32ToString(detection.width) + ", " +
        hal::common::uint32ToString(detection.height) + ")";
}

} /* namespace xwalk::ctrl */
