/******************************************************************************
 * @file        xHal_Rpi5CarPwmTimer.cpp
 * @brief       Implements PWM frequency, prescaler, and period configuration.
 *
 * @details
 * Searches for representable timer parameters, updates synchronized shared
 * period state, derives effective frequency, and writes timer registers.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm
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

#include "xHal_Rpi5CarPwm.h"

#include "xHal_Rpi5CarMath.h"
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
 * @brief Calculates and configures timer settings for a requested frequency.
 *
 * @details
 * Searches ten prescaler candidates near the square root of the ideal timer
 * divisor and selects the representable period with the smallest error.
 *
 * @param[in] frequencyHz
 * Finite requested output frequency greater than zero, in Hertz.
 * Fractional Hertz are truncated before the timer search.
 *
 * @post
 * The selected prescaler and period are stored and written to the shared
 * timer registers.
 *
 * @throws std::invalid_argument
 * If the frequency is non-finite, not greater than zero, or truncates to zero.
 *
 * @throws std::out_of_range
 * If the frequency exceeds the unsigned 32-bit range or no valid 16-bit timer
 * period is found in the search window.
 */
void XWalkPwm::setFrequency(float64 frequencyHz) {
  const hal::boolean frequencyHzInvalid = static_cast<hal::boolean>(
      !XHAL_IS_FINITE(frequencyHz) || frequencyHz <= 0.0);
  if (frequencyHzInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL, "frequency must be greater than zero");
  }

  const float64 maximumFrequencyHz =
      static_cast<float64>(XHAL_RPI5CAR_UINT32_MAX);
  if (frequencyHz > maximumFrequencyHz) {
    XWALK_HAL_ERROR(XWALK_RANGE, "frequency exceeds the uint32 range");
  }

  const uint32 requestedFrequencyHzInteger = static_cast<uint32>(frequencyHz);
  if (requestedFrequencyHzInteger == 0U) {
    XWALK_HAL_ERROR(XWALK_INVAL, "frequency is too small");
  }

  const float64 pwmClockHz = static_cast<float64>(XHAL_RPI5CAR_PWM_CLOCK_HZ);
  const float64 requestedFrequencyHz =
      static_cast<float64>(requestedFrequencyHzInteger);
  const float64 idealTimerDivisor = pwmClockHz / requestedFrequencyHz;
  const float64 idealPrescaler = XHAL_SQUARE_ROOT(idealTimerDivisor);
  int32 candidateStart =
      static_cast<int32>(idealPrescaler) - XHAL_RPI5CAR_PWM_SEARCH_RADIUS;
  candidateStart = XHAL_MAXIMUM_VALUE(XHAL_RPI5CAR_PWM_MIN_PRESCALER_CANDIDATE,
                                      candidateStart);

  uint32 bestPrescaler = 0U;
  uint32 bestPeriod = 0U;
  float64 bestErrorHz = XHAL_POSITIVE_INFINITY(float64);

  const int32 candidateEnd =
      candidateStart + XHAL_RPI5CAR_PWM_SEARCH_CANDIDATE_COUNT;
  for (int32 candidate = candidateStart; candidate < candidateEnd;
       ++candidate) {
    const uint32 candidatePrescaler = static_cast<uint32>(candidate);
    const float64 pwmPrescaler = static_cast<float64>(candidatePrescaler);
    const float64 timerDivisor = requestedFrequencyHz * pwmPrescaler;
    const float64 calculatedTimerPeriod = pwmClockHz / timerDivisor;
    const uint32 timerPeriod = static_cast<uint32>(calculatedTimerPeriod);

    if (timerPeriod == 0U || timerPeriod > XHAL_RPI5CAR_UINT16_MAX) {
      continue;
    }

    const float64 pwmTimerPeriod = static_cast<float64>(timerPeriod);
    const float64 pwmDivisor = pwmPrescaler * pwmTimerPeriod;
    const float64 actualFrequencyHz = pwmClockHz / pwmDivisor;
    const float64 frequencyErrorHz =
        XHAL_ABSOLUTE_VALUE(requestedFrequencyHz - actualFrequencyHz);

    if (frequencyErrorHz < bestErrorHz) {
      bestErrorHz = frequencyErrorHz;
      bestPrescaler = candidatePrescaler;
      bestPeriod = timerPeriod;
    }
  }

  if (bestPrescaler == 0U) {
    XWALK_HAL_ERROR(XWALK_RANGE, "frequency cannot be represented");
  }

  frequencyHzValue = requestedFrequencyHz;
  setPrescaler(static_cast<float64>(bestPrescaler));
  setPeriod(static_cast<float64>(bestPeriod));
  XWALK_HAL_TRACE_UID1(RPI .160, "PWM frequency configured to %.3f Hz",
                       frequencyHzValue);
}

