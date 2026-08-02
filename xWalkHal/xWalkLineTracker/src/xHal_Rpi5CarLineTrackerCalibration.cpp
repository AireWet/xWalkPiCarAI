/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerCalibration.cpp
 * @brief       Implements line-tracker calibration and adaptive references.
 *
 * @details
 * Stores validated linear coefficients, derives calibration from light and
 * dark samples, and updates background and dark-line reference values.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLineTracker.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Replaces the active linear calibration coefficients.
 *
 * @param[in] calibration
 * Finite per-channel slopes and offsets.
 *
 * @throws invalidargument
 * If any coefficient is non-finite.
 *
 * @post
 * `calibrationData()` equals `calibration`.
 */
void XWalkLineTracker::setCalibrationData(const XWalkLineCalibration& calibration)
{
    validateCalibration(calibration);
    calibrationValue = calibration;
}

/**
 * @brief Returns the active linear calibration coefficients.
 *
 * @return
 * Read-only calibration reference valid until the next calibration update.
 */
const XWalkLineCalibration& XWalkLineTracker::calibrationData() const noexcept
{
    return calibrationValue;
}

/**
 * @brief Derives linear calibration from light and dark samples.
 *
 * @param[in] lightData
 * Left, middle, and right samples measured over the light background.
 *
 * @param[in] darkData
 * Left, middle, and right samples measured over the dark line.
 *
 * @return
 * Applied slopes and offsets, each rounded to two decimal places.
 *
 * @throws invalidargument
 * If a non-reference channel does not have increasing dark-to-light input and output ranges.
 *
 * @post
 * `calibrationData()` equals the returned coefficients.
 */
XWalkLineCalibration XWalkLineTracker::calibrate(const linetrackervalues& lightData,
    const linetrackervalues& darkData)
{
    uint32 maximumIndex = 0U;
    for (uint32 channel = 1U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
    {
        if (lightData[channel] > lightData[maximumIndex])
        {
            maximumIndex = channel;
        }
    }

    XWalkLineCalibration calibration{};
    for (uint32 channel = 0U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
    {
        if (channel == maximumIndex)
        {
            calibration.slopes[channel] = 1.0;
            calibration.offsets[channel] = 0.0;
            continue;
        }

        const float64 inputDark = static_cast<float64>(darkData[channel]);
        const float64 outputDark = static_cast<float64>(darkData[maximumIndex]);
        const float64 inputLight = static_cast<float64>(lightData[channel]);
        const float64 outputLight = static_cast<float64>(lightData[maximumIndex]);
        const float64 inputRange = inputLight - inputDark;
        const float64 outputRange = outputLight - outputDark;
        if ((inputRange <= 0.0) || (outputRange <= 0.0))
        {
            XHAL_THROW_INVALID_ARGUMENT(
                "Line-tracker dark values must be lower than light values");
        }

        const float64 slope = outputRange / inputRange;
        const float64 scaledDarkInput = slope * inputDark;
        const float64 offset = outputDark - scaledDarkInput;
        calibration.slopes[channel] = roundTwoDecimals(slope);
        calibration.offsets[channel] = roundTwoDecimals(offset);
    }

    setCalibrationData(calibration);
    return calibrationValue;
}

/**
 * @brief Adapts the background reference toward the largest current value.
 *
 * @param[in] currentValues
 * Left, middle, and right calibrated ADC counts.
 *
 * @post
 * Five percent of the current maximum is blended into the background reference.
 */
void XWalkLineTracker::updateLineBackgroundReference(
    const linetrackervalues& currentValues) noexcept
{
    int32 maximumValue = currentValues[0U];
    for (uint32 channel = 1U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
    {
        if (currentValues[channel] > maximumValue)
        {
            maximumValue = currentValues[channel];
        }
    }

    const float64 updateRate = XHAL_RPI5CAR_LINE_TRACKER_REFERENCE_UPDATE_RATE;
    const float64 completeRate = 1.0;
    const float64 keepRate = completeRate - updateRate;
    const float64 retainedReference = keepRate * lineBackgroundReferenceValue;
    const float64 maximumFloatValue = static_cast<float64>(maximumValue);
    const float64 updatedReference = updateRate * maximumFloatValue;
    lineBackgroundReferenceValue = retainedReference + updatedReference;
}

/**
 * @brief Adapts the dark-line reference toward the smallest current value.
 *
 * @param[in] currentValues
 * Left, middle, and right calibrated ADC counts.
 *
 * @post
 * Five percent of the current minimum is blended into the dark-line reference.
 */
void XWalkLineTracker::updateLineReference(const linetrackervalues& currentValues) noexcept
{
    int32 minimumValue = currentValues[0U];
    for (uint32 channel = 1U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
    {
        if (currentValues[channel] < minimumValue)
        {
            minimumValue = currentValues[channel];
        }
    }

    const float64 updateRate = XHAL_RPI5CAR_LINE_TRACKER_REFERENCE_UPDATE_RATE;
    const float64 completeRate = 1.0;
    const float64 keepRate = completeRate - updateRate;
    const float64 retainedReference = keepRate * lineReferenceValue;
    const float64 minimumFloatValue = static_cast<float64>(minimumValue);
    const float64 updatedReference = updateRate * minimumFloatValue;
    lineReferenceValue = retainedReference + updatedReference;
}

} /* namespace xwalk::hal */
