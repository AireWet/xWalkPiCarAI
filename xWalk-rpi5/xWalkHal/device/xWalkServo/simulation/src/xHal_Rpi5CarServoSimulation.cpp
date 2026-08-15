/******************************************************************************
 * @file        xHal_Rpi5CarServoSimulation.cpp
 * @brief       Implements the device-free xWalkServo simulation.
 * @project     xWalk Firmware
 * @module      xWalkServo Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarServoSimulation.h"
#include "xHal_Rpi5CarServo.h"
#include "xHal_Rpi5CarServoHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    int32 runServoSimulation()
    {
        XWalkServoHostStub backend;
        XWalkI2c i2c(
            &backend, &XWalkServoHostStub::probe, &XWalkServoHostStub::writeRegister, &XWalkServoHostStub::read);
        XWalkPwmTimerState timerState;
        XWalkPwm pwm(i2c, 0U, {}, timerState);
        XWalkServo servo(pwm);
        static_cast<void>(servo.initialize());
        servo.setAngle(0.0);
        servo.setPulseWidthTime(1000.0);
        const boolean valid = (backend.writeCount() >= 6U) &&
                              (backend.lastRegister() == XHAL_RPI5CAR_PWM_CHANNEL_REG) &&
                              (backend.lastPayloadSize() == 2U) && (pwm.pulseWidth() == 204U);
        XWALK_HAL_TRACE_UID0(RPI .187, "xWalkServo host simulation completed");
        return valid ? 0 : 1;
    }
} /* namespace xwalk::hal::sim */
