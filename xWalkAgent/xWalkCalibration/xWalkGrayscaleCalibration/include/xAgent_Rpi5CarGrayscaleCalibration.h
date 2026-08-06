/******************************************************************************
 * @file        xAgent_Rpi5CarGrayscaleCalibration.h
 * @brief       Declares bounded automatic grayscale calibration.
 *
 * @details
 * Ports the motion, sampling, reference calculation, and deferred persistence
 * from the upstream `1.cali_grayscale.py` helper.
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

#ifndef XAGENT_RPI5CAR_GRAYSCALE_CALIBRATION_H
#define XAGENT_RPI5CAR_GRAYSCALE_CALIBRATION_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarGrayscaleCalibrationTypes.h"
#include "xAgent_Rpi5CarPicarx.h"

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
 * Class declarations
 ******************************************************************************/

/** @brief Coordinates bounded motion and sampling for grayscale calibration. */
class XWalkGrayscaleCalibration final
{
private:

    /** @brief Non-owning PiCar-X dependency that must outlive this coordinator. */
    XWalkPicarx* picarxObject{nullptr};
    /** @brief Nullable non-owning context forwarded to both callbacks. */
    agent::contextpointer callbackContext{nullptr};
    /** @brief Non-null synchronous timing callback. */
    grayscalecalibrationdelaycallback delayCallback{nullptr};
    /** @brief Non-null synchronous cancellation callback. */
    grayscalecalibrationcontinuecallback continueCallback{nullptr};
    /** @brief Smallest observed ADC value for each line-sampling channel. */
    hal::linetrackervalues lineMinimumValues{4'096, 4'096, 4'096};
    /** @brief Largest observed ADC value for each line-sampling channel. */
    hal::linetrackervalues lineMaximumValues{0, 0, 0};
    /** @brief Pending references retained until the caller confirms persistence. */
    XWalkGrayscaleCalibrationResult resultValue{};

protected:

    /** @brief Stops the motors and centers steering after cancellation. */
    void stop() noexcept;
    /** @brief Delays after confirming the operation may continue. */
    agent::boolean wait(agent::uint32 durationMs) const;
    /** @brief Samples once and updates line-channel extrema. */
    void sampleLine();
    /** @brief Samples line extrema at 200-millisecond intervals. */
    agent::boolean sampleLineFor(agent::uint32 durationMs);
    /** @brief Applies one movement phase and collects line extrema. */
    agent::boolean runLinePhase(agent::float64 steeringDegrees,
        agent::boolean forwardMovement, agent::uint32 durationMs);

public:

    /**
     * @brief Binds one PiCar-X coordinator and injected scheduling operations.
     * @param[in] picarx PiCar-X coordinator that must outlive this object.
     * @param[in,out] context Optional callback context that must outlive this object.
     * @param[in] delayOperation Non-null synchronous delay operation.
     * @param[in] continueOperation Non-null synchronous cancellation query.
     * @throws std::invalid_argument If either callback is null.
     */
    XWalkGrayscaleCalibration(XWalkPicarx& picarx, agent::contextpointer context,
        grayscalecalibrationdelaycallback delayOperation,
        grayscalecalibrationcontinuecallback continueOperation);

    /** @brief Stops drive motors without releasing the observed PiCar-X object. */
    ~XWalkGrayscaleCalibration();

    /** @brief Prevents copying of non-owning dependency bindings. */
    XWalkGrayscaleCalibration(const XWalkGrayscaleCalibration&) = delete;
    /** @brief Prevents moving of non-owning dependency bindings. */
    XWalkGrayscaleCalibration(XWalkGrayscaleCalibration&&) = delete;
    /** @brief Prevents copy assignment of non-owning dependency bindings. */
    XWalkGrayscaleCalibration& operator=(const XWalkGrayscaleCalibration&) = delete;
    /** @brief Prevents move assignment of non-owning dependency bindings. */
    XWalkGrayscaleCalibration& operator=(XWalkGrayscaleCalibration&&) = delete;

    /**
     * @brief Sweeps steering through minus 30, plus 30, and zero degrees.
     * @return `true` after completion or `false` after cancellation.
     * @warning Physically moves the steering mechanism.
     */
    agent::boolean runSteeringCheck();

    /**
     * @brief Drives the source left/right pattern while deriving line references.
     * @return `true` after completion or `false` after cancellation.
     * @warning Drives forward and backward; wheels require a reviewed clear surface.
     */
    agent::boolean calibrateLine();

    /**
     * @brief Averages ten stationary samples into pending cliff references.
     * @return `true` after completion or `false` after cancellation.
     */
    agent::boolean calibrateCliff();

    /** @brief Persists both pending reference arrays through PiCar-X. */
    void save();

    /**
     * @brief Returns pending values without persisting them.
     * @return Non-owning result reference valid for this object lifetime.
     */
    const XWalkGrayscaleCalibrationResult& result() const noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_GRAYSCALE_CALIBRATION_H */
