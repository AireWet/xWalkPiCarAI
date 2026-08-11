/******************************************************************************
 * @file        xHal_Rpi5CarServoHatSequence.cpp
 * @brief       Implements the bounded Robot HAT servo and ADC sequence.
 *
 * @details
 * Preserves the upstream reset, servo order, angles, reporting order, and
 * timing while replacing continuous ADC monitoring with bounded sampling.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarServoHatSequence.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test {

/**
 * @brief Binds the board, servos, ADC inputs, and reporting operations.
 *
 * @param[in] boardControl
 * Caller-owned board controller that must outlive the sequence.
 *
 * @param[in] servos
 * Non-null servo pointers ordered by PWM channel zero through 15.
 *
 * @param[in] adcInputs
 * Non-null ADC pointers ordered by channel zero through four.
 *
 * @param[in,out] context
 * Non-owning context forwarded to all callbacks.
 *
 * @param[in] wait
 * Non-null wait callback receiving durations in milliseconds.
 *
 * @param[in] reportServo
 * Non-null callback receiving each servo channel before movement.
 *
 * @param[in] reportAdc
 * Non-null callback receiving every ordered ADC sample.
 *
 * @throws std::invalid_argument
 * If a callback or dependency pointer is null.
 */
XWalkServoHatSequence::XWalkServoHatSequence(XWalkBoardControl &boardControl,
                                             const servohatservoarray &servos,
                                             const servohatadcarray &adcInputs,
                                             contextpointer context,
                                             servohatwaitcallback wait,
                                             servohatservocallback reportServo,
                                             servohatadccallback reportAdc)
    : boardControlObject(&boardControl), servoObjects(servos),
      adcObjects(adcInputs), callbackContext(context), waitCallback(wait),
      servoCallback(reportServo), adcCallback(reportAdc) {
  if ((waitCallback == nullptr) || (servoCallback == nullptr) ||
      (adcCallback == nullptr)) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Servo HAT callbacks must not be null");
  }
  for (const XWalkServo *const servo : servoObjects) {
    if (servo == nullptr) {
      XWALK_HAL_ERROR(XWALK_INVAL, "Servo HAT servo pointers must not be null");
    }
  }
  for (const XWalkAdc *const adc : adcObjects) {
    if (adc == nullptr) {
      XWALK_HAL_ERROR(XWALK_INVAL, "Servo HAT ADC pointers must not be null");
    }
  }
}

/**
 * @brief Resets the MCU, sweeps all servos, and samples all ADC inputs.
 *
 * @param[in] sampleCount
 * Number of five-channel ADC samples, in the inclusive range 1 through 3600.
 *
 * @post
 * On normal completion, every servo's last requested angle is zero degrees.
 *
 * @throws std::out_of_range
 * If `sampleCount` is outside its supported range.
 */
void XWalkServoHatSequence::run(uint32 sampleCount) {
  if ((sampleCount == 0U) ||
      (sampleCount > XHAL_RPI5CAR_SERVO_HAT_MAX_SAMPLES)) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "Servo HAT sample count must be from 1 to 3600");
  }

  boardControlObject->resetMcu();
  waitCallback(callbackContext, 1'000U);

  for (size channel = 0U; channel < servoObjects.size(); ++channel) {
    servoCallback(callbackContext, static_cast<uint8>(channel));
    servoObjects[channel]->setAngle(10.0);
    waitCallback(callbackContext, 100U);
    servoObjects[channel]->setAngle(0.0);
    waitCallback(callbackContext, 100U);
  }

  for (uint32 sample = 0U; sample < sampleCount; ++sample) {
    servohatreadings readings{};
    for (size channel = 0U; channel < adcObjects.size(); ++channel) {
      readings[channel] = adcObjects[channel]->read();
    }
    adcCallback(callbackContext, readings);
    waitCallback(callbackContext, 1'000U);
  }
}

} /* namespace xwalk::hal::test */
