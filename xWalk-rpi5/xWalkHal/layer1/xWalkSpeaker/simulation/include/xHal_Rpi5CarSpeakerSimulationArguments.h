/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerSimulationArguments.h
 * @brief       Declares persistent xWalkSpeaker trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkSpeaker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_SPEAKER_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_SPEAKER_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Validates and applies one optional Speaker trace selector. */
    class XWalkSpeakerSimulationArguments final
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
            XWalkSpeakerSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
            ~XWalkSpeakerSimulationArguments();
            XWalkSpeakerSimulationArguments(const XWalkSpeakerSimulationArguments&) = delete;
            XWalkSpeakerSimulationArguments& operator=(const XWalkSpeakerSimulationArguments&) = delete;
            XWalkSpeakerSimulationArguments(XWalkSpeakerSimulationArguments&&) = delete;
            XWalkSpeakerSimulationArguments& operator=(XWalkSpeakerSimulationArguments&&) = delete;
            boolean valid() const noexcept;
            boolean helpRequested() const noexcept;
            boolean applyTraceUpdate() const;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_SPEAKER_SIMULATION_ARGUMENTS_H */
