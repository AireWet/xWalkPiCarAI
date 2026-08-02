/******************************************************************************
 * @file        xHal_Rpi5CarMath.h
 * @brief       Defines common mathematical operations for xWalk HAL modules.
 *
 * @details
 * Provides project-level wrappers for finite checks, rounding, powers, sine,
 * square roots, absolute values, numeric limits, and maximum-value selection.
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

#ifndef XHAL_RPI5CAR_MATH_H
#define XHAL_RPI5CAR_MATH_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarStandardHeaders.h"

/******************************************************************************
 * Function-like macros
 ******************************************************************************/

/** @brief Reports whether a floating-point value is finite. */
#define XHAL_IS_FINITE(VALUE) (std::isfinite(VALUE))
/** @brief Rounds a floating-point value using the active rounding mode. */
#define XHAL_ROUND_NEAREST(VALUE) (std::nearbyint(VALUE))
/** @brief Computes the non-negative square root of a numeric value. */
#define XHAL_SQUARE_ROOT(VALUE) (std::sqrt(VALUE))
/** @brief Raises a numeric base to a numeric exponent. */
#define XHAL_POWER(BASE, EXPONENT) (std::pow((BASE), (EXPONENT)))
/** @brief Computes the sine of an angle expressed in radians. */
#define XHAL_SINE(ANGLE_RADIANS) (std::sin(ANGLE_RADIANS))
/** @brief Computes the absolute magnitude of a numeric value. */
#define XHAL_ABSOLUTE_VALUE(VALUE) (std::abs(VALUE))
/** @brief Provides positive infinity for the specified numeric type. */
#define XHAL_POSITIVE_INFINITY(TYPE) (std::numeric_limits<TYPE>::infinity())
/** @brief Selects the greater of two comparable values. */
#define XHAL_MAXIMUM_VALUE(FIRST_VALUE, SECOND_VALUE) (std::max((FIRST_VALUE), (SECOND_VALUE)))

#endif /* XHAL_RPI5CAR_MATH_H */
