/******************************************************************************
 * @file        xHal_Rpi5CarSpiTest.h
 * @brief       Declares the fixture for individual SPI operation tests.
 *
 * @details
 * Owns one isolated Linux backend and host mirror for every Google Test case.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Test
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

#ifndef XHAL_RPI5CAR_SPI_TEST_H
#define XHAL_RPI5CAR_SPI_TEST_H

#include "xHal_Rpi5CarSpi.h"
#include "xHal_Rpi5CarSpiHostStub.h"
#include "xHal_Rpi5CarSpiLinux.h"
#include "xHal_Rpi5CarSpiSimulationArguments.h"

#include <gtest/gtest.h>

/** @brief Owns one isolated Linux-backend mirror for each SPI operation test. */
class TEST_SUITE_XWALK_SPI: public ::testing::Test
{
    protected:
        /** @brief Constructs the Linux backend and public SPI test object. */
        TEST_SUITE_XWALK_SPI();

        /** @brief Destroys the isolated test fixture. */
        ~TEST_SUITE_XWALK_SPI() override;

        /** @brief Configures tracing before the SPI operation suite starts. */
        static void SetUpTestSuite();

        /** @brief Records completion after the SPI operation suite finishes. */
        static void TearDownTestSuite();

        /** @brief Device-free mirror injected at the hardware boundary. */
        xwalk::hal::sim::XWalkSpiHostStub hostStub;

        /** @brief Production Linux backend executed against the host mirror. */
        xwalk::hal::XWalkSpiLinux linuxBackend;

        /** @brief Public SPI API exercised by each operation test. */
        xwalk::hal::XWalkSpi spi;
};

#endif /* XHAL_RPI5CAR_SPI_TEST_H */
