/******************************************************************************
 * @file        xHal_Rpi5CarGrayscaleModule.cpp
 * @brief       Implements three-channel grayscale sensing and classification.
 *
 * @details
 * Binds caller-owned ADC channels, reads raw samples, manages reference
 * thresholds, and classifies each channel as black or white.
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

#include "xHal_Rpi5CarGrayscaleModule.h"

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
    XWalkAdc& XWalkGrayscaleModule::sensorAt(uint32 channel) const
    {
        if (channel >= XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Grayscale channel must be in the range zero through two");
        }
        return *sensors[channel];
    }

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs a three-channel grayscale module.
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
    XWalkGrayscaleModule::XWalkGrayscaleModule(XWalkAdc& left, XWalkAdc& middle, XWalkAdc& right)
        : sensors{&left, &middle, &right}
    {
        XWALK_HAL_TRACE_UID0(RPI .232, "Grayscale module constructed");
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Destroys the module without releasing its non-owning ADC dependencies.
     */
    XWalkGrayscaleModule::~XWalkGrayscaleModule() = default;

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Replaces all black/white reference thresholds.
     *
     * @param[in] references
     * Left, middle, and right reference values in raw ADC counts.
     *
     * @post
     * `reference()` equals `references`.
     */
    void XWalkGrayscaleModule::setReference(const linetrackervalues& references) noexcept
    {
        referenceValues = references;
    }

    /**
     * @brief Returns the active black/white thresholds.
     *
     * @return
     * Read-only left, middle, and right reference values in raw ADC counts.
     */
    const linetrackervalues& XWalkGrayscaleModule::reference() const noexcept
    {
        return referenceValues;
    }

    /**
     * @brief Reads one raw grayscale channel.
     *
     * @param[in] channel
     * Channel index in the inclusive range zero through two.
     *
     * @return
     * Raw ADC sample count.
     *
     * @throws outofrange
     * If `channel` exceeds two.
     *
     * @throws runtimeerror
     * If the ADC transaction does not return a complete sample.
     */
    int32 XWalkGrayscaleModule::readChannel(uint32 channel)
    {
        return static_cast<int32>(sensorAt(channel).read());
    }

    /**
     * @brief Reads all three raw grayscale channels.
     *
     * @return
     * Left, middle, and right ADC sample counts.
     */
    linetrackervalues XWalkGrayscaleModule::read()
    {
        linetrackervalues data{};
        for (uint32 channel = 0U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
        {
            data[channel] = readChannel(channel);
        }
        XWALK_HAL_TRACE_UID0(RPI .234, "Grayscale three-channel read completed");
        return data;
    }

    /**
     * @brief Reads and classifies all three grayscale channels.
     *
     * @return
     * Zero for values above their references and one for values at or below them.
     */
    linetrackerstatus XWalkGrayscaleModule::readStatus()
    {
        return readStatus(read());
    }

    /**
     * @brief Classifies supplied grayscale values.
     *
     * @param[in] data
     * Left, middle, and right values in ADC counts.
     *
     * @return
     * Zero for values above their references and one for values at or below them.
     */
    linetrackerstatus XWalkGrayscaleModule::readStatus(const linetrackervalues& data) const noexcept
    {
        linetrackerstatus status{};
        for (uint32 channel = 0U; channel < XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT; ++channel)
        {
            status[channel] = (data[channel] > referenceValues[channel]) ? 0U : 1U;
        }
        return status;
    }

} /* namespace xwalk::hal */
