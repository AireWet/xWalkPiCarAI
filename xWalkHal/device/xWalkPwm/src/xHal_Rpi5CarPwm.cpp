/******************************************************************************
 * @file        xHal_Rpi5CarPwm.cpp
 * @brief       Implements PWM channel parsing, mapping, and address selection.
 *
 * @details
 * Converts textual channels, maps twenty output channels to seven shared
 * timers, provides the default timer state, and selects a Robot HAT address.
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
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Parses a textual PWM channel designation.
 *
 * @param[in] channel
 * Channel text in the form `P0` through `P19`.
 *
 * @return
 * Parsed channel index in the inclusive range 0 to 19.
 *
 * @throws std::invalid_argument
 * If the text does not begin with `P` followed by decimal digits.
 *
 * @throws std::out_of_range
 * If the parsed index exceeds 19.
 */
uint32 XWalkPwm::parseChannel(stringview channel) {
  uint32 parsedChannel = 0U;

  const hal::boolean channelInvalid =
      static_cast<hal::boolean>(channel.size() < 2U || channel.front() != 'P');
  if (channelInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL, "PWM channel must have the form P0..P19");
  }
  for (size i = 1U; i < channel.size(); ++i) {
    if (channel[i] < '0' || channel[i] > '9') {
      XWALK_HAL_ERROR(XWALK_INVAL, "PWM channel must have the form P0..P19");
    }

    const uint32 digit = static_cast<uint32>(channel[i] - '0');
    const uint32 shiftedChannel = parsedChannel * 10U;
    parsedChannel = shiftedChannel + digit;

    if (parsedChannel > XHAL_RPI5CAR_PWM_MAX_CHANNEL) {
      XWALK_HAL_ERROR(XWALK_RANGE, "PWM channel must be in range P0..P19");
    }
  }
  return parsedChannel;
}

/**
 * @brief Maps a PWM channel to its shared hardware timer.
 *
 * @param[in] channel
 * PWM channel in the inclusive range 0 to 19.
 *
 * @return
 * Timer index in the inclusive range 0 to 6.
 *
 * @pre
 * `channel` has been validated by the constructor or parser.
 */
uint32 XWalkPwm::timerForChannel(uint32 channel) {
  if (channel < XHAL_RPI5CAR_PWM_DIRECT_CHANNEL_COUNT) {
    return channel / XHAL_RPI5CAR_PWM_CHANNELS_PER_TIMER;
  }
  if (channel <= XHAL_RPI5CAR_PWM_TIMER_FOUR_MAX_CHANNEL) {
    return XHAL_RPI5CAR_PWM_TIMER_FOUR;
  }
  if (channel == XHAL_RPI5CAR_PWM_TIMER_FIVE_CHANNEL) {
    return XHAL_RPI5CAR_PWM_TIMER_FIVE;
  }
  return XHAL_RPI5CAR_PWM_TIMER_SIX;
}

/**
 * @brief Selects an explicit or automatically probed I2C address.
 *
 * @param[in] requested
 * Optional explicit seven-bit address. Any supplied byte is returned without
 * probing; omission probes addresses `0x14`, `0x15`, and `0x16` in order.
 *
 * @return
 * The explicit address, the first responding candidate, or `0x14` when no
 * candidate responds.
 *
 * @note
 * Falling back to the first candidate preserves the Python implementation's
 * behavior even when no device acknowledges a probe.
 */
uint8 XWalkPwm::selectAddress(optionaluint8 requested) {
  if (requested) {
    return *requested;
  }

  const i2caddressarray candidates{XHAL_RPI5CAR_I2C_ADDRESS_1,
                                   XHAL_RPI5CAR_I2C_ADDRESS_2,
                                   XHAL_RPI5CAR_I2C_ADDRESS_3};

  for (const uint8 candidate : candidates) {
    const hal::boolean candidateAddressAvailable =
        static_cast<hal::boolean>(i2cObject->probe(candidate));
    if (candidateAddressAvailable) {
      return candidate;
    }
  }

  /* Preserve the Python implementation's deterministic fallback when no
   * candidate acknowledges the probe.
   */
  return candidates.front();
}

} /* namespace xwalk::hal */
