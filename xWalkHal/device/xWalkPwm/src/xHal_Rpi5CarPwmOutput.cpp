/******************************************************************************
 * @file        xHal_Rpi5CarPwmOutput.cpp
 * @brief       Implements PWM pulse-width and register-output operations.
 *
 * @details
 * Validates absolute and percentage pulse widths and encodes 16-bit Robot HAT
 * register values as high-byte-first I2C payloads.
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
 * @brief Configures the channel pulse width.
 *
 * @param[in] pulseWidth
 * Pulse width in timer-count units; valid range is 0.0 to 65535.0.
 * Fractional values are truncated during conversion to `uint32`.
 *
 * @post
 * The stored pulse width and channel output register contain the converted
 * 16-bit timer-count value.
 *
 * @throws std::invalid_argument
 * If `pulseWidth` is non-finite.
 *
 * @throws std::out_of_range
 * If `pulseWidth` is negative or greater than 65535.0.
 */
void XWalkPwm::setPulseWidth(float64 pulseWidth) {
  const float64 maximumPulseWidth =
      static_cast<float64>(XHAL_RPI5CAR_UINT16_MAX);
  const hal::boolean pulseWidthNotFinite =
      static_cast<hal::boolean>(!XHAL_IS_FINITE(pulseWidth));
  if (pulseWidthNotFinite) {
    XWALK_HAL_ERROR(XWALK_INVAL, "pulse width must be finite");
  }
  if (pulseWidth < 0.0 || pulseWidth > maximumPulseWidth) {
    XWALK_HAL_ERROR(XWALK_RANGE, "pulse width must be in range 0..65535");
  }

  pulseWidthValue = static_cast<uint32>(pulseWidth);

  const uint8 channelRegister =
      static_cast<uint8>(XHAL_RPI5CAR_PWM_CHANNEL_REG + channelValue);
  write16(channelRegister, pulseWidthValue);
  XWALK_HAL_TRACE_UID2(RPI .163, "PWM channel %u pulse width configured to %u",
                       channelValue, pulseWidthValue);
}

/**
 * @brief Configures pulse width as a percentage of the shared timer period.
 *
 * @param[in] percent
 * Finite duty cycle in the inclusive range 0.0 to 100.0 percent.
 *
 * @post
 * The requested percentage is stored and the corresponding truncated pulse
 * width is written to the channel register.
 *
 * @throws std::invalid_argument
 * If `percent` is non-finite.
 *
 * @throws std::out_of_range
 * If `percent` is outside 0.0 to 100.0 percent.
 */
void XWalkPwm::setPulseWidthPercent(float64 percent) {
  const hal::boolean percentNotFinite =
      static_cast<hal::boolean>(!XHAL_IS_FINITE(percent));
  if (percentNotFinite) {
    XWALK_HAL_ERROR(XWALK_INVAL, "pulse width percent must be finite");
  }
  if (percent < 0.0 || percent > 100.0) {
    XWALK_HAL_ERROR(XWALK_RANGE, "pulse width percent must be in range 0..100");
  }

  const float64 percentageScale = 100.0;
  const float64 normalizedDutyCycle = percent / percentageScale;
  const float64 pwmPeriod = static_cast<float64>(period());
  const float64 calculatedPulseWidth = normalizedDutyCycle * pwmPeriod;

  pulseWidthPercentValue = percent;
  setPulseWidth(calculatedPulseWidth);
  XWALK_HAL_TRACE_UID2(RPI .164,
                       "PWM channel %u duty cycle configured to %.3f percent",
                       channelValue, pulseWidthPercentValue);
}

/**
 * @brief Attempts to set duty cycle through the non-throwing I2C path.
 * @return `true` when the validated output write succeeds; otherwise `false`.
 */
boolean XWalkPwm::trySetPulseWidthPercent(float64 percent) noexcept {
  const hal::boolean percentInvalid = static_cast<hal::boolean>(
      !XHAL_IS_FINITE(percent) || (percent < 0.0) || (percent > 100.0));
  if (percentInvalid) {
    return false;
  }
  const float64 normalizedDutyCycle = percent / 100.0;
  const float64 calculatedPulseWidth =
      normalizedDutyCycle * static_cast<float64>(period());
  const uint32 pulseWidth = static_cast<uint32>(calculatedPulseWidth);
  const uint8 channelRegister =
      static_cast<uint8>(XHAL_RPI5CAR_PWM_CHANNEL_REG + channelValue);
  const bytevector data{static_cast<uint8>((pulseWidth >> 8U) & 0xFFU),
                        static_cast<uint8>(pulseWidth & 0xFFU)};
  const hal::boolean registerWritten =
      i2cObject->tryWriteRegister(addressValue, channelRegister, data);
  if (registerWritten == false) {
    return false;
  }
  pulseWidthPercentValue = percent;
  pulseWidthValue = pulseWidth;
  return true;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Writes an unsigned 16-bit value to a Robot HAT register.
 *
 * @param[in] reg
 * Eight-bit destination register address.
 *
 * @param[in] value
 * Register value in the inclusive range 0 to 65535.
 *
 * @post
 * The I2C interface receives a two-byte high-byte-first payload.
 *
 * @throws std::out_of_range
 * If `value` exceeds 16 bits.
 */
void XWalkPwm::write16(uint8 reg, uint32 value) {
  if (value > XHAL_RPI5CAR_UINT16_MAX) {
    XWALK_HAL_ERROR(XWALK_RANGE, "PWM value exceeds 16 bits");
  }

  const bytevector data{static_cast<uint8>((value >> 8U) & 0xFFU),
                        static_cast<uint8>(value & 0xFFU)};

  i2cObject->writeRegister(addressValue, reg, data);
  XWALK_HAL_TRACE_UID2(RPI .165,
                       "PWM register 0x%02X written at address 0x%02X", reg,
                       addressValue);
}

} /* namespace xwalk::hal */
