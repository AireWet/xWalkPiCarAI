/******************************************************************************
 * @file        xAgent_Rpi5CarControllerParsing.cpp
 * @brief       Implements PiCar-X CLI parsing and formatting.
 *
 * @details
 * Converts bounded command text into validated numeric values and stable output representations.
 *
 * @project     xWalk Firmware
 * @module      xWalkController
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

#include "xAgent_Rpi5CarController.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarMath.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Parses one contiguous hexadecimal SPI payload.
 * @param[in] text Even hexadecimal digits with an optional leading `0x`.
 * @return Parsed non-empty payload containing at most 256 bytes.
 * @throws std::invalid_argument If the text is empty, odd, or non-hexadecimal.
 * @throws std::out_of_range If the payload exceeds 256 bytes.
 */
hal::bytevector XWalkController::parseHexBytes(hal::stringview text)
{
    hal::size startIndex{};
    if ((text.size() >= 2U) && (text[0U] == '0') &&
        ((text[1U] == 'x') || (text[1U] == 'X')))
    {
        startIndex = 2U;
    }
    const hal::size digitCount = text.size() - startIndex;
    if ((digitCount == 0U) || ((digitCount % 2U) != 0U))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "SPI data must contain a non-empty even number of hexadecimal digits");
    }
    const hal::size byteCount = digitCount / 2U;
    if (byteCount > XHAL_RPI5CAR_SPI_MAXIMUM_TRANSFER_BYTES)
    {
        XHAL_THROW_OUT_OF_RANGE("SPI data exceeds 256 bytes");
    }

    hal::bytevector bytes;
    bytes.reserve(byteCount);
    for (hal::size index = startIndex; index < text.size(); index += 2U)
    {
        hal::uint8 value{};
        for (hal::size digitIndex = index; digitIndex < (index + 2U); ++digitIndex)
        {
            const char character = text[digitIndex];
            hal::uint8 digit{};
            if ((character >= '0') && (character <= '9'))
            {
                digit = static_cast<hal::uint8>(character - '0');
            }
            else if ((character >= 'a') && (character <= 'f'))
            {
                digit = static_cast<hal::uint8>((character - 'a') + 10);
            }
            else if ((character >= 'A') && (character <= 'F'))
            {
                digit = static_cast<hal::uint8>((character - 'A') + 10);
            }
            else
            {
                XHAL_THROW_INVALID_ARGUMENT("SPI data contains a non-hexadecimal digit");
            }
            value = static_cast<hal::uint8>((static_cast<hal::uint32>(value) * 16U) + digit);
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
hal::string XWalkController::formatHexBytes(const hal::bytevector& bytes)
{
    constexpr char digits[] = "0123456789ABCDEF";
    hal::string outputText;
    outputText.reserve((bytes.size() * 3U) - 1U);
    for (hal::size index = 0U; index < bytes.size(); ++index)
    {
        if (index > 0U)
        {
            outputText.push_back(' ');
        }
        const hal::uint8 value = bytes[index];
        outputText.push_back(digits[value >> 4U]);
        outputText.push_back(digits[value & 0x0FU]);
    }
    return outputText;
}

/**
 * @brief Parses named options beginning at one argument index.
 * @param[in] arguments Complete command arguments excluding the executable name.
 * @param[in] startIndex First option index.
 * @return Owned option names without `--` and their values.
 */
controlleroptions XWalkController::parseOptions(const hal::stringvector& arguments,
    hal::size startIndex)
{
    controlleroptions options;
    hal::size index = startIndex;
    while (index < arguments.size())
    {
        const hal::string& token = arguments[index];
        if ((token.size() < 3U) || (token[0U] != '-') || (token[1U] != '-'))
        {
            XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI expected a named option");
        }
        const hal::size equals = token.find('=');
        const hal::string name = token.substr(2U, equals == hal::string::npos ? equals : equals - 2U);
        hal::string value;
        if (equals != hal::string::npos)
        {
            value = token.substr(equals + 1U);
            ++index;
        }
        else
        {
            if ((index + 1U) >= arguments.size())
            {
                XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI option requires a value");
            }
            value = arguments[index + 1U];
            index += 2U;
        }
        if (name.empty() || value.empty() || (options.count(name) != 0U))
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
hal::string XWalkController::optionValue(const controlleroptions& options,
    hal::stringview name, hal::stringview defaultValue, hal::boolean required)
{
    const auto option = options.find(hal::string(name));
    if (option != options.end())
    {
        return option->second;
    }
    if (required)
    {
        XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI required option is missing");
    }
    return hal::string(defaultValue);
}

/**
 * @brief Rejects options outside a command-specific allow list.
 * @param[in] options Parsed option map.
 * @param[in] allowed Valid option names without `--`.
 */
void XWalkController::validateOptions(const controlleroptions& options,
    const hal::stringvector& allowed)
{
    for (const auto& option : options)
    {
        if (std::find(allowed.begin(), allowed.end(), option.first) == allowed.end())
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
hal::float64 XWalkController::parseNumber(hal::stringview text, hal::cstring name,
    hal::float64 minimum, hal::float64 maximum)
{
    hal::size parsedLength{};
    const hal::float64 value = hal::common::parseFloat64(text, parsedLength);
    if ((parsedLength != text.size()) || !XHAL_IS_FINITE(value))
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
hal::uint32 XWalkController::durationMilliseconds(hal::float64 durationSeconds)
{
    const hal::float64 milliseconds = durationSeconds * 1'000.0;
    return hal::common::roundedValue(milliseconds, "duration milliseconds", 0U,
        hal::common::UINT32_MAXIMUM);
}

/**
 * @brief Formats one sensor value with one fractional decimal digit.
 * @param[in] value Finite value whose tenths fit the signed 32-bit range.
 * @return Owned decimal text with exactly one fractional digit.
 */
hal::string XWalkController::formatOneDecimal(hal::float64 value)
{
    const hal::float64 tenthsValue = XHAL_ROUND_NEAREST(value * 10.0);
    if (!XHAL_IS_FINITE(tenthsValue) ||
        (tenthsValue < static_cast<hal::float64>(hal::common::INT32_MINIMUM)) ||
        (tenthsValue > static_cast<hal::float64>(hal::common::INT32_MAXIMUM)))
    {
        XHAL_THROW_OUT_OF_RANGE("PiCar-X CLI sensor value cannot be formatted");
    }
    const hal::int32 tenths = static_cast<hal::int32>(tenthsValue);
    const hal::int32 whole = tenths / 10;
    const hal::int32 digit = static_cast<hal::int32>(XHAL_ABSOLUTE_VALUE(tenths % 10));
    return hal::common::int32ToString(whole) + "." + hal::common::int32ToString(digit);
}

/**
 * @brief Formats three signed sensor counts in bracketed list form.
 * @param[in] values Left, middle, and right values.
 * @return Owned bracketed list.
 */
hal::string XWalkController::formatValues(const hal::linetrackervalues& values)
{
    return hal::string("[") + hal::common::int32ToString(values[0U]) + ", " +
        hal::common::int32ToString(values[1U]) + ", " + hal::common::int32ToString(values[2U]) + "]";
}

/**
 * @brief Formats three binary statuses in bracketed list form.
 * @param[in] status Left, middle, and right statuses.
 * @return Owned bracketed list.
 */
hal::string XWalkController::formatStatus(const hal::linetrackerstatus& status)
{
    return hal::string("[") + hal::common::uint32ToString(status[0U]) + ", " +
        hal::common::uint32ToString(status[1U]) + ", " + hal::common::uint32ToString(status[2U]) + "]";
}

} /* namespace xwalk::agent */
