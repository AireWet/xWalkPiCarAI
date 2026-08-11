/******************************************************************************
 * @file        xHal_Rpi5CarGpioSimulationArguments.h
 * @brief       Declares xWalkGpio simulation trace-option parsing.
 *
 * @details
 * Validates one optional trace selector before the simulation opens its
 * build-selected GPIO device.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_GPIO_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_GPIO_SIMULATION_ARGUMENTS_H

#include "xHal_Rpi5CarCommon.h"

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkGpio simulation support.
 */
namespace xwalk::hal::sim
{

/** @brief Validates and applies one optional simulation trace selector. */
class XWalkGpioSimulationArguments final
{
    public:
        XWalkGpioSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkGpioSimulationArguments();

        XWalkGpioSimulationArguments(const XWalkGpioSimulationArguments&) = delete;
        XWalkGpioSimulationArguments& operator=(const XWalkGpioSimulationArguments&) = delete;
        XWalkGpioSimulationArguments(XWalkGpioSimulationArguments&&) = delete;
        XWalkGpioSimulationArguments& operator=(XWalkGpioSimulationArguments&&) = delete;

        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;

    protected:
        static boolean targetIsValid(stringview target) noexcept;
        void parseSelector(stringview selector);

    private:
        string traceTargetValue;
        boolean traceEnabledValue;
        boolean traceUpdateRequestedValue;
        boolean validValue;
        boolean helpRequestedValue;
};

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_GPIO_SIMULATION_ARGUMENTS_H */
