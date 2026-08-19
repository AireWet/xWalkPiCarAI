/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxSensing.cpp
 * @brief       Implements PiCar-X grayscale and distance operations.
 *
 * @details
 * Delegates physical acquisition to caller-owned HAL objects and persists sensing thresholds.
 *
 * @project     xWalk Firmware
 * @module      xWalkPicarx
 *
 * @author      Joxy John
 * @date        2026-07-31
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

#include "xAgent_Rpi5CarPicarx.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains application coordinators for the xWalk firmware. */
namespace xwalk::agent
{

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /** @brief Returns one ultrasonic distance measurement in centimeters. */
    agent::float64 XWalkPicarx::distance()
    {
        return ultrasonicObject->read();
    }

    /** @brief Sets and persists all grayscale line references. */
    void XWalkPicarx::setGrayscaleReference(const hal::linetrackervalues& value)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .082, "PiCar-X grayscale reference update requested");
        grayscaleObject->setReference(value);
        configStoreObject->set("line_reference", formatReferences(value));
    }

    /**
     * @brief Returns the active grayscale line references.
     * @return Non-owning reference valid for the PiCar-X coordinator lifetime.
     */
    const hal::linetrackervalues& XWalkPicarx::grayscaleReference() const noexcept
    {
        return grayscaleObject->reference();
    }

    /** @brief Returns current raw grayscale data. */
    hal::linetrackervalues XWalkPicarx::grayscaleData()
    {
        return grayscaleObject->read();
    }

    /** @brief Classifies supplied grayscale data against the line references. */
    hal::linetrackerstatus XWalkPicarx::lineStatus(const hal::linetrackervalues& value) const noexcept
    {
        return grayscaleObject->readStatus(value);
    }

    /** @brief Returns true when any supplied channel is at or below its cliff threshold. */
    agent::boolean XWalkPicarx::cliffStatus(const hal::linetrackervalues& value) const noexcept
    {
        for (agent::uint32 index = 0U; index < 3U; ++index)
        {
            if (value[index] <= cliffReferenceValues[index])
            {
                return true;
            }
        }
        return false;
    }

    /** @brief Sets and persists all cliff thresholds. */
    void XWalkPicarx::setCliffReference(const hal::linetrackervalues& value)
    {
        cliffReferenceValues = value;
        configStoreObject->set("cliff_reference", formatReferences(value));
    }

    /**
     * @brief Returns the active cliff thresholds.
     * @return Non-owning reference valid for the PiCar-X coordinator lifetime.
     */
    const hal::linetrackervalues& XWalkPicarx::cliffReference() const noexcept
    {
        return cliffReferenceValues;
    }

} /* namespace xwalk::agent */
