/******************************************************************************
 * @file        xHal_Rpi5CarI2cTest.h
 * @brief       Declares the fixture for individual I2C operation tests.
 *
 * @details
 * Owns one isolated Linux backend and host mirror for every Google Test case.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Test
 *
 * @author      Joxy John
 * @date        2026-08-09
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_I2C_TEST_H
#define XHAL_RPI5CAR_I2C_TEST_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarI2cHostStub.h"
#include "xHal_Rpi5CarI2cLinux.h"
#include "xHal_Rpi5CarI2cSimulationArguments.h"

#include <gtest/gtest.h>

/******************************************************************************
 * Test fixture declarations
 ******************************************************************************/

/**
 * @class TEST_SUITE_XWALK_I2C
 * @brief Owns one isolated Linux-backend mirror for each I2C operation test.
 */
class TEST_SUITE_XWALK_I2C: public ::testing::Test
{
    protected:
        /**************************************************************************
         * Protected constructors and destructor
         **************************************************************************/

        /** @brief Constructs the Linux backend and public I2C test object. */
        TEST_SUITE_XWALK_I2C();

        /** @brief Destroys the isolated test fixture. */
        ~TEST_SUITE_XWALK_I2C() override;

        /**************************************************************************
         * Protected static member functions
         **************************************************************************/

        /** @brief Configures tracing before the I2C operation suite starts. */
        static void SetUpTestSuite();

        /** @brief Records completion after the I2C operation suite finishes. */
        static void TearDownTestSuite();

        /**************************************************************************
         * Protected data members
         **************************************************************************/

        /** @brief Device-free mirror injected at the hardware boundary. */
        xwalk::hal::sim::XWalkI2cHostStub hostStub;

        /** @brief Production Linux backend executed against the host mirror. */
        xwalk::hal::XWalkI2cLinux linuxBackend;

        /** @brief Public I2C API exercised by each operation test. */
        xwalk::hal::XWalkI2c i2c;
};

#endif /* XHAL_RPI5CAR_I2C_TEST_H */
