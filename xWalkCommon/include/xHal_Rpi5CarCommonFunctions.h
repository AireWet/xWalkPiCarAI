/******************************************************************************
 * @file        xHal_Rpi5CarCommonFunctions.h
 * @brief       Declares reusable non-member functions for the xWalk HAL.
 *
 * @details
 * Provides inline validation and conversion helpers shared by hardware
 * abstraction modules without introducing a compiled common-library object.
 *
 * @project     xWalk Firmware
 * @module      xWalkCommon
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_COMMON_FUNCTIONS_H
#define XHAL_RPI5CAR_COMMON_FUNCTIONS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarMath.h"
#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::common
 * @brief Contains reusable non-member functions for xWalk HAL modules.
 */
namespace xwalk::hal::common
{

/******************************************************************************
 * Constants
 ******************************************************************************/

/** @brief Highest address representable by the seven-bit I2C protocol. */
inline constexpr uint8 I2C_MAXIMUM_SEVEN_BIT_ADDRESS{0x7FU};
/** @brief Highest value representable by the project unsigned 32-bit type. */
inline constexpr uint32 UINT32_MAXIMUM{std::numeric_limits<uint32>::max()};
/** @brief Lowest value representable by the project signed 32-bit type. */
inline constexpr int32 INT32_MINIMUM{std::numeric_limits<int32>::min()};
/** @brief Highest value representable by the project signed 32-bit type. */
inline constexpr int32 INT32_MAXIMUM{std::numeric_limits<int32>::max()};

/******************************************************************************
 * Inline function definitions
 ******************************************************************************/

/**
 * @brief Validates an I2C address against the seven-bit protocol range.
 *
 * @param[in] address
 * I2C address expected in the inclusive range `0x00` to `0x7F`.
 *
 * @throws std::out_of_range
 * If `address` exceeds the seven-bit protocol range.
 */
inline void validateI2cAddress(uint8 address)
{
    if (address > I2C_MAXIMUM_SEVEN_BIT_ADDRESS)
    {
        XHAL_THROW_OUT_OF_RANGE("I2C address must be seven-bit");
    }
}

/**
 * @brief Creates the conventional Linux GPIO name for a line offset.
 *
 * @param[in] pin
 * GPIO line offset in the range 0 through 255.
 *
 * @return
 * Owned name containing the `GPIO` prefix followed by the decimal line offset.
 */
inline string createGpioName(uint8 pin)
{
    return string("GPIO") + std::to_string(pin);
}

/**
 * @brief Parses the leading double-precision value from text.
 *
 * @param[in] text
 * Character sequence containing a numeric representation.
 *
 * @param[out] parsedLength
 * Number of characters consumed by the conversion.
 *
 * @return
 * Parsed double-precision value.
 *
 * @throws invalidargument
 * If no numeric conversion can be performed.
 *
 * @throws outofrange
 * If the represented value is outside the double-precision range.
 */
inline float64 parseFloat64(stringview text, size& parsedLength)
{
    return std::stod(string(text), &parsedLength);
}

/**
 * @brief Converts a double-precision value to decimal text.
 *
 * @param[in] value
 * Value to convert.
 *
 * @return
 * Owned decimal representation using the standard fixed fractional precision.
 */
inline string float64ToString(float64 value)
{
    return std::to_string(value);
}

/**
 * @brief Converts an unsigned 32-bit value to decimal text.
 *
 * @param[in] value
 * Value in the inclusive range zero through `UINT32_MAX`.
 *
 * @return
 * Owned base-ten representation without leading zeroes.
 */
inline string uint32ToString(uint32 value)
{
    return std::to_string(value);
}

/**
 * @brief Converts a signed 32-bit value to decimal text.
 *
 * @param[in] value
 * Value in the inclusive range `INT32_MIN` through `INT32_MAX`.
 *
 * @return
 * Owned base-ten representation with a leading minus sign when negative.
 */
inline string int32ToString(int32 value)
{
    return std::to_string(value);
}

/**
 * @brief Returns elapsed monotonic time in microseconds.
 *
 * @return
 * Non-decreasing microsecond count from the steady-clock epoch.
 */
inline uint64 monotonicMicroseconds() noexcept
{
    const auto elapsed = std::chrono::steady_clock::now().time_since_epoch();
    const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    return static_cast<uint64>(microseconds);
}

/**
 * @brief Suspends the current thread for at least the requested duration.
 *
 * @param[in] durationMs
 * Delay duration in milliseconds; zero returns without intentional delay.
 */
inline void sleepMilliseconds(uint32 durationMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
}

/**
 * @brief Suspends the current thread for at least the requested duration.
 *
 * @param[in] durationUs
 * Delay duration in microseconds; zero returns without intentional delay.
 */
inline void sleepMicroseconds(uint32 durationUs)
{
    std::this_thread::sleep_for(std::chrono::microseconds(durationUs));
}

/**
 * @brief Rounds and validates a non-negative numeric configuration value.
 *
 * @details
 * Uses the active floating-point rounding mode through `nearbyint`, validates
 * the rounded result against the inclusive limits, and converts it to the
 * project unsigned 32-bit type.
 *
 * @param[in] value
 * Floating-point value to round and validate.
 *
 * @param[in] name
 * Non-null text used to identify the value in an exception message.
 *
 * @param[in] minimum
 * Inclusive minimum permitted rounded value.
 *
 * @param[in] maximum
 * Inclusive maximum permitted rounded value.
 *
 * @return
 * The validated rounded value.
 *
 * @pre
 * `name` points to a null-terminated string that remains valid for this call,
 * and `minimum` is not greater than `maximum`.
 *
 * @throws std::invalid_argument
 * If `value` is not finite.
 *
 * @throws std::out_of_range
 * If the rounded value is outside the inclusive limits.
 */
inline uint32 roundedValue(float64 value, cstring name, uint32 minimum, uint32 maximum)
{
    if (!XHAL_IS_FINITE(value))
    {
        XHAL_THROW_INVALID_ARGUMENT_DETAIL(name, " must be finite");
    }

    const float64 roundedValue = XHAL_ROUND_NEAREST(value);
    const float64 minimumValue = static_cast<float64>(minimum);
    const float64 maximumValue = static_cast<float64>(maximum);
    if (roundedValue < minimumValue || roundedValue > maximumValue)
    {
        XHAL_THROW_OUT_OF_RANGE_DETAIL(name, " is outside its range");
    }
    return static_cast<uint32>(roundedValue);
}

} /* namespace xwalk::hal::common */

#endif /* XHAL_RPI5CAR_COMMON_FUNCTIONS_H */
