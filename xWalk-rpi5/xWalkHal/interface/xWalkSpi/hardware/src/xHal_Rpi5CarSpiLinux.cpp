/******************************************************************************
 * @file        xHal_Rpi5CarSpiLinux.cpp
 * @brief       Implements serialized Linux spidev transactions.
 *
 * @details
 * Builds one `SPI_IOC_MESSAGE` request whose chip select remains asserted for
 * the complete bounded full-duplex transfer and rejects incomplete results.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Linux Backend
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xHal_Rpi5CarSpiLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"
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
 * @brief Performs one chip-select-bounded full-duplex transfer.
 * @param[in] transmitData Non-empty payload containing at most 256 bytes.
 * @return Received bytes with exactly the transmitted length.
 * @throws std::invalid_argument If the payload is empty.
 * @throws std::out_of_range If the payload exceeds 256 bytes.
 * @throws std::runtime_error If the Linux transaction fails or is incomplete.
 */
bytevector XWalkSpiLinux::transfer(const bytevector &transmitData) {
  const hal::boolean transmitDataEmpty =
      static_cast<hal::boolean>(transmitData.empty());
  if (transmitDataEmpty) {
    XWALK_HAL_ERROR(XWALK_INVAL, "SPI transfer payload must not be empty");
  }
  const hal::boolean transmitDataTooLarge = static_cast<hal::boolean>(
      transmitData.size() > XHAL_RPI5CAR_SPI_MAXIMUM_TRANSFER_BYTES);
  if (transmitDataTooLarge) {
    XWALK_HAL_ERROR(XWALK_RANGE, "SPI transfer payload exceeds 256 bytes");
  }

  XWALK_HAL_TRACE_UID1(RPI .048, "Linux SPI transfer started for %zu bytes",
                       transmitData.size());

  bytevector receivedData(transmitData.size(), 0U);
  spi_ioc_transfer request{};
  request.tx_buf = reinterpret_cast<uint64>(transmitData.data());
  request.rx_buf = reinterpret_cast<uint64>(receivedData.data());
  request.len = static_cast<uint32>(transmitData.size());
  request.speed_hz = speedHzValue;
  request.bits_per_word = bitsPerWordValue;
  request.cs_change = 0U;

  const mutexlock lock(mutex);
  const int32 transferredBytes =
      deviceInterfaceValue.transfer(fileDescriptor, &request);
  if (transferredBytes < 0) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Linux SPI transfer failed");
  }
  const hal::boolean transferredBytesTransmitDataDifferent =
      static_cast<hal::boolean>(static_cast<size>(transferredBytes) !=
                                transmitData.size());
  if (transferredBytesTransmitDataDifferent) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Linux SPI transfer was incomplete");
  }
  XWALK_HAL_TRACE_UID1(RPI .049, "Linux SPI transfer completed for %zu bytes",
                       receivedData.size());
  return receivedData;
}

} /* namespace xwalk::hal */
