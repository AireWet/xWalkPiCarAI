/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelSimulationArguments.h
 * @brief       Declares language-model simulation trace argument parsing.
 * @details     Validates one optional persistent trace selector.
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LANGUAGE_MODEL_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_LANGUAGE_MODEL_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    class XWalkLanguageModelSimulationArguments final
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
            XWalkLanguageModelSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
            ~XWalkLanguageModelSimulationArguments();
            XWalkLanguageModelSimulationArguments(const XWalkLanguageModelSimulationArguments&) = delete;
            XWalkLanguageModelSimulationArguments& operator=(const XWalkLanguageModelSimulationArguments&) = delete;
            XWalkLanguageModelSimulationArguments(XWalkLanguageModelSimulationArguments&&) = delete;
            XWalkLanguageModelSimulationArguments& operator=(XWalkLanguageModelSimulationArguments&&) = delete;
            boolean valid() const noexcept;
            boolean helpRequested() const noexcept;
            boolean applyTraceUpdate() const;
    };
} /* namespace xwalk::hal::sim */
#endif
