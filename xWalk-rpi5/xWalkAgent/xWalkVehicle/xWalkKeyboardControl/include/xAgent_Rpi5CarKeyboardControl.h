/******************************************************************************
 * @file        xAgent_Rpi5CarKeyboardControl.h
 * @brief       Declares bounded keyboard-driven PiCar-X control.
 *
 * @details
 * Ports the actuator behavior from upstream `example/3.keyboard_control.py`.
 * Terminal input remains an application responsibility.
 *
 * @project     xWalk Firmware
 * @module      xWalkKeyboardControl
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

#ifndef XAGENT_RPI5CAR_KEYBOARD_CONTROL_H
#define XAGENT_RPI5CAR_KEYBOARD_CONTROL_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarKeyboardControlTypes.h"
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
 * @class XWalkKeyboardControl
 * @brief Maps one-character input to bounded PiCar-X movement and camera pulses.
 *
 * @details
 * Observes a caller-owned PiCar-X coordinator and scheduling callbacks. Camera
 * state is retained by value and constrained to minus 30 through plus 30 degrees.
 */
class XWalkKeyboardControl final
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
    keyboardcontroldelaycallback delayCallback{nullptr};
    /** @brief Non-null synchronous cancellation callback. */
    keyboardcontrolcontinuecallback continueCallback{nullptr};
    /** @brief Current logical camera-pan command in degrees. */
    agent::float64 panAngleDegreesValue{};
    /** @brief Current logical camera-tilt command in degrees. */
    agent::float64 tiltAngleDegreesValue{};

protected:
    /**************************************************************************
     * Protected member functions
     **************************************************************************/

    /** @brief Waits in cancellable slices no longer than 20 milliseconds. */
    agent::boolean wait(agent::uint32 durationMs) const;
    /** @brief Applies both retained camera angles to the observed vehicle. */
    void applyCameraAngles();

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
    XWalkKeyboardControl(XWalkPicarx& picarx, agent::contextpointer context,
        keyboardcontroldelaycallback delayOperation,
        keyboardcontrolcontinuecallback continueOperation);

    /** @brief Performs a non-throwing emergency motor stop without releasing dependencies. */
    ~XWalkKeyboardControl() noexcept;

    XWalkKeyboardControl(const XWalkKeyboardControl&) = delete;
    XWalkKeyboardControl(XWalkKeyboardControl&&) = delete;
    XWalkKeyboardControl& operator=(const XWalkKeyboardControl&) = delete;
    XWalkKeyboardControl& operator=(XWalkKeyboardControl&&) = delete;

    /**************************************************************************
     * Public member functions
     **************************************************************************/

    /**
     * @brief Processes one key and performs the upstream 500-millisecond pulse.
     * @param[in] keyText One-character `w`, `s`, `a`, `d`, `i`, `k`, `j`, or `l` input.
     * @return Handled, ignored, or cancelled processing status.
     * @warning A movement key drives the physical car at 80-percent requested speed.
     */
    XWalkKeyboardControlResult handleKey(agent::stringview keyText);

    /**
     * @brief Centers steering and camera servos, stops motors, and waits 200 milliseconds.
     * @warning Physically moves all three servos to their logical centers.
     */
    void finish();

    /**
     * @brief Returns the retained logical camera-pan angle.
     * @return Current value from minus 30 through plus 30 degrees.
     */
    agent::float64 panAngleDegrees() const noexcept;
    /**
     * @brief Returns the retained logical camera-tilt angle.
     * @return Current value from minus 30 through plus 30 degrees.
     */
    agent::float64 tiltAngleDegrees() const noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_KEYBOARD_CONTROL_H */
