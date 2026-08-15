/******************************************************************************
 * @file        xHal_Rpi5CarSensorGroupTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarSensorGroupTest.cpp.
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

#ifndef XHAL_RPI5CARSENSORGROUPTESTTYPES_H
#define XHAL_RPI5CARSENSORGROUPTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarBuzzer.h"
#include "xHal_Rpi5CarBuzzerTestSupport.h"
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarLedTestSupport.h"
#include "xHal_Rpi5CarLineTracker.h"
#include "xHal_Rpi5CarLineTrackerTestSupport.h"
#include "xHal_Rpi5CarMotor.h"
#include "xHal_Rpi5CarMotors.h"
#include "xHal_Rpi5CarMotorTestSupport.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carsensorgrouptest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carsensorgrouptest
{

    using namespace xwalk::hal;

    /** @brief Describes one line observation and its expected actuator response. */
    struct LineResponseCase
    {
            /** @brief Left, middle, and right raw ADC samples. */
            fixedarray<uint16, 3U> samples{};
            /** @brief Expected left-motor speed in percent. */
            float64 leftSpeedPercent{};
            /** @brief Expected right-motor speed in percent. */
            float64 rightSpeedPercent{};
            /** @brief `true` when the policy must enter its critical safe state. */
            boolean critical{};
    };

    /** @brief Provides one independent hardware-free composition for each line case. */
    class LineResponseGroupTest : public testing::TestWithParam<LineResponseCase>
    {
    };

} /* namespace xwalk::source_types::xhal_rpi5carsensorgrouptest */

#endif /* XHAL_RPI5CARSENSORGROUPTESTTYPES_H */
