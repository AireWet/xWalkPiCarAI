/******************************************************************************
 * @file        xHal_Rpi5CarRobotPosition.cpp
 * @brief       Implements xWalk robot positioning and calibration behavior.
 *
 * @details
 * Applies raw and relative servo frames, persists bounded offsets, configures
 * origins and directions, and provides calibration and reset operations.
 *
 * @project     xWalk Firmware
 * @module      xWalkRobot
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

#include "xHal_Rpi5CarRobot.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Writes one physical angle per servo without changing logical
 * positions.
 *
 * @param[in] angles
 * One finite physical angle in degrees per servo.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @throws invalidargument
 * If the vector count is wrong or a value is non-finite.
 */
void XWalkRobot::servoWriteRaw(const float64vector &angles) {
  requireInitialized();
  validateAngles(angles, "Raw servo angle");
  for (uint32 index = 0U; index < servoCountValue; ++index) {
    servoAt(index).setAngle(angles[index]);
  }
}

/**
 * @brief Writes logical angles after applying origin, direction, and offset
 * values.
 *
 * @param[in] angles
 * One finite logical angle in degrees per servo.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @throws invalidargument
 * If the vector count is wrong or a value is non-finite.
 */
void XWalkRobot::servoWriteAll(const float64vector &angles) {
  requireInitialized();
  validateAngles(angles, "Logical servo angle");
  float64vector physicalAngles = newList(0.0);
  for (uint32 index = 0U; index < servoCountValue; ++index) {
    const float64 originAndCommand =
        originPositionsValue[index] + angles[index];
    const float64 adjustedAngle = originAndCommand + offsetValues[index];
    physicalAngles[index] = directionValues[index] * adjustedAngle;
  }
  servoWriteRaw(physicalAngles);
}

/**
 * @brief Clamps and persists one calibration offset per servo.
 *
 * @param[in] offsets
 * One finite requested offset in degrees per servo.
 *
 * @throws runtimeerror
 * If the robot is uninitialized or persistent storage cannot be updated.
 *
 * @throws invalidargument
 * If the vector count is wrong or a value is non-finite.
 */
void XWalkRobot::setOffsets(const float64vector &offsets) {
  requireInitialized();
  validateAngles(offsets, "Robot offset");
  float64vector clampedOffsets = offsets;
  for (float64 &offset : clampedOffsets) {
    if (offset < XHAL_RPI5CAR_ROBOT_MIN_OFFSET_DEG) {
      offset = XHAL_RPI5CAR_ROBOT_MIN_OFFSET_DEG;
    }
    if (offset > XHAL_RPI5CAR_ROBOT_MAX_OFFSET_DEG) {
      offset = XHAL_RPI5CAR_ROBOT_MAX_OFFSET_DEG;
    }
  }
  configStore->set(offsetKeyValue, serializeOffsets(clampedOffsets));
  offsetValues = clampedOffsets;
  XWALK_HAL_TRACE_UID1(RPI .345, "Robot persisted %zu servo offset(s)",
                       offsetValues.size());
}

/**
 * @brief Sets one origin angle in degrees per servo.
 *
 * @param[in] origins
 * One finite origin angle in degrees per servo.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @throws invalidargument
 * If the vector count is wrong or a value is non-finite.
 */
void XWalkRobot::setOriginPositions(const float64vector &origins) {
  requireInitialized();
  validateAngles(origins, "Robot origin");
  originPositionsValue = origins;
  XWALK_HAL_TRACE_UID1(RPI .346, "Robot updated %zu servo origin(s)",
                       origins.size());
}

/**
 * @brief Sets one calibration target angle in degrees per servo.
 *
 * @param[in] positions
 * One finite calibration target in degrees per servo.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @throws invalidargument
 * If the vector count is wrong or a value is non-finite.
 */
void XWalkRobot::setCalibrationPositions(const float64vector &positions) {
  requireInitialized();
  validateAngles(positions, "Robot calibration position");
  calibrationPositionsValue = positions;
  XWALK_HAL_TRACE_UID1(RPI .347, "Robot updated %zu calibration target(s)",
                       positions.size());
}

/**
 * @brief Sets one finite direction multiplier per servo.
 *
 * @param[in] directions
 * One finite direction multiplier per servo, normally positive or negative one.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @throws invalidargument
 * If the vector count is wrong or a value is non-finite.
 */
void XWalkRobot::setDirections(const float64vector &directions) {
  requireInitialized();
  validateAngles(directions, "Robot direction");
  directionValues = directions;
  XWALK_HAL_TRACE_UID1(RPI .348, "Robot updated %zu servo direction(s)",
                       directions.size());
}

/**
 * @brief Moves all servos to the configured calibration positions.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @post
 * Current logical positions equal the configured calibration frame.
 */
void XWalkRobot::calibration() {
  requireInitialized();
  servoPositionsValue = calibrationPositionsValue;
  servoWriteAll(servoPositionsValue);
  XWALK_HAL_TRACE_UID0(RPI .349, "Robot moved to its calibration frame");
}

/**
 * @brief Resets logical positions to zero and writes the resulting angles.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @post
 * Every current logical position is zero degrees.
 */
void XWalkRobot::reset() { reset(newList(0.0)); }

/**
 * @brief Resets logical positions to a supplied frame and writes it.
 *
 * @param[in] positions
 * One finite logical angle in degrees per servo.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 *
 * @throws invalidargument
 * If the vector count is wrong or a value is non-finite.
 *
 * @post
 * Current logical positions equal `positions`.
 */
void XWalkRobot::reset(const float64vector &positions) {
  requireInitialized();
  validateAngles(positions, "Robot reset position");
  servoPositionsValue = positions;
  servoWriteAll(servoPositionsValue);
  XWALK_HAL_TRACE_UID1(RPI .350, "Robot reset to a %zu-servo frame",
                       positions.size());
}

/**
 * @brief Writes the zero frame without modifying stored logical positions.
 *
 * @throws runtimeerror
 * If the robot has not been initialized.
 */
void XWalkRobot::softReset() {
  requireInitialized();
  servoWriteAll(newList(0.0));
  XWALK_HAL_TRACE_UID0(RPI .351, "Robot soft reset wrote the zero frame");
}

/**
 * @brief Returns the number of registered servos.
 *
 * @return
 * Servo count in the range 0 through 12.
 */
uint32 XWalkRobot::servoCount() const noexcept { return servoCountValue; }

/**
 * @brief Returns whether initialization completed.
 *
 * @return
 * `true` after successful initialization; otherwise `false`.
 */
boolean XWalkRobot::initialized() const noexcept { return initializedValue; }

/**
 * @brief Returns current logical positions in degrees.
 *
 * @return
 * Read-only reference valid for the robot lifetime or until its next mutation.
 */
const float64vector &XWalkRobot::servoPositions() const noexcept {
  return servoPositionsValue;
}

/**
 * @brief Returns persisted calibration offsets in degrees.
 *
 * @return
 * Read-only reference valid for the robot lifetime or until offsets are
 * changed.
 */
const float64vector &XWalkRobot::offsets() const noexcept {
  return offsetValues;
}

} /* namespace xwalk::hal */
