/******************************************************************************
 * @file        xHal_Rpi5CarLineTracker.h
 * @brief       Declares the calibrated line-tracking interface.
 *
 * @details
 * Defines calibrated three-channel sampling, cliff and line detection,
 * adaptive references, and weighted line-position estimation.
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

#ifndef XHAL_RPI5CAR_LINE_TRACKER_H
#define XHAL_RPI5CAR_LINE_TRACKER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarLineTrackerTypes.h"

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
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkLineTracker
 * @brief Calibrates three grayscale sensors and estimates line position.
 *
 * @details
 * Applies per-channel linear calibration, detects cliff and line conditions,
 * estimates a normalized position, and adapts dark-line and background references.
 */
class XWalkLineTracker
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning ADC pointers that are never null after construction. */
        linetrackeradcpointers sensors{};

        /** @brief Active finite per-channel linear calibration coefficients. */
        XWalkLineCalibration calibrationValue{};

        /** @brief Adaptive calibrated background reference in ADC counts. */
        float64 lineBackgroundReferenceValue{XHAL_RPI5CAR_LINE_TRACKER_BACKGROUND_REFERENCE};

        /** @brief Adaptive calibrated dark-line reference in ADC counts. */
        float64 lineReferenceValue{XHAL_RPI5CAR_LINE_TRACKER_LINE_REFERENCE};

        /** @brief Calibrated count below which any sensor indicates a cliff. */
        int32 cliffThresholdValue{XHAL_RPI5CAR_LINE_TRACKER_DEFAULT_CLIFF_THRESHOLD};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

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
        XWalkAdc& sensorAt(uint32 channel) const;

        /**
         * @brief Validates that all calibration coefficients are finite.
         *
         * @param[in] calibration
         * Coefficients to validate.
         *
         * @throws invalidargument
         * If any slope or offset is non-finite.
         */
        static void validateCalibration(const XWalkLineCalibration& calibration);

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
        static int32 roundReading(float64 value);

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
        static float64 constrain(float64 value, float64 minimum, float64 maximum) noexcept;

        /**
         * @brief Rounds a finite value to two decimal places.
         *
         * @param[in] value
         * Finite value to round.
         *
         * @return
         * Value rounded using the active floating-point rounding mode.
         */
        static float64 roundTwoDecimals(float64 value) noexcept;

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

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
        XWalkLineTracker(XWalkAdc& left, XWalkAdc& middle, XWalkAdc& right);

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
        XWalkLineTracker(XWalkAdc& left, XWalkAdc& middle, XWalkAdc& right,
            const XWalkLineCalibration& calibration);

        /** @brief Destroys the tracker without releasing its non-owning ADC dependencies. */
        ~XWalkLineTracker();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction to preserve dependency identities. */
        XWalkLineTracker(XWalkLineTracker&&) = delete;
        /** @brief Disables copying of ADC dependency bindings. */
        XWalkLineTracker(const XWalkLineTracker&) = delete;
        /** @brief Disables move assignment of ADC dependency bindings. */
        XWalkLineTracker& operator=(XWalkLineTracker&&) = delete;
        /** @brief Disables copy assignment of ADC dependency bindings. */
        XWalkLineTracker& operator=(const XWalkLineTracker&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Reads one raw or calibrated line-tracker channel.
         *
         * @param[in] channel
         * Channel index in the inclusive range zero through two.
         *
         * @param[in] raw
         * `true` returns the ADC count without calibration; `false` applies calibration.
         *
         * @return
         * Raw or calibrated ADC count.
         *
         * @throws outofrange
         * If the channel or calibrated result exceeds its supported range.
         *
         * @throws invalidargument
         * If calibration produces a non-finite result.
         *
         * @throws runtimeerror
         * If the ADC transaction does not return a complete sample.
         */
        int32 readChannel(uint32 channel, boolean raw = false);

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
        linetrackervalues calibrateData(const linetrackervalues& data) const;

        /**
         * @brief Reads all three raw or calibrated line-tracker channels.
         *
         * @param[in] raw
         * `true` returns raw ADC counts; `false` applies active calibration.
         *
         * @return
         * Left, middle, and right raw or calibrated counts.
         */
        linetrackervalues read(boolean raw = false);

        /**
         * @brief Reads all channels and checks whether any indicates a cliff.
         *
         * @return
         * `true` when any calibrated count is below the cliff threshold; otherwise `false`.
         */
        boolean isOnCliff();

        /**
         * @brief Checks supplied readings for a cliff condition.
         *
         * @param[in] data
         * Left, middle, and right calibrated counts.
         *
         * @return
         * `true` when any count is below the cliff threshold; otherwise `false`.
         */
        boolean isOnCliff(const linetrackervalues& data) const noexcept;

        /**
         * @brief Reads all channels and estimates normalized line position.
         *
         * @return
         * Position from -1.0 at the left through 0.0 at center to 1.0 at the right.
         *
         * @throws runtimeerror
         * If the adaptive background and line references are equal.
         */
        float64 getLinePosition();

        /**
         * @brief Estimates normalized line position from supplied calibrated readings.
         *
         * @param[in] data
         * Left, middle, and right calibrated counts.
         *
         * @return
         * Position in the inclusive range -1.0 through 1.0, rounded to two decimals.
         *
         * @throws runtimeerror
         * If the adaptive background and line references are equal.
         *
         * @post
         * Adaptive references are updated when the detected pattern is not edge-only.
         */
        float64 getLinePosition(const linetrackervalues& data);

        /**
         * @brief Reads all channels and checks whether a line is detected.
         *
         * @return
         * `true` when no cliff exists and channel spread exceeds 200 counts.
         */
        boolean isOnLine();

        /**
         * @brief Checks supplied readings for a detected line.
         *
         * @param[in] data
         * Left, middle, and right calibrated counts.
         *
         * @return
         * `true` when no cliff exists and channel spread exceeds 200 counts.
         */
        boolean isOnLine(const linetrackervalues& data) const noexcept;

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
        void setCalibrationData(const XWalkLineCalibration& calibration);

        /**
         * @brief Returns the active cliff threshold.
         *
         * @return
         * Calibrated ADC count below which a channel indicates a cliff.
         */
        int32 cliffThreshold() const noexcept;

        /**
         * @brief Replaces the cliff threshold.
         *
         * @param[in] threshold
         * Calibrated ADC count below which a channel indicates a cliff.
         *
         * @post
         * `cliffThreshold()` equals `threshold`.
         */
        void setCliffThreshold(int32 threshold) noexcept;

        /**
         * @brief Returns the active linear calibration coefficients.
         *
         * @return
         * Read-only calibration reference valid until the next calibration update.
         */
        const XWalkLineCalibration& calibrationData() const noexcept;

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
         * If a non-reference channel lacks increasing dark-to-light input and output ranges.
         *
         * @post
         * `calibrationData()` equals the returned coefficients.
         */
        XWalkLineCalibration calibrate(const linetrackervalues& lightData,
            const linetrackervalues& darkData);

        /**
         * @brief Adapts the background reference toward the largest current value.
         *
         * @param[in] currentValues
         * Left, middle, and right calibrated ADC counts.
         *
         * @post
         * Five percent of the current maximum is blended into the background reference.
         */
        void updateLineBackgroundReference(const linetrackervalues& currentValues) noexcept;

        /**
         * @brief Adapts the dark-line reference toward the smallest current value.
         *
         * @param[in] currentValues
         * Left, middle, and right calibrated ADC counts.
         *
         * @post
         * Five percent of the current minimum is blended into the dark-line reference.
         */
        void updateLineReference(const linetrackervalues& currentValues) noexcept;

        /**
         * @brief Returns the adaptive background reference.
         *
         * @return
         * Background reference in calibrated ADC counts.
         */
        float64 lineBackgroundReference() const noexcept;

        /**
         * @brief Returns the adaptive dark-line reference.
         *
         * @return
         * Dark-line reference in calibrated ADC counts.
         */
        float64 lineReference() const noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_LINE_TRACKER_H */
