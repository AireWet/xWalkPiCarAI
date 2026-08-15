/******************************************************************************
 * @file        main.cpp
 * @brief       Provides the single xExample process entry point.
 *
 * @details
 * Delegates process arguments to the centralized ported-example runner.
 *
 * @project     xWalk Firmware
 * @module      xExample
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

#include "xHal_Rpi5CarExampleRunner.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Selects one explicitly requested ported example.
 *
 * @param[in] argumentCount
 * Executable name followed by a future example selector and its arguments.
 *
 * @param[in] argumentValues
 * Process argument values reserved for the central selector.
 *
 * @return
 * Zero after the selected example completes, or two for invalid arguments.
 *
 * @warning Ported examples may access physical Raspberry Pi hardware.
 */
int main(int argumentCount, char* argumentValues[])
{
    return static_cast<int>(
        xwalk::hal::example::XWalkExampleRunner::run(static_cast<xwalk::hal::int32>(argumentCount), argumentValues));
}
