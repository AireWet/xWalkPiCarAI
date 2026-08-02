/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestFunctions.h
 * @brief       Declares the PWM host-test scenarios.
 *
 * @details
 * Provides independently selectable assertions for address selection, timer
 * mapping, register encoding, shared state, frequency, and input validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Test
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

#ifndef XHAL_RPI5CAR_PWM_TEST_FUNCTIONS_H
#define XHAL_RPI5CAR_PWM_TEST_FUNCTIONS_H

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Function declarations
 ******************************************************************************/

/** @brief Verifies address probing and timer selection for channel `P19`. */
void testAddressAndTimerSelection();

/** @brief Verifies the timer mapping for all twenty PWM channels. */
void testAllTimerMappings();

/** @brief Verifies high-byte-first encoding of 16-bit register data. */
void testBigEndianRegisterData();

/** @brief Verifies shared timer periods and percentage conversion. */
void testSharedPeriodAndPercentage();

/** @brief Verifies that construction configures a frequency near 50 Hertz. */
void testDefaultFrequency();

/** @brief Verifies rejection of invalid channels and numeric settings. */
void testValidation();

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_PWM_TEST_FUNCTIONS_H */
