/******************************************************************************
 * @file        main.cpp
 * @brief       Provides the xSequenceTest process entry point.
 *
 * @details
 * Delegates process arguments to the centralized physical-sequence runner.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSequenceTestRunner.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs one explicitly selected physical sequence test.
 *
 * @param[in] argumentCount
 * Executable, test selector, and selector-specific arguments.
 *
 * @param[in] argumentValues
 * Process argument values in the order documented above.
 *
 * @return
 * Zero after monitoring, or two when arguments are invalid.
 *
 * @warning
 * Sequences may reset the MCU, claim GPIO17, move actuators, or produce audio.
 */
int main(int argumentCount, char* argumentValues[])
{
    return static_cast<int>(xwalk::hal::test::XWalkSequenceTestRunner::run(
        static_cast<xwalk::hal::int32>(argumentCount), argumentValues)
        );
}
