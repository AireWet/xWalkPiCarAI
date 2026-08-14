/******************************************************************************
 * @file        xHal_Rpi5CarDeviceGroupTestSupport.h
 * @brief       Declares failure-injection support for device-group tests.
 * @details     Supplies an I2C backend that rejects register writes deterministically.
 * @project     xWalk Firmware
 * @module      xWalk Device Group Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_DEVICE_GROUP_TEST_SUPPORT_H
#define XHAL_RPI5CAR_DEVICE_GROUP_TEST_SUPPORT_H

#include "xHal_Rpi5CarI2c.h"

/** @brief Contains deterministic failure support for device-group tests. */
namespace xwalk::hal::test::device_group
{
/** @brief Counts register writes before injecting a configured bus failure. */
struct FailingI2cBackend
{
    /** @brief Number of register writes observed by the fake bus. */
    uint32 writeCount{};
    /** @brief Write index that fails; zero keeps all writes successful. */
    uint32 failingWrite{};
};

/**
 * @brief Reports that no automatically probed address is present.
 * @param[in] context Non-owning fake state; unused.
 * @param[in] address Seven-bit probe address; unused.
 * @return Always `false` so tests select an explicit address.
 */
boolean probe(contextpointer context, uint8 address);

/**
 * @brief Counts a register write and injects the configured failure.
 * @param[in,out] context Non-null `FailingI2cBackend` state.
 * @param[in] address Seven-bit destination address; unused.
 * @param[in] reg Eight-bit register address; unused.
 * @param[in] data Register payload; unused.
 */
void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);

/**
 * @brief Returns the requested number of zero bytes.
 * @param[in] context Non-owning fake state; unused.
 * @param[in] address Seven-bit source address; unused.
 * @param[in] length Number of bytes requested.
 * @return Zero-filled response of exactly `length` bytes.
 */
bytevector read(contextpointer context, uint8 address, size length);
} /* namespace xwalk::hal::test::device_group */

#endif /* XHAL_RPI5CAR_DEVICE_GROUP_TEST_SUPPORT_H */
