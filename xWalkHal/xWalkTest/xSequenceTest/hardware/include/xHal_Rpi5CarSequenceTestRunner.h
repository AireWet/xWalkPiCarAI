/******************************************************************************
 * @file        xHal_Rpi5CarSequenceTestRunner.h
 * @brief       Declares centralized physical-sequence CLI dispatch.
 *
 * @details
 * Owns argument validation, usage output, selector resolution, and delegation
 * to each Linux sequence adapter while leaving `main.cpp` as a thin entry point.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_SEQUENCE_TEST_RUNNER_H
#define XHAL_RPI5CAR_SEQUENCE_TEST_RUNNER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/** @brief Validates and dispatches every xSequenceTest command-line selector. */
class XWalkSequenceTestRunner final
{
protected:

    /** @brief Prints every supported selector and its required arguments. */
    static void printUsage();
    /** @brief Validates and runs the button-event selector. */
    static int32 runButtonEvent(int32 argumentCount, char* argumentValues[]);
    /** @brief Validates and runs the initialization-angle selector. */
    static int32 runInitAngles(int32 argumentCount, char* argumentValues[]);
    /** @brief Validates and runs the Robot HAT v5 motor selector. */
    static int32 runRobotHat5Motor(int32 argumentCount, char* argumentValues[]);
    /** @brief Validates and runs the two-motor selector. */
    static int32 runMotor(int32 argumentCount, char* argumentValues[]);
    /** @brief Validates and runs the Servo HAT selector. */
    static int32 runServoHat(int32 argumentCount, char* argumentValues[]);
    /** @brief Validates and runs the 12-channel servo selector. */
    static int32 runServo(int32 argumentCount, char* argumentValues[]);
    /** @brief Validates and runs the tone selector. */
    static int32 runTone(int32 argumentCount, char* argumentValues[]);
    /** @brief Dispatches one selector after its formal arguments are resolved. */
    static int32 runSelection(int32 argumentCount, char* argumentValues[]);
    /** @brief Loads selector arguments from one validated YAML file. */
    static int32 runConfigured(
        stringview executable, stringview selection, stringview configurationPath);

public:

    /**
     * @brief Runs one explicitly selected physical sequence test.
     *
     * @param[in] argumentCount
     * Executable, optional YAML path, selector, and optional formal arguments.
     *
     * @param[in] argumentValues
     * Process argument values in the documented selector order.
     *
     * @return
     * Zero after the selected sequence completes, or two for invalid input.
     *
     * @warning
     * Sequences may reset the MCU, claim GPIO17, move actuators, or produce audio.
     */
    static int32 run(int32 argumentCount, char* argumentValues[]);
};

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_SEQUENCE_TEST_RUNNER_H */
