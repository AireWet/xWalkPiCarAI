/******************************************************************************
 * @file        xHal_Rpi5CarDeviceGroupTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarDeviceGroupTest.cpp.
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

#ifndef XHAL_RPI5CARDEVICEGROUPTESTTYPES_H
#define XHAL_RPI5CARDEVICEGROUPTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarDeviceGroupTestSupport.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarAdcTestSupport.h"
#include "xHal_Rpi5CarAdxl345.h"
#include "xHal_Rpi5CarAdxl345TestSupport.h"
#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarCameraTestSupport.h"
#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarPwmTestI2c.h"
#include "xHal_Rpi5CarServo.h"
#include "xHal_Rpi5CarUltrasonic.h"
#include "xHal_Rpi5CarUltrasonicTestSupport.h"
#include "xHal_Rpi5CarUserButton.h"
#include "xHal_Rpi5CarUserButtonTestSupport.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5cardevicegrouptest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5cardevicegrouptest
{

    using namespace xwalk::hal;

    /** @brief Associates one servo command with its expected PWM timer count. */
    struct ServoAngleCase
    {
            /** @brief Requested angle in degrees, including clamped boundary inputs. */
            float64 angleDegrees{};
            /** @brief Expected PWM timer count after the servo mapping. */
            uint32 pulseWidth{};
    };

    /** @brief Provides one fresh PWM and Servo chain for each angle case. */
    class ServoAngleGroupTest : public testing::TestWithParam<ServoAngleCase>
    {
    };

} /* namespace xwalk::source_types::xhal_rpi5cardevicegrouptest */

#endif /* XHAL_RPI5CARDEVICEGROUPTESTTYPES_H */
