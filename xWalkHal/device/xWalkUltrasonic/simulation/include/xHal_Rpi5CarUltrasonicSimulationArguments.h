/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicSimulationArguments.h
 * @brief       Declares persistent ultrasonic trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ULTRASONIC_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_ULTRASONIC_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional ultrasonic trace selector. */
class XWalkUltrasonicSimulationArguments final
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
        XWalkUltrasonicSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkUltrasonicSimulationArguments();
        XWalkUltrasonicSimulationArguments(const XWalkUltrasonicSimulationArguments&) = delete;
        XWalkUltrasonicSimulationArguments& operator=(const XWalkUltrasonicSimulationArguments&) = delete;
        XWalkUltrasonicSimulationArguments(XWalkUltrasonicSimulationArguments&&) = delete;
        XWalkUltrasonicSimulationArguments& operator=(XWalkUltrasonicSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ULTRASONIC_SIMULATION_ARGUMENTS_H */
