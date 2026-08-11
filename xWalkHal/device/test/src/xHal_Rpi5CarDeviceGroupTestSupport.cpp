/******************************************************************************
 * @file        xHal_Rpi5CarDeviceGroupTestSupport.cpp
 * @brief       Implements failure-injection support for device-group tests.
 * @details     Rejects one configured register write without physical bus
 *access.
 * @project     xWalk Firmware
 * @module      xWalk Device Group Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarDeviceGroupTestSupport.h"

#include "xHal_Rpi5CarTrace.h"
/** @brief Contains deterministic failure support for device-group tests. */
namespace xwalk::hal::test::device_group {
/**
 * @brief Reports that no automatically probed address is present.
 * @param[in] context Non-owning fake state; unused.
 * @param[in] address Seven-bit probe address; unused.
 * @return Always `false` so tests select an explicit address.
 */
boolean probe(contextpointer context, uint8 address) {
  static_cast<void>(context);
  static_cast<void>(address);
  return false;
}

/**
 * @brief Counts a register write and injects the configured failure.
 * @param[in,out] context Non-null `FailingI2cBackend` state.
 * @param[in] address Seven-bit destination address; unused.
 * @param[in] reg Eight-bit register address; unused.
 * @param[in] data Register payload; unused.
 */
void writeRegister(contextpointer context, uint8 address, uint8 reg,
                   const bytevector &data) {
  static_cast<void>(address);
  static_cast<void>(reg);
  static_cast<void>(data);
  FailingI2cBackend &backend = *static_cast<FailingI2cBackend *>(context);
  ++backend.writeCount;
  if (backend.writeCount == backend.failingWrite) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Device-group I2C register write failed");
  }
}

/**
 * @brief Returns the requested number of zero bytes.
 * @param[in] context Non-owning fake state; unused.
 * @param[in] address Seven-bit source address; unused.
 * @param[in] length Number of bytes requested.
 * @return Zero-filled response of exactly `length` bytes.
 */
bytevector read(contextpointer context, uint8 address, size length) {
  static_cast<void>(context);
  static_cast<void>(address);
  return bytevector(length, 0U);
}
} /* namespace xwalk::hal::test::device_group */
