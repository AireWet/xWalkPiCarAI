/******************************************************************************
 * @file        xAgent_Rpi5CarCliffDetection.h
 * @brief       Declares bounded grayscale cliff detection.
 *
 * @details
 * Ports the safe/danger movement state machine from upstream
 * `example/5.cliff_detection.py` while preserving project-owned calibration.
 *
 * @project     xWalk Firmware
 * @module      xWalkCliffDetection
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

#ifndef XAGENT_RPI5CAR_CLIFF_DETECTION_H
#define XAGENT_RPI5CAR_CLIFF_DETECTION_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarCliffDetectionTypes.h"
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

/**
 * @class XWalkCliffDetection
 * @brief Acquires one grayscale sample and applies the cliff response.
 *
 * @details
 * Observes a caller-owned PiCar-X coordinator and scheduling callbacks. It
 * retains only whether the preceding completed sample was dangerous.
 */
class XWalkCliffDetection final
{
private:
    /**************************************************************************
     * Private data members
     **************************************************************************/

    /** @brief Non-owning PiCar-X pointer that remains non-null for this lifetime. */
    XWalkPicarx* picarxObject{nullptr};
    /** @brief Nullable non-owning context forwarded synchronously to callbacks. */
    agent::contextpointer callbackContext{nullptr};
    /** @brief Non-null synchronous timing callback. */
    cliffdetectiondelaycallback delayCallback{nullptr};
    /** @brief Non-null synchronous cancellation callback. */
    cliffdetectioncontinuecallback continueCallback{nullptr};
    /** @brief True when the preceding completed sample selected danger behavior. */
    agent::boolean lastDangerValue{};

protected:
    /**************************************************************************
     * Protected member functions
     **************************************************************************/

    /** @brief Waits in cancellable slices no longer than 20 milliseconds. */
    agent::boolean wait(agent::uint32 durationMs) const;

public:
    /**************************************************************************
     * Public constructors and destructor
     **************************************************************************/

    /**
     * @brief Binds caller-owned vehicle and synchronous scheduling operations.
     * @param[in] picarx PiCar-X coordinator that must outlive this Agent.
     * @param[in,out] context Optional callback context that must outlive this Agent.
     * @param[in] delayOperation Non-null synchronous delay operation.
     * @param[in] continueOperation Non-null synchronous cancellation query.
     * @throws std::invalid_argument If either callback is null.
     */
    XWalkCliffDetection(XWalkPicarx& picarx, agent::contextpointer context,
        cliffdetectiondelaycallback delayOperation,
        cliffdetectioncontinuecallback continueOperation);

    /** @brief Performs a non-throwing emergency motor stop without releasing dependencies. */
    ~XWalkCliffDetection() noexcept;

    XWalkCliffDetection(const XWalkCliffDetection&) = delete;
    XWalkCliffDetection(XWalkCliffDetection&&) = delete;
    XWalkCliffDetection& operator=(const XWalkCliffDetection&) = delete;
    XWalkCliffDetection& operator=(XWalkCliffDetection&&) = delete;

    /**************************************************************************
     * Public member functions
     **************************************************************************/

    /**
     * @brief Acquires grayscale data and applies one source-compatible response.
     * @return Safe, danger, or cancelled step result.
     * @warning Danger commands physical reverse movement at 80-percent requested speed.
     */
    XWalkCliffDetectionResult step();

    /** @brief Stops both motors and resets the retained state to safe. */
    void stop();

    /**
     * @brief Reports whether the preceding completed sample selected danger.
     * @return `true` for danger or `false` for safe/reset state.
     */
    agent::boolean lastDanger() const noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_CLIFF_DETECTION_H */
