/******************************************************************************
 * @file        xHal_Rpi5CarAdcLifecycle.cpp
 * @brief       Implements ADC construction, validation, and destruction.
 *
 * @details
 * Binds the non-owning I2C dependency, parses channels, maps hardware commands,
 *and selects addresses.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdc
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
#include "xHal_Rpi5CarAdc.h"
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
 * @brief Parses a channel name in the form `A0` through `A7`.
 *
 * @param[in] channel
 * Case-sensitive ADC channel name.
 *
 * @return
 * Numeric channel in the range 0 through 7.
 *
 * @throws std::invalid_argument
 * If the name is not exactly `A0` through `A7`.
 */
uint8 XWalkAdc::parseChannel(stringview channel) {
  const hal::boolean channelInvalid = static_cast<hal::boolean>(
      (channel.size() != 2U) || (channel[0U] != 'A') || (channel[1U] < '0') ||
      (channel[1U] > '7'));
  if (channelInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL, "ADC channel name must be A0 through A7");
  }
  return static_cast<uint8>(channel[1U] - '0');
}

/**
 * @brief Selects an explicit or automatically detected ADC address.
 *
 * @param[in] i2c
 * I2C interface used to probe candidate addresses.
 *
 * @param[in] address
 * Optional explicit seven-bit address.
 *
 * @return
 * Explicit address, first responding candidate, or 0x14 when neither candidate
 * responds.
 *
 * @note
 * Falling back to the first candidate when probing finds no device preserves
 * the Python implementation.
 */
uint8 XWalkAdc::selectAddress(XWalkI2c &i2c, optionaluint8 address) {
  const hal::boolean addressProvided =
      static_cast<hal::boolean>(address.has_value());
  if (addressProvided) {
    common::validateI2cAddress(address.value());
    return address.value();
  }
  const hal::boolean candidateAddressAvailable =
      static_cast<hal::boolean>(i2c.probe(XHAL_RPI5CAR_ADC_ADDRESS_1));
  if (candidateAddressAvailable) {
    return XHAL_RPI5CAR_ADC_ADDRESS_1;
  }
  const hal::boolean secondaryAddressAvailable =
      static_cast<hal::boolean>(i2c.probe(XHAL_RPI5CAR_ADC_ADDRESS_2));
  if (secondaryAddressAvailable) {
    return XHAL_RPI5CAR_ADC_ADDRESS_2;
  }
  return XHAL_RPI5CAR_ADC_ADDRESS_1;
}

/**
 * @brief Validates and converts a logical channel to its hardware command.
 *
 * @param[in] channel
 * Logical channel in the range 0 through 7.
 *
 * @return
 * Hardware ADC read command.
 *
 * @throws std::out_of_range
 * If `channel` exceeds 7.
 */
uint8 XWalkAdc::createCommand(uint32 channel) {
  if (channel > XHAL_RPI5CAR_ADC_MAX_CHANNEL) {
    XWALK_HAL_ERROR(XWALK_RANGE, "ADC channel must be between 0 and 7");
  }
  const uint32 mappedChannel = XHAL_RPI5CAR_ADC_MAX_CHANNEL - channel;
  return static_cast<uint8>(mappedChannel | XHAL_RPI5CAR_ADC_READ_COMMAND);
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs an ADC interface for a numeric channel.
 *
 * @param[in] i2c
 * I2C dependency that must outlive this object.
 *
 * @param[in] channel
 * Logical ADC channel in the range 0 through 7.
 *
 * @param[in] address
 * Optional explicit seven-bit device address; omission probes 0x14 then 0x15.
 *
 * @throws std::out_of_range
 * If `channel` exceeds 7 or an explicit address exceeds the seven-bit I2C
 * range.
 */
XWalkAdc::XWalkAdc(XWalkI2c &i2c, uint32 channel, optionaluint8 address)
    : i2cObject(&i2c), addressValue(selectAddress(i2c, address)),
      channelValue(static_cast<uint8>(channel)),
      commandValue(createCommand(channel)) {
  XWALK_HAL_TRACE_UID3(
      RPI .174,
      "ADC channel %u constructed at address 0x%02X with command 0x%02X",
      channelValue, addressValue, commandValue);
}

/**
 * @brief Constructs an ADC interface for a named channel.
 *
 * @param[in] i2c
 * I2C dependency that must outlive this object.
 *
 * @param[in] channel
 * Case-sensitive channel name in the form `A0` through `A7`.
 *
 * @param[in] address
 * Optional explicit seven-bit device address; omission probes 0x14 then 0x15.
 *
 * @throws std::invalid_argument
 * If `channel` is not exactly `A0` through `A7`.
 *
 * @throws std::out_of_range
 * If an explicit address exceeds the seven-bit I2C range.
 */
XWalkAdc::XWalkAdc(XWalkI2c &i2c, stringview channel, optionaluint8 address)
    : XWalkAdc(i2c, static_cast<uint32>(parseChannel(channel)), address) {}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the ADC interface.
 *
 * @note
 * The I2C pointer is non-owning and is not released.
 */
XWalkAdc::~XWalkAdc() = default;

} /* namespace xwalk::hal */
