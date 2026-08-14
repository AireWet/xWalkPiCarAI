/******************************************************************************
 * @file        xHal_Rpi5CarPwmTimerState.cpp
 * @brief       Implements synchronized PWM timer-period access.
 *
 * @details
 * Validates timer indices and serializes reads and updates of the seven shared
 * PWM timer periods.
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
 * @brief Reads one shared timer period under mutual exclusion.
 *
 * @param[in] timerIndex
 * Timer index in the inclusive range 0 to 6.
 *
 * @return
 * Stored period in timer-count units.
 *
 * @throws std::out_of_range
 * If `timerIndex` exceeds six.
 */
uint32 XWalkPwmTimerState::getPeriod(uint32 timerIndex) const {
  const hal::boolean timerIndexInvalid =
      static_cast<hal::boolean>(timerIndex >= periods.size());
  if (timerIndexInvalid) {
    XWALK_HAL_ERROR(XWALK_RANGE, "PWM timer index must be in range 0..6");
  }

  const mutexlock lock(mutex);
  return periods[timerIndex];
}

/**
 * @brief Updates one shared timer period under mutual exclusion.
 *
 * @param[in] timerIndex
 * Timer index in the inclusive range 0 to 6.
 *
 * @param[in] period
 * New period in timer-count units. This storage function does not validate
 * the value independently.
 *
 * @post
 * The selected timer contains `period` when the function returns.
 *
 * @throws std::out_of_range
 * If `timerIndex` exceeds six.
 */
void XWalkPwmTimerState::updatePeriod(uint32 timerIndex, uint32 period) {
  const hal::boolean timerIndexInvalid =
      static_cast<hal::boolean>(timerIndex >= periods.size());
  if (timerIndexInvalid) {
    XWALK_HAL_ERROR(XWALK_RANGE, "PWM timer index must be in range 0..6");
  }

  const mutexlock lock(mutex);
  periods[timerIndex] = period;
  XWALK_HAL_TRACE_UID2(RPI .167, "PWM shared timer %u updated to period %u",
                       timerIndex, period);
}
} /* namespace xwalk::hal */
