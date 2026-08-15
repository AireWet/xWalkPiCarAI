/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerReading.cpp
 * @brief       Implements line-tracker sampling and position estimation.
 *
 * @details
 * Reads and calibrates ADC samples, detects cliff and line conditions, computes
 * normalized line position, and exposes adaptive reference state.
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

#include "xHal_Rpi5CarTrace.h"

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
     * @brief Reads one raw or calibrated line-tracker channel.
     *
     * @param[in] channel
     * Channel index in the inclusive range zero through two.
     *
     * @param[in] raw
     * `true` returns the ADC count without calibration; `false` applies
     * calibration.
     *
     * @return
     * Raw or calibrated ADC count.
     *
     * @throws outofrange
     * If `channel` exceeds two or the calibrated result exceeds the signed count
     * range.
     *
     * @throws invalidargument
     * If calibration produces a non-finite result.
     *
     * @throws runtimeerror
     * If the ADC transaction does not return a complete sample.
     */
    int32 XWalkLineTracker::readChannel(uint32 channel, boolean raw)
    {
        const int32 rawValue = static_cast<int32>(sensorAt(channel).read());
        if (raw)
        {
            return rawValue;
        }

        const float64 rawFloatValue = static_cast<float64>(rawValue);
        const float64 scaledValue = rawFloatValue * calibrationValue.slopes[channel];
        const float64 calibratedValue = scaledValue + calibrationValue.offsets[channel];
        return roundReading(calibratedValue);
    }

    /**
     * @brief Applies active linear calibration to three supplied readings.
     *
     * @param[in] data
     * Left, middle, and right raw ADC counts.
     *
     * @return
     * Rounded calibrated counts.
     *
     * @throws outofrange
     * If a calibrated result exceeds the signed count range.
     *
     * @throws invalidargument
     * If calibration produces a non-finite result.
     */
    linetrackervalues XWalkLineTracker::calibrateData(const linetrackervalues& data) const
    {
        linetrackervalues calibratedData{};
        for (uint32 channel = 0U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
        {
            const float64 rawValue = static_cast<float64>(data[channel]);
            const float64 scaledValue = rawValue * calibrationValue.slopes[channel];
            const float64 calibratedValue = scaledValue + calibrationValue.offsets[channel];
            calibratedData[channel] = roundReading(calibratedValue);
        }
        return calibratedData;
    }

    /**
     * @brief Reads all three raw or calibrated line-tracker channels.
     *
     * @param[in] raw
     * `true` returns raw ADC counts; `false` applies active calibration.
     *
     * @return
     * Left, middle, and right raw or calibrated counts.
     */
    linetrackervalues XWalkLineTracker::read(boolean raw)
    {
        linetrackervalues data{};
        for (uint32 channel = 0U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
        {
            data[channel] = readChannel(channel, raw);
        }
        XWALK_HAL_TRACE_UID0(RPI .235, "Line-tracker three-channel read completed");
        return data;
    }

    /**
     * @brief Reads all channels and checks whether any indicates a cliff.
     *
     * @return
     * `true` when any calibrated count is below the cliff threshold; otherwise
     * `false`.
     */
    boolean XWalkLineTracker::isOnCliff()
    {
        return isOnCliff(read());
    }

    /**
     * @brief Checks supplied readings for a cliff condition.
     *
     * @param[in] data
     * Left, middle, and right calibrated counts.
     *
     * @return
     * `true` when any count is below the cliff threshold; otherwise `false`.
     */
    boolean XWalkLineTracker::isOnCliff(const linetrackervalues& data) const noexcept
    {
        for (uint32 channel = 0U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
        {
            if (data[channel] < cliffThresholdValue)
            {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Reads all channels and estimates normalized line position.
     *
     * @return
     * Position from -1.0 at the left through 0.0 at center to 1.0 at the right.
     *
     * @throws runtimeerror
     * If the adaptive background and line references are equal.
     */
    float64 XWalkLineTracker::getLinePosition()
    {
        return getLinePosition(read());
    }

    /**
     * @brief Estimates normalized line position from supplied calibrated readings.
     *
     * @param[in] data
     * Left, middle, and right calibrated counts.
     *
     * @return
     * Position from -1.0 at the left through 0.0 at center to 1.0 at the right,
     * rounded to two decimal places. Zero is returned when no line is detected.
     *
     * @throws runtimeerror
     * If the adaptive background and line references are equal.
     *
     * @post
     * Adaptive references are updated when the detected pattern is not edge-only.
     */
    float64 XWalkLineTracker::getLinePosition(const linetrackervalues& data)
    {
        const float64 referenceDifference = lineBackgroundReferenceValue - lineReferenceValue;
        if (referenceDifference == 0.0)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Line-tracker references must not be equal");
        }

        linetrackercalibrationvalues weights{};
        for (uint32 channel = 0U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
        {
            const float64 channelValue = static_cast<float64>(data[channel]);
            const float64 referenceDelta = lineBackgroundReferenceValue - channelValue;
            const float64 rawWeight = referenceDelta / referenceDifference;
            weights[channel] = constrain(rawWeight, 0.0, 1.0);
        }

        const float64 leftWeight = weights[XHAL_RPI5CAR_LINE_TRACKER_LEFT_CHANNEL];
        const float64 middleWeight = weights[XHAL_RPI5CAR_LINE_TRACKER_MIDDLE_CHANNEL];
        const float64 rightWeight = weights[XHAL_RPI5CAR_LINE_TRACKER_RIGHT_CHANNEL];
        const float64 leftAndMiddleWeight = leftWeight + middleWeight;
        const float64 weightSum = leftAndMiddleWeight + rightWeight;
        if (weightSum < XHAL_RPI5CAR_LINE_TRACKER_MINIMUM_WEIGHT)
        {
            return 0.0;
        }
        const hal::boolean onLineNotMatched = static_cast<hal::boolean>(!isOnLine(data));
        if (onLineNotMatched)
        {
            return 0.0;
        }

        float64 position = 0.0;
        const float64 minimumWeight = XHAL_RPI5CAR_LINE_TRACKER_MINIMUM_WEIGHT;
        const boolean leftOnly =
            (leftWeight >= minimumWeight) && (middleWeight < minimumWeight) && (rightWeight < minimumWeight);
        const boolean rightOnly =
            (rightWeight >= minimumWeight) && (middleWeight < minimumWeight) && (leftWeight < minimumWeight);
        if (leftOnly)
        {
            position = leftWeight - XHAL_RPI5CAR_LINE_TRACKER_EDGE_WEIGHT_OFFSET;
        }
        else if (rightOnly)
        {
            position = XHAL_RPI5CAR_LINE_TRACKER_EDGE_WEIGHT_OFFSET - rightWeight;
        }
        else
        {
            const float64 sideWeightDifference = rightWeight - leftWeight;
            position = sideWeightDifference / weightSum;
            updateLineBackgroundReference(data);
            updateLineReference(data);
        }

        const float64 normalizedPosition = position / XHAL_RPI5CAR_LINE_TRACKER_POSITION_DIVISOR;
        const float64 constrainedPosition = constrain(
            normalizedPosition, XHAL_RPI5CAR_LINE_TRACKER_MINIMUM_POSITION, XHAL_RPI5CAR_LINE_TRACKER_MAXIMUM_POSITION);
        const float64 result = roundTwoDecimals(constrainedPosition);
        XWALK_HAL_TRACE_UID1(RPI .238, "Line position calculated as %.2f", result);
        return result;
    }

    /**
     * @brief Reads all channels and checks whether a line is detected.
     *
     * @return
     * `true` when no cliff exists and channel spread exceeds 200 counts.
     */
    boolean XWalkLineTracker::isOnLine()
    {
        return isOnLine(read());
    }

    /**
     * @brief Checks supplied readings for a detected line.
     *
     * @param[in] data
     * Left, middle, and right calibrated counts.
     *
     * @return
     * `true` when no cliff exists and channel spread exceeds 200 counts.
     */
    boolean XWalkLineTracker::isOnLine(const linetrackervalues& data) const noexcept
    {
        const hal::boolean onCliffMatched = static_cast<hal::boolean>(isOnCliff(data));
        if (onCliffMatched)
        {
            return false;
        }

        int32 minimumValue = data[0U];
        int32 maximumValue = data[0U];
        for (uint32 channel = 1U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
        {
            if (data[channel] < minimumValue)
            {
                minimumValue = data[channel];
            }
            if (data[channel] > maximumValue)
            {
                maximumValue = data[channel];
            }
        }
        const float64 minimumFloatValue = static_cast<float64>(minimumValue);
        const float64 maximumFloatValue = static_cast<float64>(maximumValue);
        const float64 valueDifference = maximumFloatValue - minimumFloatValue;
        const float64 requiredDifference = static_cast<float64>(XHAL_RPI5CAR_LINE_TRACKER_LINE_DIFFERENCE);
        return valueDifference > requiredDifference;
    }

    /**
     * @brief Returns the active cliff threshold.
     *
     * @return
     * Calibrated ADC count below which a channel indicates a cliff.
     */
    int32 XWalkLineTracker::cliffThreshold() const noexcept
    {
        return cliffThresholdValue;
    }

    /**
     * @brief Replaces the cliff threshold.
     *
     * @param[in] threshold
     * Calibrated ADC count below which a channel indicates a cliff.
     *
     * @post
     * `cliffThreshold()` equals `threshold`.
     */
    void XWalkLineTracker::setCliffThreshold(int32 threshold) noexcept
    {
        cliffThresholdValue = threshold;
    }

    /**
     * @brief Returns the adaptive background reference.
     *
     * @return
     * Background reference in calibrated ADC counts.
     */
    float64 XWalkLineTracker::lineBackgroundReference() const noexcept
    {
        return lineBackgroundReferenceValue;
    }

    /**
     * @brief Returns the adaptive dark-line reference.
     *
     * @return
     * Dark-line reference in calibrated ADC counts.
     */
    float64 XWalkLineTracker::lineReference() const noexcept
    {
        return lineReferenceValue;
    }

} /* namespace xwalk::hal */
