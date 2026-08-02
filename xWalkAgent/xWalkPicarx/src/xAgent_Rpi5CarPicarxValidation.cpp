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
hal::float64 XWalkPicarx::constrain(hal::float64 value, hal::float64 minimum, hal::float64 maximum)
{
    if (!XHAL_IS_FINITE(value))
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
hal::float64 XWalkPicarx::parseFloat(hal::stringview text, hal::cstring name)
{
    hal::size parsedLength{};
    const hal::float64 value = hal::common::parseFloat64(text, parsedLength);
    if ((parsedLength != text.size()) || !XHAL_IS_FINITE(value))
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
hal::linetrackervalues XWalkPicarx::parseReferences(hal::stringview text, hal::cstring name)
{
    if ((text.size() < 5U) || (text.front() != '[') || (text.back() != ']'))
    {
        XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " must be a three-element list");
    }

    hal::linetrackervalues values{};
    hal::size position{1U};
    for (hal::uint32 index = 0U; index < 3U; ++index)
    {
        while ((position < text.size()) && ((text[position] == ' ') || (text[position] == '\t')))
        {
            ++position;
        }
        hal::size parsedLength{};
        const hal::float64 parsed = hal::common::parseFloat64(text.substr(position), parsedLength);
        const hal::float64 rounded = XHAL_ROUND_NEAREST(parsed);
        if (!XHAL_IS_FINITE(parsed) || (parsed != rounded) ||
            (rounded < static_cast<hal::float64>(hal::common::INT32_MINIMUM)) ||
            (rounded > static_cast<hal::float64>(hal::common::INT32_MAXIMUM)))
        {
            XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " must contain signed integers");
        }
        values[index] = static_cast<hal::int32>(rounded);
        position += parsedLength;
        while ((position < text.size()) && ((text[position] == ' ') || (text[position] == '\t')))
        {
            ++position;
        }
        const char expected = (index < 2U) ? ',' : ']';
        if ((position >= text.size()) || (text[position] != expected))
        {
            XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " must contain exactly three values");
        }
        ++position;
    }
    if (position != text.size())
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
hal::fixedarray<hal::int32, 2U> XWalkPicarx::parseMotorDirections(hal::stringview text)
{
    if ((text.size() < 5U) || (text.front() != '[') || (text.back() != ']'))
    {
        XHAL_THROW_INVALID_ARGUMENT("motor directions must be a two-element list");
    }
    hal::fixedarray<hal::int32, 2U> values{};
    hal::size position{1U};
    for (hal::uint32 index = 0U; index < 2U; ++index)
    {
        while ((position < text.size()) && ((text[position] == ' ') || (text[position] == '\t')))
        {
            ++position;
        }
        hal::size parsedLength{};
        const hal::float64 parsed = hal::common::parseFloat64(text.substr(position), parsedLength);
        if ((parsed != 1.0) && (parsed != -1.0))
        {
            XHAL_THROW_INVALID_ARGUMENT("motor directions must contain only 1 or -1");
        }
        values[index] = static_cast<hal::int32>(parsed);
        position += parsedLength;
        while ((position < text.size()) && ((text[position] == ' ') || (text[position] == '\t')))
        {
            ++position;
        }
        const char expected = (index == 0U) ? ',' : ']';
        if ((position >= text.size()) || (text[position] != expected))
        {
            XHAL_THROW_INVALID_ARGUMENT("motor directions must contain exactly two values");
        }
        ++position;
    }
    if (position != text.size())
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
hal::string XWalkPicarx::formatReferences(const hal::linetrackervalues& values)
{
    return hal::string("[") + hal::common::int32ToString(values[0U]) + ", " +
        hal::common::int32ToString(values[1U]) + ", " + hal::common::int32ToString(values[2U]) + "]";
}

} /* namespace xwalk::agent */
