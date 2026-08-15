/******************************************************************************
 * @file        xAgent_Rpi5CarCliffDetectionTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarCliffDetectionTest.cpp.
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

#ifndef XAGENT_RPI5CARCLIFFDETECTIONTESTTYPES_H
#define XAGENT_RPI5CARCLIFFDETECTIONTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarCliffDetection.h"
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
 * @namespace xwalk::source_types::xagent_rpi5carcliffdetectiontest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carcliffdetectiontest
{

    using namespace xwalk::hal;
    using namespace xwalk::agent;

    /** @brief Provides a caller-selected ADC sample. */
    struct TestBus
    {
            agent::bytevector sample{0x03U, 0xE8U};
    };

    /** @brief Stores one simulated GPIO level. */
    struct TestGpio
    {
            agent::boolean value{};
    };

    /** @brief Records injected delays and cancellation queries. */
    struct TestSchedule
    {
            agent::uint32vector delays{};
            agent::uint32 queryCount{};
            agent::uint32 queryLimit{10'000U};
    };

} /* namespace xwalk::source_types::xagent_rpi5carcliffdetectiontest */

#endif /* XAGENT_RPI5CARCLIFFDETECTIONTESTTYPES_H */
