/******************************************************************************
 * @file        xHal_Rpi5CarExceptions.h
 * @brief       Defines the exception interface used by xWalk HAL modules.
 *
 * @details
 * Centralizes construction of standard exceptions so module code does not
 * depend directly on standard-library exception types.
 *
 * @project     xWalk Firmware
 * @module      xWalkLibraryCommon
 *
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_EXCEPTIONS_H
#define XHAL_RPI5CAR_EXCEPTIONS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarStandardHeaders.h"

/******************************************************************************
 * Function-like macros
 ******************************************************************************/

/** @brief Throws `std::invalid_argument` with the supplied message. */
#define XHAL_THROW_INVALID_ARGUMENT(MESSAGE) throw std::invalid_argument(std::string(MESSAGE))
/** @brief Throws `std::invalid_argument` using a name and detail suffix. */
#define XHAL_THROW_INVALID_ARGUMENT_DETAIL(NAME, DETAIL) \
    throw std::invalid_argument(std::string(NAME).append(DETAIL))
/** @brief Throws `std::out_of_range` with the supplied message. */
#define XHAL_THROW_OUT_OF_RANGE(MESSAGE) throw std::out_of_range(std::string(MESSAGE))
/** @brief Throws `std::out_of_range` using a name and detail suffix. */
#define XHAL_THROW_OUT_OF_RANGE_DETAIL(NAME, DETAIL) \
    throw std::out_of_range(std::string(NAME).append(DETAIL))
/** @brief Throws `std::logic_error` with the supplied message. */
#define XHAL_THROW_LOGIC_ERROR(MESSAGE) throw std::logic_error(std::string(MESSAGE))
/** @brief Throws `std::runtime_error` with the supplied message. */
#define XHAL_THROW_RUNTIME_ERROR(MESSAGE) throw std::runtime_error(std::string(MESSAGE))
/** @brief Throws `std::runtime_error` using a name and detail suffix. */
#define XHAL_THROW_RUNTIME_ERROR_DETAIL(NAME, DETAIL) \
    throw std::runtime_error(std::string(NAME).append(DETAIL))

#endif /* XHAL_RPI5CAR_EXCEPTIONS_H */
