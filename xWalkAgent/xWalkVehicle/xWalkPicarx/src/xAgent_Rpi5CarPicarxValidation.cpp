/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxValidation.cpp
 * @brief       Implements PiCar-X coordinator parsing and validation helpers.
 *
 * @details
 * Validates finite values and the three-element list format used by the upstream Python configuration.
 *
 * @project     xWalk Firmware
 * @module      xWalkPicarx
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

#include "xAgent_Rpi5CarPicarx.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarMath.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains application coordinators for the xWalk firmware. */
namespace xwalk::agent
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Restricts one finite value to an inclusive range.
 * @param[in] value Value to restrict.
 * @param[in] minimum Inclusive lower bound.
 * @param[in] maximum Inclusive upper bound.
 * @return Restricted value.
 */
agent::float64 XWalkPicarx::constrain(agent::float64 value, agent::float64 minimum, agent::float64 maximum)
{
    const agent::boolean valueNotFinite =
        static_cast<agent::boolean>(
            !XHAL_IS_FINITE(value));
    if (valueNotFinite)
    {
        XHAL_THROW_INVALID_ARGUMENT("PiCar-X value must be finite");
    }
    if (value < minimum)
    {
        return minimum;
    }
    return (value > maximum) ? maximum : value;
}

/**
 * @brief Parses one complete finite floating-point configuration value.
 * @param[in] text Configuration text.
 * @param[in] name Non-null field name used in validation errors.
 * @return Parsed finite value.
 */
agent::float64 XWalkPicarx::parseFloat(agent::stringview text, agent::cstring name)
{
    agent::size parsedLength{};
    const agent::float64 value = hal::common::parseFloat64(text, parsedLength);
    const agent::boolean parsedLengthTextValueInvalid =
        static_cast<agent::boolean>(
            (parsedLength != text.size()) || !XHAL_IS_FINITE(value));
    if (parsedLengthTextValueInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " must be one finite number");
    }
    return value;
}

/**
 * @brief Parses exactly three signed integer values enclosed in square brackets.
 * @param[in] text Python-compatible list text.
 * @param[in] name Non-null field name used in validation errors.
 * @return Parsed three-element value.
 */
hal::linetrackervalues XWalkPicarx::parseReferences(agent::stringview text, agent::cstring name)
{
    const agent::boolean textInvalid =
        static_cast<agent::boolean>(
            (text.size() < 5U) || (text.front() != '[') || (text.back() != ']'));
    if (textInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " must be a three-element list");
    }

    hal::linetrackervalues values{};
    agent::size position{1U};
    for (agent::uint32 index = 0U; index < 3U; ++index)
    {
        const agent::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const agent::boolean whitespaceAvailable =
                static_cast<agent::boolean>(
                    (position < text.size()) && ((text[position] == ' ') || (text[position] == '\t')));
            if (whitespaceAvailable == false)
            {
                break;
            }
            ++position;
        }
        agent::size parsedLength{};
        const agent::float64 parsed = hal::common::parseFloat64(text.substr(position), parsedLength);
        const agent::float64 rounded = XHAL_ROUND_NEAREST(parsed);
        const agent::boolean parsedRoundedInvalid =
            static_cast<agent::boolean>(
                !XHAL_IS_FINITE(parsed) || (parsed != rounded) ||
            (rounded < static_cast<agent::float64>(hal::common::INT32_MINIMUM)) ||
            (rounded > static_cast<agent::float64>(hal::common::INT32_MAXIMUM)));
        if (parsedRoundedInvalid)
        {
            XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " must contain signed integers");
        }
        values[index] = static_cast<agent::int32>(rounded);
        position += parsedLength;
        const agent::boolean trailingWhitespaceParsingRequested{true};
        while (trailingWhitespaceParsingRequested)
        {
            const agent::boolean positionTextTInvalid =
                static_cast<agent::boolean>(
                    (position < text.size()) && ((text[position] == ' ') || (text[position] == '\t')));
            if (positionTextTInvalid == false)
            {
                break;
            }
            ++position;
        }
        const char expected = (index < 2U) ? ',' : ']';
        const agent::boolean positionTextExpectedInvalid =
            static_cast<agent::boolean>(
                (position >= text.size()) || (text[position] != expected));
        if (positionTextExpectedInvalid)
        {
            XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " must contain exactly three values");
        }
        ++position;
    }
    const agent::boolean positionTextDifferent =
        static_cast<agent::boolean>(
            position != text.size());
    if (positionTextDifferent)
    {
        XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " contains trailing data");
    }
    return values;
}

/**
 * @brief Parses exactly two motor direction values enclosed in square brackets.
 * @param[in] text Python-compatible two-element list text.
 * @return Parsed directions, each equal to 1 or -1.
 */
agent::fixedarray<agent::int32, 2U> XWalkPicarx::parseMotorDirections(agent::stringview text)
{
    const agent::boolean motorDirectionsTextInvalid =
        static_cast<agent::boolean>(
            (text.size() < 5U) || (text.front() != '[') || (text.back() != ']'));
    if (motorDirectionsTextInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("motor directions must be a two-element list");
    }
    agent::fixedarray<agent::int32, 2U> values{};
    agent::size position{1U};
    for (agent::uint32 index = 0U; index < 2U; ++index)
    {
        const agent::boolean leadingWhitespaceParsingRequested{true};
        while (leadingWhitespaceParsingRequested)
        {
            const agent::boolean leadingWhitespaceAvailable =
                static_cast<agent::boolean>(
                    (position < text.size()) && ((text[position] == ' ') || (text[position] == '\t')));
            if (leadingWhitespaceAvailable == false)
            {
                break;
            }
            ++position;
        }
        agent::size parsedLength{};
        const agent::float64 parsed = hal::common::parseFloat64(text.substr(position), parsedLength);
        if ((parsed != 1.0) && (parsed != -1.0))
        {
            XHAL_THROW_INVALID_ARGUMENT("motor directions must contain only 1 or -1");
        }
        values[index] = static_cast<agent::int32>(parsed);
        position += parsedLength;
        const agent::boolean trailingWhitespaceParsingRequested{true};
        while (trailingWhitespaceParsingRequested)
        {
            const agent::boolean trailingWhitespaceAvailable =
                static_cast<agent::boolean>(
                    (position < text.size()) && ((text[position] == ' ') || (text[position] == '\t')));
            if (trailingWhitespaceAvailable == false)
            {
                break;
            }
            ++position;
        }
        const char expected = (index == 0U) ? ',' : ']';
        const agent::boolean separatorMissing =
            static_cast<agent::boolean>(
                (position >= text.size()) || (text[position] != expected));
        if (separatorMissing)
        {
            XHAL_THROW_INVALID_ARGUMENT("motor directions must contain exactly two values");
        }
        ++position;
    }
    const agent::boolean trailingTextPresent =
        static_cast<agent::boolean>(
            position != text.size());
    if (trailingTextPresent)
    {
        XHAL_THROW_INVALID_ARGUMENT("motor directions contain trailing data");
    }
    return values;
}

/**
 * @brief Formats three signed integer values using the Python-compatible list form.
 * @param[in] values Values to format.
 * @return Owned bracketed comma-separated text.
 */
agent::string XWalkPicarx::formatReferences(const hal::linetrackervalues& values)
{
    return agent::string("[") + hal::common::int32ToString(values[0U]) + ", " +
        hal::common::int32ToString(values[1U]) + ", " + hal::common::int32ToString(values[2U]) + "]";
}

} /* namespace xwalk::agent */
