/******************************************************************************
 * @file        xHal_Rpi5CarServoSimulationArguments.h
 * @brief       Declares persistent xWalkServo trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkServo Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_SERVO_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_SERVO_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional Servo trace selector. */
class XWalkServoSimulationArguments final
{
    private:
        string traceTargetValue;
        boolean traceEnabledValue;
        boolean traceUpdateRequestedValue;
        boolean validValue;
        boolean helpRequestedValue;
    protected:
        static boolean targetIsValid(stringview target) noexcept;
        void parseSelector(stringview selector);
    public:
        XWalkServoSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkServoSimulationArguments();
        XWalkServoSimulationArguments(const XWalkServoSimulationArguments&) = delete;
        XWalkServoSimulationArguments& operator=(const XWalkServoSimulationArguments&) = delete;
        XWalkServoSimulationArguments(XWalkServoSimulationArguments&&) = delete;
        XWalkServoSimulationArguments& operator=(XWalkServoSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_SERVO_SIMULATION_ARGUMENTS_H */
