/******************************************************************************
 * @file        xAgent_Rpi5CarMoveExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarMoveExampleTest.cpp.
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

#ifndef XAGENT_RPI5CARMOVEEXAMPLETESTTYPES_H
#define XAGENT_RPI5CARMOVEEXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarMoveExample.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarMotor.h"
#include "xHal_Rpi5CarMotors.h"
#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarServo.h"
#include "xHal_Rpi5CarUltrasonic.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xagent_rpi5carmoveexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carmoveexampletest
{

    using namespace xwalk::hal;
    using namespace xwalk::agent;

    struct TestBus
    {
            agent::bytevector sample{0x03U, 0xE8U};
    };

    struct TestGpio
    {
            agent::boolean value{};
    };

    struct TestSchedule
    {
            agent::uint32vector delays{};
            agent::uint32 queryCount{};
            agent::uint32 queryLimit{10'000U};
    };

} /* namespace xwalk::source_types::xagent_rpi5carmoveexampletest */

#endif /* XAGENT_RPI5CARMOVEEXAMPLETESTTYPES_H */
