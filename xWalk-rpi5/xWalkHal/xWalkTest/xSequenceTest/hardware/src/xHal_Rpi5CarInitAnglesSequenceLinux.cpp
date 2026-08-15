/******************************************************************************
 * @file        xHal_Rpi5CarInitAnglesSequenceLinux.cpp
 * @brief       Implements Linux composition for the init-angles sequence.
 *
 * @details
 * Composes physical MCU-reset GPIO, Robot HAT I2C, PWM channels 10 through 12,
 * three servos, and a zero-offset Robot configuration.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarInitAnglesSequenceLinux.h"

#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarI2cLinux.h"
#include "xHal_Rpi5CarInitAnglesSequence.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

    void unusedSpeakerPrime(XWalkHal::contextpointer context, XWalkHal::uint32 durationMilliseconds)
    {
        static_cast<void>(context);
        static_cast<void>(durationMilliseconds);
    }

} /* namespace */

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

    void XWalkInitAnglesSequenceLinux::run(cstring i2cDevice,
                                           cstring gpioDevice,
                                           cstring gpioChipName,
                                           cstring gpioChipLabel,
                                           stringview configurationPath)
    {
        XWalkI2cLinux i2cBackend(i2cDevice);
        XWalkI2c i2c(&i2cBackend,
                     XHAL_I2C_PROBE_CALLBACK(XWalkI2cLinux),
                     XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkI2cLinux),
                     XHAL_I2C_READ_CALLBACK(XWalkI2cLinux));

        const XWalkGpioCallbacks gpioCallbacks = XHAL_GPIO_CALLBACKS(XWalkGpioLinux);
        XWalkGpioLinux resetBackend(gpioDevice, gpioChipName, gpioChipLabel, 28U);
        XWalkGpio resetGpio(&resetBackend, gpioCallbacks, "MCURST");
        XWalkGpioLinux speakerBackend(gpioDevice, gpioChipName, gpioChipLabel, 28U);
        XWalkGpio speakerGpio(&speakerBackend, gpioCallbacks, XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
        XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc, nullptr, &unusedSpeakerPrime);

        XWalkPwmTimerState timerState;
        XWalkPwm firstPwm(i2c, 10U, {}, timerState);
        XWalkPwm secondPwm(i2c, 11U, {}, timerState);
        XWalkPwm thirdPwm(i2c, 12U, {}, timerState);
        XWalkServo firstServo(firstPwm);
        XWalkServo secondServo(secondPwm);
        XWalkServo thirdServo(thirdPwm);
        XWalkConfigStore store(configurationPath);
        store.set("init_angles_servo_offset_list", "[0,0,0]");
        XWalkRobot robot(store, "init_angles");

        XWalkInitAnglesSequence sequence(boardControl, robot, firstServo, secondServo, thirdServo);
        sequence.run();
    }

} /* namespace xwalk::hal::test */
