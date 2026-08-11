/******************************************************************************
 * @file        xHal_Rpi5CarSpiHostStub.cpp
 * @brief       Implements the Linux SPI device-interface host mirror.
 *
 * @details
 * Mirrors device-node, configuration, and full-duplex requests so the complete
 * Linux SPI backend runs on a host without physical SPI hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
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

#include "xHal_Rpi5CarSpiHostStub.h"

#include "xHal_Rpi5CarLinuxHeaders.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkSpi simulation support.
 */
namespace xwalk::hal::sim {

/** @brief Constructs an empty device-free SPI mirror. */
XWalkSpiHostStub::XWalkSpiHostStub() = default;

/** @brief Destroys the mirror and its owned transfer record. */
XWalkSpiHostStub::~XWalkSpiHostStub() = default;

/**
 * @brief Returns a deterministic logical descriptor without opening hardware.
 * @param[in] devicePath Non-null logical path supplied by the Linux backend.
 * @return Descriptor value `102`, or `-1` when the path is null.
 */
int32 XWalkSpiHostStub::openDevice(cstring devicePath) {
  return devicePath == nullptr ? -1 : 102;
}

/**
 * @brief Records one standard SPI mode configuration request.
 * @param[in] fileDescriptor Logical descriptor returned by `openDevice`.
 * @param[in,out] mode Requested mode copied into mirror state.
 * @return `true` when the logical descriptor is valid.
 */
boolean XWalkSpiHostStub::configureMode(int32 fileDescriptor, uint8 &mode) {
  modeValue = mode;
  return fileDescriptor == 102;
}

/**
 * @brief Records one SPI word-width configuration request.
 * @param[in] fileDescriptor Logical descriptor returned by `openDevice`.
 * @param[in,out] bitsPerWord Requested word width copied into mirror state.
 * @return `true` when the logical descriptor is valid.
 */
boolean XWalkSpiHostStub::configureBitsPerWord(int32 fileDescriptor,
                                               uint8 &bitsPerWord) {
  bitsPerWordValue = bitsPerWord;
  return fileDescriptor == 102;
}

/**
 * @brief Records one SPI clock configuration request.
 * @param[in] fileDescriptor Logical descriptor returned by `openDevice`.
 * @param[in,out] speedHz Requested clock frequency in Hertz copied into mirror
 * state.
 * @return `true` when the logical descriptor is valid.
 */
boolean XWalkSpiHostStub::configureSpeed(int32 fileDescriptor,
                                         uint32 &speedHz) {
  speedHzValue = speedHz;
  return fileDescriptor == 102;
}

/**
 * @brief Mirrors one full-duplex Linux SPI message.
 * @param[in] fileDescriptor Logical descriptor returned by `openDevice`.
 * @param[in,out] request Opaque pointer to one `spi_ioc_transfer` request.
 * @return Requested byte count on success; otherwise `-1`.
 */
int32 XWalkSpiHostStub::transfer(int32 fileDescriptor, contextpointer request) {
  const boolean requestInvalid =
      (fileDescriptor != 102) || (request == nullptr);
  if (requestInvalid) {
    return -1;
  }

  auto *const spiRequest = static_cast<spi_ioc_transfer *>(request);
  // Linux defines these ABI fields as 64-bit user addresses. The host mirror
  // reverses the production pointer-to-integer conversion without dereferencing
  // an arbitrary external integer; the buffers originate in XWalkSpiLinux.
  // NOLINTNEXTLINE(performance-no-int-to-ptr)
  auto *const transmitData =
      reinterpret_cast<const uint8 *>(spiRequest->tx_buf);
  // NOLINTNEXTLINE(performance-no-int-to-ptr)
  auto *const receivedData = reinterpret_cast<uint8 *>(spiRequest->rx_buf);
  const size transferLength = static_cast<size>(spiRequest->len);
  lastTransmitDataValue.assign(transmitData, transmitData + transferLength);
  for (size index = 0U; index < transferLength; ++index) {
    receivedData[index] = responseDataValue[index % responseDataValue.size()];
  }
  XWALK_HAL_TRACE_UID1(RPI .056, "Host SPI mirrored %zu full-duplex bytes",
                       transferLength);
  return static_cast<int32>(spiRequest->len);
}

/**
 * @brief Accepts closure of the deterministic logical descriptor.
 * @param[in] fileDescriptor Logical descriptor being released.
 */
void XWalkSpiHostStub::closeDevice(int32 fileDescriptor) noexcept {
  static_cast<void>(fileDescriptor);
}

/**
 * @brief Returns the most recently mirrored transmit payload.
 * @return Read-only reference valid until the next transfer or stub
 * destruction.
 */
const bytevector &XWalkSpiHostStub::lastTransmitData() const noexcept {
  return lastTransmitDataValue;
}

/**
 * @brief Returns the most recently configured SPI clock.
 * @return Clock frequency in Hertz.
 */
uint32 XWalkSpiHostStub::speedHz() const noexcept { return speedHzValue; }

/**
 * @brief Returns the most recently configured standard SPI mode.
 * @return Mode value in the inclusive range zero through three.
 */
uint8 XWalkSpiHostStub::mode() const noexcept { return modeValue; }

/**
 * @brief Returns the most recently configured SPI word width.
 * @return Word width in bits.
 */
uint8 XWalkSpiHostStub::bitsPerWord() const noexcept {
  return bitsPerWordValue;
}

} /* namespace xwalk::hal::sim */
