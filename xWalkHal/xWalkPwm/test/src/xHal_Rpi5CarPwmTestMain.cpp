/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestMain.cpp
 * @brief       Provides selection and execution for PWM host-test scenarios.
 *
 * @details
 * Runs all PWM assertions when invoked without an argument or dispatches one
 * named scenario for individual CTest registration.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarPwmTestFunctions.h"

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

    /**
     * @brief Contains test-runner functions private to this translation unit.
     */
    namespace
    {

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

        /** @brief Runs every registered PWM host-test scenario in sequence. */
        void runAllTests()
        {
            xwalk::hal::test::testAddressAndTimerSelection();
            xwalk::hal::test::testAllTimerMappings();
            xwalk::hal::test::testBigEndianRegisterData();
            xwalk::hal::test::testSharedPeriodAndPercentage();
            xwalk::hal::test::testDefaultFrequency();
            xwalk::hal::test::testValidation();
        }

        /**
         * @brief Dispatches one host-test scenario by its command-line name.
         *
         * @param[in] testName
         * Selector matching `address`, `mapping`, `register`, `percentage`,
         * `frequency`, or `validation`.
         *
         * @return
         * Zero when the selector is recognized and assertions pass; otherwise
         * one for an unknown selector.
         */
        int32 runSelectedTest(stringview testName)
        {
            if (testName == "address")
            {
                testAddressAndTimerSelection();
            }
            else if (testName == "mapping")
            {
                testAllTimerMappings();
            }
            else if (testName == "register")
            {
                testBigEndianRegisterData();
            }
            else if (testName == "percentage")
            {
                testSharedPeriodAndPercentage();
            }
            else if (testName == "frequency")
            {
                testDefaultFrequency();
            }
            else if (testName == "validation")
            {
                testValidation();
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
 * @brief Runs all PWM host tests or one selected scenario.
 *
 * @param[in] argumentCount
 * Number of command-line arguments, including the executable name.
 *
 * @param[in] argumentValues
 * Non-owning array of command-line string pointers valid for this call.
 *
 * @return
 * Zero when the requested assertions pass; otherwise one for invalid command
 * usage or an unknown selector.
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
