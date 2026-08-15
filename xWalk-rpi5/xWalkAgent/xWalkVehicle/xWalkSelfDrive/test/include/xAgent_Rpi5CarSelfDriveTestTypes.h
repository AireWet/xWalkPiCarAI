/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarSelfDriveTest.cpp.
 *
 * @project     xWalk Firmware
 * @module      Source Type Support
 *
 * @author      Joxy John
 * @date        2026-08-15
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CARSELFDRIVETESTTYPES_H
#define XAGENT_RPI5CARSELFDRIVETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarSelfDrive.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xagent_rpi5carselfdrivetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carselfdrivetest
{

    using namespace xwalk::hal;
    using namespace xwalk::agent;

    /** @brief Provides deterministic I2C samples and accepts register writes. */
    struct TestBus
    {
            agent::bytevector sample{0x03U, 0xE8U};
    };

    /** @brief Stores one simulated GPIO level. */
    struct TestGpio
    {
            agent::boolean value{};
    };

    /** @brief Records injected delay and audio operations. */
    struct TestBackend
    {
            agent::uint32vector delays{};
            agent::stringvector backgroundFiles{};
            agent::float64vector backgroundVolumes{};
            agent::boolean outputEnabled{};
            agent::uint32 continueQueries{};
            agent::uint32 continueQueryLimit{1'000'000U};
            xwalk::hal::XWalkMotors* motors{nullptr};
            agent::boolean failDelayWhileMoving{};
    };

} /* namespace xwalk::source_types::xagent_rpi5carselfdrivetest */

#endif /* XAGENT_RPI5CARSELFDRIVETESTTYPES_H */
