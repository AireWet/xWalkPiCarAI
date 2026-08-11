/******************************************************************************
 * @file        xHal_Rpi5CarGpioTest.h
 * @brief       Declares the fixture for individual GPIO operation tests.
 *
 * @details
 * Owns one isolated Linux backend and host mirror for every Google Test case.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Test
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_GPIO_TEST_H
#define XHAL_RPI5CAR_GPIO_TEST_H

#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarGpioHostStub.h"
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarGpioSimulationArguments.h"

#include <gtest/gtest.h>

/** @brief Owns one isolated Linux-backend mirror for each GPIO operation test. */
class TEST_SUITE_XWALK_GPIO: public ::testing::Test
{
    protected:
        /** @brief Constructs the Linux backend and public GPIO test object. */
        TEST_SUITE_XWALK_GPIO();

        /** @brief Destroys the isolated test fixture. */
        ~TEST_SUITE_XWALK_GPIO() override;

        /** @brief Configures tracing before the GPIO operation suite starts. */
        static void SetUpTestSuite();

        /** @brief Records completion after the GPIO operation suite finishes. */
        static void TearDownTestSuite();

        /** @brief Device-free mirror injected at the hardware boundary. */
        xwalk::hal::sim::XWalkGpioHostStub hostStub;

        /** @brief Production Linux backend executed against the host mirror. */
        xwalk::hal::XWalkGpioLinux linuxBackend;

        /** @brief Public GPIO API exercised by each operation test. */
        xwalk::hal::XWalkGpio gpio;
};

#endif /* XHAL_RPI5CAR_GPIO_TEST_H */
