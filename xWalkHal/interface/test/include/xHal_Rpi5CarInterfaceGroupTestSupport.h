/******************************************************************************
 * @file        xHal_Rpi5CarInterfaceGroupTestSupport.h
 * @brief       Declares deterministic interface-group callback fakes.
 *
 * @details
 * Records I2C, SPI, and GPIO requests so group scenarios can verify
 * configuration-driven data flow without opening platform devices.
 *
 * @project     xWalk Firmware
 * @module      xWalk Interface Group Test
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

#ifndef XHAL_RPI5CAR_INTERFACE_GROUP_TEST_SUPPORT_H
#define XHAL_RPI5CAR_INTERFACE_GROUP_TEST_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarSpi.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test::interface_group
 * @brief Contains reusable callback fakes for interface-group tests.
 */
namespace xwalk::hal::test::interface_group
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records I2C traffic and supplies deterministic bus responses. */
struct I2cBackend
{
    /** @brief Seven-bit address reported as present by the fake bus. */
    uint8 presentAddress{0x14U};
    /** @brief Most recently probed seven-bit address. */
    uint8 probeAddress{};
    /** @brief Most recently read seven-bit address. */
    uint8 readAddress{};
    /** @brief Number of bytes requested by the most recent read. */
    size readLength{};
    /** @brief Bytes returned by a sequential read. */
    bytevector response{0x12U, 0x34U};
    /** @brief Number of probe operations observed. */
    uint32 probeCount{};
    /** @brief Number of read operations observed. */
    uint32 readCount{};
    /** @brief Enables deterministic probe failure injection. */
    boolean failProbe{};
};

/** @brief Records one full-duplex SPI transfer and its configured response. */
struct SpiBackend
{
    /** @brief Most recently transmitted bytes in wire order. */
    bytevector transmitted{};
    /** @brief Bytes returned to the SPI interface. */
    bytevector response{0xA5U, 0x5AU};
    /** @brief Number of transfer operations observed. */
    uint32 transferCount{};
    /** @brief Enables deterministic transfer failure injection. */
    boolean failTransfer{};
};

/** @brief Records GPIO configuration and logical output operations. */
struct GpioBackend
{
    /** @brief Most recently configured Linux line offset. */
    uint8 pin{};
    /** @brief Most recently configured GPIO direction. */
    XWalkGpioMode mode{XWalkGpioMode::Output};
    /** @brief Most recently configured pull mode. */
    XWalkGpioPull pull{XWalkGpioPull::None};
    /** @brief Most recently observed logical output value. */
    boolean value{};
    /** @brief Number of configuration operations observed. */
    uint32 configureCount{};
    /** @brief Number of write operations observed. */
    uint32 writeCount{};
};

/******************************************************************************
 * Function declarations
 ******************************************************************************/

/**
 * @brief Records and answers one I2C probe.
 * @param[in,out] context Non-null `I2cBackend` state that outlives the callback.
 * @param[in] address Seven-bit address requested by the interface.
 * @return `true` only for the configured present address.
 */
boolean probeI2c(contextpointer context, uint8 address);

/**
 * @brief Accepts an interface-group I2C register write.
 * @param[in] context Non-owning callback context; unused.
 * @param[in] address Seven-bit destination address; unused.
 * @param[in] reg Eight-bit register address; unused.
 * @param[in] data Payload bytes; unused.
 */
void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data);

/**
 * @brief Records one sequential I2C read and returns configured bytes.
 * @param[in,out] context Non-null `I2cBackend` state that outlives the callback.
 * @param[in] address Seven-bit source address.
 * @param[in] length Required response length in bytes.
 * @return Configured response resized to `length` bytes.
 */
bytevector readI2c(contextpointer context, uint8 address, size length);

/**
 * @brief Records one full-duplex SPI transaction.
 * @param[in,out] context Non-null `SpiBackend` state that outlives the callback.
 * @param[in] transmitData Non-empty bounded payload in wire order.
 * @return Configured response bytes.
 */
bytevector transferSpi(contextpointer context, const bytevector& transmitData);

/**
 * @brief Records one GPIO configuration request.
 * @param[in,out] context Non-null `GpioBackend` state that outlives the callback.
 * @param[in] pin Linux line offset selected by the public GPIO mapping.
 * @param[in] mode Requested GPIO direction.
 * @param[in] pull Requested pull configuration.
 * @param[in] initialValue Initial logical output value.
 */
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue);

/**
 * @brief Returns the fake GPIO logical value.
 * @param[in] context Non-null `GpioBackend` state that outlives the callback.
 * @param[in] pin Linux line offset; unused after interface validation.
 * @return Most recently recorded logical value.
 */
boolean readGpio(contextpointer context, uint8 pin);

/**
 * @brief Records one GPIO logical output operation.
 * @param[in,out] context Non-null `GpioBackend` state that outlives the callback.
 * @param[in] pin Linux line offset; unused after interface validation.
 * @param[in] value Requested logical output state.
 */
void writeGpio(contextpointer context, uint8 pin, boolean value);

/**
 * @brief Accepts an unused interrupt registration for callback completeness.
 * @param[in] context Non-owning callback context; unused.
 * @param[in] pin Linux line offset; unused.
 * @param[in] edge Requested interrupt edge; unused.
 * @param[in] debounceMs Requested debounce interval in milliseconds; unused.
 * @param[in] handlerContext Non-owning application context; unused.
 * @param[in] handler Application interrupt handler; unused.
 */
void registerInterrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler);

/**
 * @brief Accepts an unused interrupt cancellation for callback completeness.
 * @param[in] context Non-owning callback context; unused.
 * @param[in] pin Linux line offset; unused.
 */
void cancelInterrupt(contextpointer context, uint8 pin);

/**
 * @brief Creates the complete interface-group GPIO callback table.
 * @return Callback table bound to the deterministic fake operations.
 */
XWalkGpioCallbacks gpioCallbacks();

} /* namespace xwalk::hal::test::interface_group */

#endif /* XHAL_RPI5CAR_INTERFACE_GROUP_TEST_SUPPORT_H */
