/******************************************************************************
 * @file        xAgent_Rpi5CarGrayscaleCalibrationTypes.h
 * @brief       Declares grayscale-calibration callbacks and results.
 *
 * @details
 * Defines the injected scheduling boundary and pending reference values used by
 * the port of the PiCar-X grayscale calibration helper.
 *
 * @project     xWalk Firmware
 * @module      xWalkGrayscaleCalibration
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_GRAYSCALE_CALIBRATION_TYPES_H
#define XAGENT_RPI5CAR_GRAYSCALE_CALIBRATION_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLineTrackerTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Suspends calibration for one bounded interval.
 * @param[in,out] context Non-owning application context valid for the call duration.
 * @param[in] durationMs Requested delay in milliseconds.
 */
using grayscalecalibrationdelaycallback = void (*)(
    agent::contextpointer context, agent::uint32 durationMs);

/**
 * @brief Reports whether the active calibration may continue.
 * @param[in,out] context Non-owning application context valid for the call duration.
 * @return `true` to continue or `false` to stop and leave pending values unsaved.
 */
using grayscalecalibrationcontinuecallback = agent::boolean (*)(
    agent::contextpointer context);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Pending line and cliff references produced by calibration. */
struct XWalkGrayscaleCalibrationResult
{
    /** @brief Three pending line-reference ADC values. */
    hal::linetrackervalues lineReference{1'000, 1'000, 1'000};
    /** @brief Three pending cliff-reference ADC values. */
    hal::linetrackervalues cliffReference{500, 500, 500};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_GRAYSCALE_CALIBRATION_TYPES_H */
