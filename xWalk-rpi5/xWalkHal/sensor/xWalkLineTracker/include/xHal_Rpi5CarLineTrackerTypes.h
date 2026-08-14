/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerTypes.h
 * @brief       Declares fixed-size line-tracker types and calibration data.
 *
 * @details
 * Defines the value, calibration, status, non-owning ADC-pointer arrays, and
 * linear calibration structure used by the xWalkLineTracker public interface.
 *
 * @project     xWalk Firmware
 * @module      xWalkLineTracker
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_LINE_TRACKER_TYPES_H
#define XHAL_RPI5CAR_LINE_TRACKER_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Forward declarations
 ******************************************************************************/

/** @brief Declares the ADC dependency referenced by module-owned non-owning pointers. */
class XWalkAdc;

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Fixed left, middle, and right calibrated sensor values in ADC counts. */
using linetrackervalues = fixedarray<int32, XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT>;
/** @brief Fixed left, middle, and right floating-point calibration values. */
using linetrackercalibrationvalues =
    fixedarray<float64, XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT>;
/** @brief Fixed status values where zero is white and one is black. */
using linetrackerstatus = fixedarray<uint8, XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT>;
/** @brief Fixed non-owning ADC pointers ordered left, middle, and right. */
using linetrackeradcpointers = fixedarray<XWalkAdc*, XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT>;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @brief Contains per-channel linear calibration coefficients.
 */
struct XWalkLineCalibration
{
    /** @brief Finite multiplicative coefficient for each sensor channel. */
    linetrackercalibrationvalues slopes{1.0, 1.0, 1.0};
    /** @brief Finite additive coefficient for each sensor channel, in ADC counts. */
    linetrackercalibrationvalues offsets{0.0, 0.0, 0.0};
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_LINE_TRACKER_TYPES_H */
