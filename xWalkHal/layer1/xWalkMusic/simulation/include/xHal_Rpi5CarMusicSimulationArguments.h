/******************************************************************************
 * @file        xHal_Rpi5CarMusicSimulationArguments.h
 * @brief       Declares persistent xWalkMusic trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkMusic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_MUSIC_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_MUSIC_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional Music trace selector. */
class XWalkMusicSimulationArguments final
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
        XWalkMusicSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkMusicSimulationArguments();
        XWalkMusicSimulationArguments(const XWalkMusicSimulationArguments&) = delete;
        XWalkMusicSimulationArguments& operator=(const XWalkMusicSimulationArguments&) = delete;
        XWalkMusicSimulationArguments(XWalkMusicSimulationArguments&&) = delete;
        XWalkMusicSimulationArguments& operator=(XWalkMusicSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_MUSIC_SIMULATION_ARGUMENTS_H */
