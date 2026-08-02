/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerLifecycle.cpp
 * @brief       Implements line-tracker construction and numeric helpers.
 *
 * @details
 * Binds caller-owned ADC channels, validates calibration coefficients, and
 * provides bounded conversion helpers used by line-tracker calculations.
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
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Returns a validated ADC dependency by zero-based channel.
 *
 * @param[in] channel
 * Channel index in the inclusive range zero through two.
 *
 * @return
 * Caller-owned ADC object for the selected channel.
 *
 * @throws outofrange
 * If `channel` exceeds two.
 */
XWalkAdc& XWalkLineTracker::sensorAt(uint32 channel) const
{
    if (channel >= XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT)
    {
        XHAL_THROW_OUT_OF_RANGE("Line-tracker channel must be in the range zero through two");
    }
    return *sensors[channel];
}

/**
 * @brief Validates that all calibration coefficients are finite.
 *
 * @param[in] calibration
 * Coefficients to validate.
 *
 * @throws invalidargument
 * If any slope or offset is non-finite.
 */
void XWalkLineTracker::validateCalibration(const XWalkLineCalibration& calibration)
{
    for (uint32 channel = 0U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
    {
        if (!XHAL_IS_FINITE(calibration.slopes[channel]) ||
            !XHAL_IS_FINITE(calibration.offsets[channel]))
        {
            XHAL_THROW_INVALID_ARGUMENT("Line-tracker calibration values must be finite");
        }
    }
}

/**
 * @brief Rounds a finite calibrated value to the project signed count type.
 *
 * @param[in] value
 * Calibrated floating-point ADC count.
 *
 * @return
 * Nearest representable signed count.
 *
 * @throws invalidargument
 * If `value` is non-finite.
 *
 * @throws outofrange
 * If the rounded value exceeds the signed 32-bit range.
 */
int32 XWalkLineTracker::roundReading(float64 value)
{
    if (!XHAL_IS_FINITE(value))
    {
        XHAL_THROW_INVALID_ARGUMENT("Calibrated line-tracker value must be finite");
    }
    const float64 roundedValue = XHAL_ROUND_NEAREST(value);
    const float64 minimumValue = static_cast<float64>(common::INT32_MINIMUM);
    const float64 maximumValue = static_cast<float64>(common::INT32_MAXIMUM);
    if ((roundedValue < minimumValue) || (roundedValue > maximumValue))
    {
        XHAL_THROW_OUT_OF_RANGE("Calibrated line-tracker value exceeds the signed count range");
    }
    return static_cast<int32>(roundedValue);
}

/**
 * @brief Restricts a value to an inclusive floating-point range.
 *
 * @param[in] value
 * Value to restrict.
 *
 * @param[in] minimum
 * Inclusive lower bound.
 *
 * @param[in] maximum
 * Inclusive upper bound.
 *
 * @return
 * Restricted value.
 */
float64 XWalkLineTracker::constrain(float64 value, float64 minimum, float64 maximum) noexcept
{
    float64 constrainedValue = value;
    if (constrainedValue < minimum)
    {
        constrainedValue = minimum;
    }
    if (constrainedValue > maximum)
    {
        constrainedValue = maximum;
    }
    return constrainedValue;
}

/**
 * @brief Rounds a finite value to two decimal places.
 *
 * @param[in] value
 * Finite value to round.
 *
 * @return
 * Value rounded using the active floating-point rounding mode.
 */
float64 XWalkLineTracker::roundTwoDecimals(float64 value) noexcept
{
    const float64 roundingScale =
        static_cast<float64>(XHAL_RPI5CAR_LINE_TRACKER_ROUNDING_SCALE);
    const float64 scaledValue = value * roundingScale;
    const float64 roundedValue = XHAL_ROUND_NEAREST(scaledValue);
    return roundedValue / roundingScale;
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a line tracker with identity calibration.
 *
 * @param[in] left
 * Left ADC dependency that must outlive this object.
 *
 * @param[in] middle
 * Middle ADC dependency that must outlive this object.
 *
 * @param[in] right
 * Right ADC dependency that must outlive this object.
 */
XWalkLineTracker::XWalkLineTracker(XWalkAdc& left, XWalkAdc& middle, XWalkAdc& right)
    : XWalkLineTracker(left, middle, right, XWalkLineCalibration{})
{
}

/**
 * @brief Constructs a line tracker with supplied calibration.
 *
 * @param[in] left
 * Left ADC dependency that must outlive this object.
 *
 * @param[in] middle
 * Middle ADC dependency that must outlive this object.
 *
 * @param[in] right
 * Right ADC dependency that must outlive this object.
 *
 * @param[in] calibration
 * Finite per-channel slopes and offsets.
 *
 * @throws invalidargument
 * If any calibration coefficient is non-finite.
 */
XWalkLineTracker::XWalkLineTracker(XWalkAdc& left, XWalkAdc& middle, XWalkAdc& right,
    const XWalkLineCalibration& calibration)
    : sensors{&left, &middle, &right}, calibrationValue(calibration)
{
    validateCalibration(calibrationValue);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the tracker without releasing its non-owning ADC dependencies.
 */
XWalkLineTracker::~XWalkLineTracker() = default;

} /* namespace xwalk::hal */
