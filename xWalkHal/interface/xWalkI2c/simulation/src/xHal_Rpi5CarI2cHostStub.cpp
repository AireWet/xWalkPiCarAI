/******************************************************************************
 * @file        xHal_Rpi5CarI2cHostStub.cpp
 * @brief       Implements the Linux I2C device-interface host mirror.
 *
 * @details
 * Mirrors device-node, address-selection, and SMBus transfer requests so the
 * complete Linux I2C backend runs on a host without physical I2C hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-09
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

#include "xHal_Rpi5CarI2cHostStub.h"

#include "xHal_Rpi5CarLinuxHeaders.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim {

/******************************************************************************
 * Constructor and destructor definitions
 ******************************************************************************/

XWalkI2cHostStub::XWalkI2cHostStub() = default;
XWalkI2cHostStub::~XWalkI2cHostStub() = default;

/******************************************************************************
 * Linux device-interface definitions
 ******************************************************************************/

/**
 * @brief Returns a deterministic logical descriptor without opening hardware.
 * @param[in] devicePath Non-null logical path supplied by the Linux backend.
 * @return Descriptor value `101`, or `-1` when the path is null.
 */
int32 XWalkI2cHostStub::openDevice(cstring devicePath) {
  return devicePath == nullptr ? -1 : 101;
}

/**
 * @brief Records one Linux I2C slave-address selection.
 * @param[in] fileDescriptor Logical descriptor returned by `openDevice`.
 * @param[in] address Seven-bit address selected by the Linux backend.
 * @return `true` when the logical descriptor is valid.
 */
boolean XWalkI2cHostStub::selectAddress(int32 fileDescriptor, uint8 address) {
  lastAddressValue = address;
  sequentialReadIndex = 0U;
  const boolean descriptorValid = fileDescriptor == 101;
  XWALK_HAL_TRACE_UID2(RPI .037, "Host I2C selected address %u with status %u",
                       static_cast<uint32>(address),
                       static_cast<uint32>(descriptorValid));
  return descriptorValid;
}

/**
 * @brief Mirrors one SMBus ioctl request from the Linux backend.
 * @param[in] fileDescriptor Logical descriptor returned by `openDevice`.
 * @param[in,out] request Opaque pointer to `i2c_smbus_ioctl_data`.
 * @return `true` when the mirrored request is supported and succeeds.
 */
boolean XWalkI2cHostStub::transfer(int32 fileDescriptor,
                                   contextpointer request) {
  if ((fileDescriptor != 101) || (request == nullptr)) {
    return false;
  }

  auto *const smbusRequest = static_cast<i2c_smbus_ioctl_data *>(request);
  if (smbusRequest->size == I2C_SMBUS_QUICK) {
    return lastAddressValue == 0x14U;
  }

  if ((smbusRequest->read_write == I2C_SMBUS_READ) &&
      (smbusRequest->size == I2C_SMBUS_BYTE) &&
      (smbusRequest->data != nullptr)) {
    const uint8 mirroredBytes[]{0xABU, 0xCDU};
    const size mirroredByteCount =
        sizeof(mirroredBytes) / sizeof(mirroredBytes[0U]);
    smbusRequest->data->byte =
        mirroredBytes[sequentialReadIndex % mirroredByteCount];
    ++sequentialReadIndex;
    lastReadLengthValue = sequentialReadIndex;
    XWALK_HAL_TRACE_UID2(
        RPI .039, "Host I2C mirrored address %u and %zu sequential-read bytes",
        static_cast<uint32>(lastAddressValue), lastReadLengthValue);
    return true;
  }

  if ((smbusRequest->read_write == I2C_SMBUS_READ) &&
      (smbusRequest->size == I2C_SMBUS_I2C_BLOCK_DATA) &&
      (smbusRequest->data != nullptr)) {
    const size requestedLength =
        static_cast<size>(smbusRequest->data->block[0U]);
    const uint8 mirroredBytes[]{0x34U, 0x12U};
    for (size index = 0U; index < requestedLength; ++index) {
      smbusRequest->data->block[index + 1U] = mirroredBytes[index % 2U];
    }
    lastRegisterValue = smbusRequest->command;
    lastReadLengthValue = requestedLength;
    XWALK_HAL_TRACE_UID3(RPI .040,
                         "Host I2C mirrored address %u, register %u, and %zu "
                         "register-read bytes",
                         static_cast<uint32>(lastAddressValue),
                         static_cast<uint32>(lastRegisterValue),
                         requestedLength);
    return true;
  }

  if ((smbusRequest->read_write != I2C_SMBUS_WRITE) ||
      (smbusRequest->data == nullptr)) {
    return false;
  }

  lastRegisterValue = smbusRequest->command;
  lastDataValue.clear();
  if (smbusRequest->size == I2C_SMBUS_BYTE_DATA) {
    lastDataValue.push_back(smbusRequest->data->byte);
  } else if (smbusRequest->size == I2C_SMBUS_WORD_DATA) {
    lastDataValue.push_back(
        static_cast<uint8>(smbusRequest->data->word & 0xFFU));
    lastDataValue.push_back(static_cast<uint8>(smbusRequest->data->word >> 8U));
  } else if (smbusRequest->size == I2C_SMBUS_I2C_BLOCK_DATA) {
    const size payloadLength = static_cast<size>(smbusRequest->data->block[0U]);
    for (size index = 0U; index < payloadLength; ++index) {
      lastDataValue.push_back(smbusRequest->data->block[index + 1U]);
    }
  } else {
    return false;
  }

  XWALK_HAL_TRACE_UID3(
      RPI .038,
      "Host I2C mirrored address %u, register %u, and %zu write bytes",
      static_cast<uint32>(lastAddressValue),
      static_cast<uint32>(lastRegisterValue), lastDataValue.size());
  return true;
}

/**
 * @brief Accepts closure of the deterministic logical descriptor.
 * @param[in] fileDescriptor Logical descriptor being released.
 */
void XWalkI2cHostStub::closeDevice(int32 fileDescriptor) noexcept {
  static_cast<void>(fileDescriptor);
}

/******************************************************************************
 * Observation definitions
 ******************************************************************************/

uint8 XWalkI2cHostStub::lastAddress() const noexcept {
  return lastAddressValue;
}

uint8 XWalkI2cHostStub::lastRegister() const noexcept {
  return lastRegisterValue;
}

const bytevector &XWalkI2cHostStub::lastData() const noexcept {
  return lastDataValue;
}

size XWalkI2cHostStub::lastReadLength() const noexcept {
  return lastReadLengthValue;
}

} /* namespace xwalk::hal::sim */
