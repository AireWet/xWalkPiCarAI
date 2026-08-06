/******************************************************************************
 * @file        xAgent_Rpi5CarMoveExample.h
 * @brief       Declares the bounded movement example coordinator.
 *
 * @details
 * Ports the forward, steering, camera-pan, and camera-tilt sequence from
 * upstream `example/2.move.py` while providing cancellation and final cleanup.
 *
 * @project     xWalk Firmware
 * @module      xWalkMoveExample
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_MOVE_EXAMPLE_H
#define XAGENT_RPI5CAR_MOVE_EXAMPLE_H

#include "xAgent_Rpi5CarMoveExampleTypes.h"
#include "xAgent_Rpi5CarPicarx.h"

namespace xwalk::agent
{

/** @brief Coordinates the complete bounded sequence from upstream `2.move.py`. */
class XWalkMoveExample final
{
private:
    /** @brief Non-owning PiCar-X dependency that must outlive this coordinator. */
    XWalkPicarx* picarxObject{nullptr};
    /** @brief Nullable non-owning context forwarded to both callbacks. */
    agent::contextpointer callbackContext{nullptr};
    /** @brief Non-null synchronous timing callback. */
    moveexampledelaycallback delayCallback{nullptr};
    /** @brief Non-null synchronous cancellation callback. */
    moveexamplecontinuecallback continueCallback{nullptr};

protected:
    /** @brief Waits in cancellable slices no longer than 20 milliseconds. */
    agent::boolean wait(agent::uint32 durationMs) const;
    /** @brief Stops both motors without allowing exceptions to escape. */
    void stop() noexcept;
    /** @brief Commands one logical angle on steering, pan, or tilt. */
    void setServoAngle(agent::uint8 servoId, agent::float64 angleDegrees);
    /** @brief Runs the three source-compatible angle loops on one servo. */
    agent::boolean sweepServo(agent::uint8 servoId);

public:
    /** @brief Binds caller-owned vehicle and synchronous scheduling callbacks. */
    XWalkMoveExample(XWalkPicarx& picarx, agent::contextpointer context,
        moveexampledelaycallback delayOperation,
        moveexamplecontinuecallback continueOperation);

    /** @brief Stops drive motors without releasing the observed PiCar-X object. */
    ~XWalkMoveExample();

    XWalkMoveExample(const XWalkMoveExample&) = delete;
    XWalkMoveExample(XWalkMoveExample&&) = delete;
    XWalkMoveExample& operator=(const XWalkMoveExample&) = delete;
    XWalkMoveExample& operator=(XWalkMoveExample&&) = delete;

    /**
     * @brief Runs the forward, steering, pan, and tilt movement example.
     * @return `true` after completion or `false` after cancellation.
     * @warning Physically moves both drive motors and all three servos.
     */
    agent::boolean run();
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_MOVE_EXAMPLE_H */
