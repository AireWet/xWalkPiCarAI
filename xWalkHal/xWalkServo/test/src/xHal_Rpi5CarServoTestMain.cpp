/******************************************************************************
 * @file        xHal_Rpi5CarServoTestMain.cpp
 * @brief       Provides selection and execution for Servo host tests.
 *
 * @details
 * Runs every Servo scenario or dispatches one named scenario for CTest.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServoTestFunctions.h"

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

    /** @brief Contains test-runner functions private to this translation unit. */
    namespace
    {

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

        /** @brief Runs every registered Servo host-test scenario in sequence. */
        void runAllTests()
        {
            testServoInitialization();
            testServoAngles();
            testServoPulseWidths();
            testServoValidation();
        }

        /**
         * @brief Dispatches one Servo host-test scenario by name.
         *
         * @param[in] testName
         * Selector matching `initialization`, `angle`, `pulse`, or `validation`.
         *
         * @return
         * Zero for a recognized passing scenario; otherwise one.
         */
        int32 runSelectedTest(stringview testName)
        {
            if (testName == "initialization")
            {
                testServoInitialization();
            }
            else if (testName == "angle")
            {
                testServoAngles();
            }
            else if (testName == "pulse")
            {
                testServoPulseWidths();
            }
            else if (testName == "validation")
            {
                testServoValidation();
            }
            else
            {
                return 1;
            }
            return 0;
        }

    } /* namespace */
} /* namespace xwalk::hal::test */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all Servo host tests or one selected scenario.
 *
 * @param[in] argumentCount
 * Number of command-line arguments, including the executable name.
 *
 * @param[in] argumentValues
 * Non-owning array of command-line string pointers valid for this call.
 *
 * @return
 * Zero when requested assertions pass; otherwise one for invalid usage.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer argumentValues[])
{
    if (argumentCount == 1)
    {
        xwalk::hal::test::runAllTests();
        return 0;
    }
    if (argumentCount != 2)
    {
        return 1;
    }
    return xwalk::hal::test::runSelectedTest(argumentValues[1]);
}
