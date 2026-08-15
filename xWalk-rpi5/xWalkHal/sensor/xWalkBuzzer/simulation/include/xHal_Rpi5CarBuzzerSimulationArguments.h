/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerSimulationArguments.h
 * @brief       Declares persistent xWalkBuzzer trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkBuzzer Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_BUZZER_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_BUZZER_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Validates and applies one optional Buzzer trace selector. */
    class XWalkBuzzerSimulationArguments final
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
            XWalkBuzzerSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
            ~XWalkBuzzerSimulationArguments();
            XWalkBuzzerSimulationArguments(const XWalkBuzzerSimulationArguments&) = delete;
            XWalkBuzzerSimulationArguments& operator=(const XWalkBuzzerSimulationArguments&) = delete;
            XWalkBuzzerSimulationArguments(XWalkBuzzerSimulationArguments&&) = delete;
            XWalkBuzzerSimulationArguments& operator=(XWalkBuzzerSimulationArguments&&) = delete;
            boolean valid() const noexcept;
            boolean helpRequested() const noexcept;
            boolean applyTraceUpdate() const;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_BUZZER_SIMULATION_ARGUMENTS_H */
