/******************************************************************************
 * @file        xHal_Rpi5CarServoTestFunctions.h
 * @brief       Declares the Servo host-test scenarios.
 *
 * @details
 * Provides independently selectable assertions for timer initialization,
 * angle conversion, pulse-duration conversion, and input validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkServo Host Test
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

#ifndef XHAL_RPI5CAR_SERVO_TEST_FUNCTIONS_H
#define XHAL_RPI5CAR_SERVO_TEST_FUNCTIONS_H

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

/** @brief Verifies period and prescaler configuration during construction. */
void testServoInitialization();

/** @brief Verifies angle clamping and angle-to-count conversion. */
void testServoAngles();

/** @brief Verifies direct pulse-duration clamping and conversion. */
void testServoPulseWidths();

/** @brief Verifies rejection of non-finite servo commands. */
void testServoValidation();

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_SERVO_TEST_FUNCTIONS_H */