/**
 * @brief Configures the shared timer prescaler.
 *
 * @param[in] prescaler
 * Requested division factor; rounded valid range is 1 to 65536.
 *
 * @post
 * The stored prescaler, derived frequency in Hertz, and hardware prescaler
 * register reflect the rounded value.
 *
 * @throws std::invalid_argument
 * If `prescaler` is non-finite.
 *
 * @throws std::out_of_range
 * If the rounded value is outside 1 to 65536.
 */
void XWalkPwm::setPrescaler(float64 prescaler) {
  const uint32 roundedPrescaler =
      common::roundedValue(prescaler, "prescaler", 1U, 65536U);
  const uint32 currentPeriod = timerState->getPeriod(timerIndexValue);
  prescalerValue = roundedPrescaler;

  const float64 pwmClockHz = static_cast<float64>(XHAL_RPI5CAR_PWM_CLOCK_HZ);
  const float64 pwmPrescaler = static_cast<float64>(prescalerValue);
  const float64 pwmPeriod = static_cast<float64>(currentPeriod);
  const float64 pwmDivisor = pwmPrescaler * pwmPeriod;
  frequencyHzValue = pwmClockHz / pwmDivisor;

  uint32 prescalerRegisterValue{};
  if (timerIndexValue < XHAL_RPI5CAR_PWM_TIMER_FOUR) {
    prescalerRegisterValue = XHAL_RPI5CAR_PWM_PRESCALER_REG + timerIndexValue;
  } else {
    const uint32 secondaryTimerIndex =
        timerIndexValue - XHAL_RPI5CAR_PWM_TIMER_FOUR;
    prescalerRegisterValue =
        XHAL_RPI5CAR_PWM_PRESCALER_REG_2 + secondaryTimerIndex;
  }
  const uint8 prescalerRegister = static_cast<uint8>(prescalerRegisterValue);
  write16(prescalerRegister, prescalerValue - 1U);
  XWALK_HAL_TRACE_UID2(RPI .161, "PWM timer %u prescaler configured to %u",
                       timerIndexValue, prescalerValue);
}

/**
 * @brief Reads the synchronized period for this channel's shared timer.
 *
 * @return
 * Current period in timer-count units.
 *
 * @pre
 * `timerState` points to a live timer-state object.
 */
uint32 XWalkPwm::period() const {
  return timerState->getPeriod(timerIndexValue);
}

/**
 * @brief Configures the shared timer period.
 *
 * @param[in] periodValue
 * Requested period in timer-count units; rounded valid range is 1 to 65535.
 *
 * @post
 * The shared period, derived frequency in Hertz, and hardware period register
 * reflect the rounded value.
 *
 * @throws std::invalid_argument
 * If `periodValue` is non-finite.
 *
 * @throws std::out_of_range
 * If the rounded value is outside 1 to 65535.
 */
void XWalkPwm::setPeriod(float64 periodValue) {
  const uint32 roundedPeriod =
      common::roundedValue(periodValue, "period", 1U, XHAL_RPI5CAR_UINT16_MAX);
  timerState->updatePeriod(timerIndexValue, roundedPeriod);
  const float64 pwmClockHz = static_cast<float64>(XHAL_RPI5CAR_PWM_CLOCK_HZ);
  const float64 pwmPrescaler = static_cast<float64>(prescalerValue);
  const float64 pwmPeriod = static_cast<float64>(roundedPeriod);
  const float64 pwmDivisor = pwmPrescaler * pwmPeriod;
  frequencyHzValue = pwmClockHz / pwmDivisor;

  uint32 periodRegisterValue{};
  if (timerIndexValue < XHAL_RPI5CAR_PWM_TIMER_FOUR) {
    periodRegisterValue = XHAL_RPI5CAR_PWM_PERIOD_REG + timerIndexValue;
  } else {
    const uint32 secondaryTimerIndex =
        timerIndexValue - XHAL_RPI5CAR_PWM_TIMER_FOUR;
    periodRegisterValue = XHAL_RPI5CAR_PWM_PERIOD_REG_2 + secondaryTimerIndex;
  }
  const uint8 periodRegister = static_cast<uint8>(periodRegisterValue);

  write16(periodRegister, roundedPeriod);
  XWALK_HAL_TRACE_UID2(RPI .162, "PWM timer %u period configured to %u",
                       timerIndexValue, roundedPeriod);
}

} /* namespace xwalk::hal */
