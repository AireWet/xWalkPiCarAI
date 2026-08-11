/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiServoZeroing.cpp
 * @brief       Composes the Raspberry Pi twelve-servo zeroing mode.
 * @details     Retains PWM channels zero through eleven for one synchronous callback.
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xAgent_Rpi5CarServoZeroing.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarServo.h"

namespace xwalk::agent
{

/**
 * @brief Runs the ordered twelve-channel servo-zeroing service.
 * @param[in,out] context Nullable caller-owned application context.
 * @param[in] callback Non-null synchronous application callback.
 * @param[in,out] i2c Caller-owned bus that remains valid through the callback.
 * @return Status returned by `callback`.
 */
agent::int32 XWalkBootRpi::runServoZeroing(agent::contextpointer context,
    bootapplicationcallback callback, hal::XWalkI2c& i2c)
{
    hal::XWalkPwmTimerState timerState;
    hal::XWalkPwm pwm0(i2c, 0U, {}, timerState);
    hal::XWalkPwm pwm1(i2c, 1U, {}, timerState);
    hal::XWalkPwm pwm2(i2c, 2U, {}, timerState);
    hal::XWalkPwm pwm3(i2c, 3U, {}, timerState);
    hal::XWalkPwm pwm4(i2c, 4U, {}, timerState);
    hal::XWalkPwm pwm5(i2c, 5U, {}, timerState);
    hal::XWalkPwm pwm6(i2c, 6U, {}, timerState);
    hal::XWalkPwm pwm7(i2c, 7U, {}, timerState);
    hal::XWalkPwm pwm8(i2c, 8U, {}, timerState);
    hal::XWalkPwm pwm9(i2c, 9U, {}, timerState);
    hal::XWalkPwm pwm10(i2c, 10U, {}, timerState);
    hal::XWalkPwm pwm11(i2c, 11U, {}, timerState);
    hal::XWalkServo servo0(pwm0);
    hal::XWalkServo servo1(pwm1);
    hal::XWalkServo servo2(pwm2);
    hal::XWalkServo servo3(pwm3);
    hal::XWalkServo servo4(pwm4);
    hal::XWalkServo servo5(pwm5);
    hal::XWalkServo servo6(pwm6);
    hal::XWalkServo servo7(pwm7);
    hal::XWalkServo servo8(pwm8);
    hal::XWalkServo servo9(pwm9);
    hal::XWalkServo servo10(pwm10);
    hal::XWalkServo servo11(pwm11);
    hal::XWalkServo* servos[]{&servo0, &servo1, &servo2, &servo3,
        &servo4, &servo5, &servo6, &servo7, &servo8, &servo9,
        &servo10, &servo11};
    for (hal::XWalkServo* const servo : servos)
    {
        static_cast<void>(servo->initialize());
    }
    const XWalkServoZeroingCallbacks zeroingCallbacks{
        &setServoZeroingAngle, &delayMilliseconds, &continueComputerVision};
    XWalkServoZeroing servoZeroing(servos, zeroingCallbacks);
    XWalkBootServices services{};
    services.servoZeroing = &servoZeroing;
    return callback(context, services);
}

} /* namespace xwalk::agent */
