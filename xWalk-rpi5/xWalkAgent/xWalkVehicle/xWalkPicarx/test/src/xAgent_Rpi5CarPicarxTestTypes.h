/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarPicarxTest.cpp.
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

#ifndef XAGENT_RPI5CARPICARXTESTTYPES_H
#define XAGENT_RPI5CARPICARXTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarPicarxSafetyGuard.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xagent_rpi5carpicarxtest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carpicarxtest
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

    struct InvalidConfiguration
    {
            agent::cstring key;
            agent::cstring invalidValue;
            agent::cstring validValue;
    };

} /* namespace xwalk::source_types::xagent_rpi5carpicarxtest */

#endif /* XAGENT_RPI5CARPICARXTESTTYPES_H */
