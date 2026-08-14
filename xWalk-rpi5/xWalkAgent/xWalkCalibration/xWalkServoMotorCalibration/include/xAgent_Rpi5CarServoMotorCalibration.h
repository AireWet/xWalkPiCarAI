/******************************************************************************
 * @file        xAgent_Rpi5CarServoMotorCalibration.h
 * @brief       Declares bounded servo and motor calibration behavior.
 *
 * @details
 * Ports pending offsets, servo tests, motor direction changes, motor preview,
 * and explicit persistence from upstream `1.cali_servo_motor.py`.
 *
 * @project     xWalk Firmware
 * @module      xWalkServoMotorCalibration
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

#ifndef XAGENT_RPI5CAR_SERVO_MOTOR_CALIBRATION_H
#define XAGENT_RPI5CAR_SERVO_MOTOR_CALIBRATION_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarServoMotorCalibrationTypes.h"

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

/** @brief Coordinates source-compatible pending servo and motor calibration. */
class XWalkServoMotorCalibration final
{
private:

    /** @brief Non-owning PiCar-X dependency that must outlive this coordinator. */
    XWalkPicarx* picarxObject{nullptr};
    /** @brief Nullable non-owning context forwarded to both callbacks. */
    agent::contextpointer callbackContext{nullptr};
    /** @brief Non-null synchronous timing callback. */
    servomotorcalibrationdelaycallback delayCallback{nullptr};
    /** @brief Non-null synchronous cancellation callback. */
    servomotorcalibrationcontinuecallback continueCallback{nullptr};
    /** @brief Pending calibration state initialized from PiCar-X. */
    XWalkServoMotorCalibrationResult resultValue{};
protected:

    /** @brief Waits in cancellable slices no longer than 20 milliseconds. */
    agent::boolean wait(agent::uint32 durationMs) const;
    /** @brief Stops both motors without allowing exceptions to escape. */
    void stop() noexcept;
    /** @brief Applies one pending servo offset and centers its logical command. */
    agent::boolean applyServoOffset(agent::uint8 servoId);
    /** @brief Commands one logical angle on a selected servo. */
    void setServoAngle(agent::uint8 servoId, agent::float64 angleDegrees);

public:

    /**
     * @brief Binds one PiCar-X coordinator and injected scheduling operations.
     * @param[in] picarx PiCar-X coordinator that must outlive this object.
     * @param[in,out] context Optional callback context that must outlive this object.
     * @param[in] delayOperation Non-null synchronous delay operation.
     * @param[in] continueOperation Non-null synchronous cancellation query.
     * @throws std::invalid_argument If either callback is null.
     */
    XWalkServoMotorCalibration(XWalkPicarx& picarx, agent::contextpointer context,
        servomotorcalibrationdelaycallback delayOperation,
        servomotorcalibrationcontinuecallback continueOperation);

    /** @brief Stops drive motors without releasing the observed PiCar-X object. */
    ~XWalkServoMotorCalibration();

    /** @brief Prevents copying of non-owning dependency bindings. */
    XWalkServoMotorCalibration(const XWalkServoMotorCalibration&) = delete;
    /** @brief Prevents moving of non-owning dependency bindings. */
    XWalkServoMotorCalibration(XWalkServoMotorCalibration&&) = delete;
    /** @brief Prevents copy assignment of non-owning dependency bindings. */
    XWalkServoMotorCalibration& operator=(const XWalkServoMotorCalibration&) = delete;
    /** @brief Prevents move assignment of non-owning dependency bindings. */
    XWalkServoMotorCalibration& operator=(XWalkServoMotorCalibration&&) = delete;

    /**
     * @brief Centers all three logical servo commands with source timing.
     * @return `true` after completion or `false` after cancellation.
     */
    agent::boolean resetServos();

    /**
     * @brief Sweeps steering, pan, and tilt through minus 30, plus 30, and zero.
     * @return `true` after all nine commands or `false` after cancellation.
     * @warning Physically moves all three servo mechanisms.
     */
    agent::boolean testServos();

    /**
     * @brief Applies one pending servo offset without persisting it.
     * @param[in] servoId Zero for steering, one for pan, or two for tilt.
     * @param[in] offsetDegrees Offset from minus 20 through plus 20 degrees.
     * @return `true` after centered preview or `false` after cancellation.
     */
    agent::boolean setServoOffset(agent::uint8 servoId, agent::float64 offsetDegrees);

    /**
     * @brief Applies one pending motor direction without persisting it.
     * @param[in] motorId One for left or two for right.
     * @param[in] direction Direction equal to 1 or -1.
     */
    void setMotorDirection(agent::uint8 motorId, agent::int32 direction);

    /**
     * @brief Reverses one pending motor direction and starts source preview power.
     * @param[in] motorId One for left or two for right.
     */
    void toggleMotorDirection(agent::uint8 motorId);

    /**
     * @brief Starts or stops the source-compatible 30-percent forward preview.
     * @param[in] running `true` to run or `false` to stop both motors.
     */
    void setMotorRunning(agent::boolean running);

    /** @brief Persists all pending servo offsets and motor directions. */
    void save();

    /**
     * @brief Returns pending values without persisting them.
     * @return Non-owning result reference valid for this object lifetime.
     */
    const XWalkServoMotorCalibrationResult& result() const noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_SERVO_MOTOR_CALIBRATION_H */
