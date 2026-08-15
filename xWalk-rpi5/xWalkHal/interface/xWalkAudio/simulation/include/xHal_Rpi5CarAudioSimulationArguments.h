/******************************************************************************
 * @file        xHal_Rpi5CarAudioSimulationArguments.h
 * @brief       Declares xWalkAudio simulation trace-option parsing.
 *
 * @details
 * Accepts help and persistent trace selectors without exposing console output
 * or trace-configuration implementation details to the executable entry point.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio Host Simulation
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

#ifndef XHAL_RPI5CAR_AUDIO_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_AUDIO_SIMULATION_ARGUMENTS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkAudio simulation support.
 */
namespace xwalk::hal::sim
{

    /**
     * @class XWalkAudioSimulationArguments
     * @brief Validates and applies one optional Audio simulation trace selector.
     */
    class XWalkAudioSimulationArguments final
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
            /**
             * @brief Parses the standalone Audio simulation command line.
             * @param[in] argumentCount Number of entries in `argumentValues`.
             * @param[in] argumentValues Command-line values valid for construction.
             */
            XWalkAudioSimulationArguments(int32 argumentCount, charpointer argumentValues[]);

            /** @brief Destroys the owned parsed selector state. */
            ~XWalkAudioSimulationArguments();

            XWalkAudioSimulationArguments(const XWalkAudioSimulationArguments&) = delete;
            XWalkAudioSimulationArguments& operator=(const XWalkAudioSimulationArguments&) = delete;
            XWalkAudioSimulationArguments(XWalkAudioSimulationArguments&&) = delete;
            XWalkAudioSimulationArguments& operator=(XWalkAudioSimulationArguments&&) = delete;

            /** @brief Returns whether the command-line shape and selector are valid. */
            boolean valid() const noexcept;

            /** @brief Returns whether help was requested. */
            boolean helpRequested() const noexcept;

            /**
             * @brief Applies and persists the parsed trace update when requested.
             * @return `true` when no update was requested or the update succeeded.
             */
            boolean applyTraceUpdate() const;
    };

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_AUDIO_SIMULATION_ARGUMENTS_H */
