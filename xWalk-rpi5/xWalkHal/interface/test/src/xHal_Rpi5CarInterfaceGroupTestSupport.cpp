/******************************************************************************
 * @file        xHal_Rpi5CarInterfaceGroupTestSupport.cpp
 * @brief       Implements deterministic interface-group callback fakes.
 *
 * @details
 * Supplies bounded in-memory bus and GPIO operations without platform access.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarInterfaceGroupTestSupport.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test::interface_group
 * @brief Contains reusable callback fakes for interface-group tests.
 */
namespace xwalk::hal::test::interface_group
{

    /**
     * @brief Records and answers one I2C probe.
     * @param[in,out] context Non-null `I2cBackend` state that outlives the
     * callback.
     * @param[in] address Seven-bit address requested by the interface.
     * @return `true` only for the configured present address.
     */
    boolean probeI2c(contextpointer context, uint8 address)
    {
        I2cBackend& backend = *static_cast<I2cBackend*>(context);
        ++backend.probeCount;
        backend.probeAddress = address;
        if (backend.failProbe)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Interface-group I2C probe failed");
        }
        return address == backend.presentAddress;
    }

    /**
     * @brief Accepts an interface-group I2C register write.
     * @param[in] context Non-owning callback context; unused.
     * @param[in] address Seven-bit destination address; unused.
     * @param[in] reg Eight-bit register address; unused.
     * @param[in] data Payload bytes; unused.
     */
    void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
    }

    /**
     * @brief Records one sequential I2C read and returns configured bytes.
     * @param[in,out] context Non-null `I2cBackend` state that outlives the
     * callback.
     * @param[in] address Seven-bit source address.
     * @param[in] length Required response length in bytes.
     * @return Configured response resized to `length` bytes.
     */
    bytevector readI2c(contextpointer context, uint8 address, size length)
    {
        I2cBackend& backend = *static_cast<I2cBackend*>(context);
        ++backend.readCount;
        backend.readAddress = address;
        backend.readLength = length;
        bytevector result = backend.response;
        result.resize(length, 0U);
        return result;
    }

    /**
     * @brief Records one full-duplex SPI transaction.
     * @param[in,out] context Non-null `SpiBackend` state that outlives the
     * callback.
     * @param[in] transmitData Non-empty bounded payload in wire order.
     * @return Configured response bytes.
     */
    bytevector transferSpi(contextpointer context, const bytevector& transmitData)
    {
        SpiBackend& backend = *static_cast<SpiBackend*>(context);
        ++backend.transferCount;
        backend.transmitted = transmitData;
        if (backend.failTransfer)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Interface-group SPI transfer failed");
        }
        return backend.response;
    }

    /**
     * @brief Records one GPIO configuration request.
     * @param[in,out] context Non-null `GpioBackend` state that outlives the
     * callback.
     * @param[in] pin Linux line offset selected by the public GPIO mapping.
     * @param[in] mode Requested GPIO direction.
     * @param[in] pull Requested pull configuration.
     * @param[in] initialValue Initial logical output value.
     */
    void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        GpioBackend& backend = *static_cast<GpioBackend*>(context);
        ++backend.configureCount;
        backend.pin = pin;
        backend.mode = mode;
        backend.pull = pull;
        backend.value = initialValue;
    }

    /**
     * @brief Returns the fake GPIO logical value.
     * @param[in] context Non-null `GpioBackend` state that outlives the callback.
     * @param[in] pin Linux line offset; unused after interface validation.
     * @return Most recently recorded logical value.
     */
    boolean readGpio(contextpointer context, uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<GpioBackend*>(context)->value;
    }

    /**
     * @brief Records one GPIO logical output operation.
     * @param[in,out] context Non-null `GpioBackend` state that outlives the
     * callback.
     * @param[in] pin Linux line offset; unused after interface validation.
     * @param[in] value Requested logical output state.
     */
    void writeGpio(contextpointer context, uint8 pin, boolean value)
    {
        static_cast<void>(pin);
        GpioBackend& backend = *static_cast<GpioBackend*>(context);
        ++backend.writeCount;
        backend.value = value;
    }

    /**
     * @brief Accepts an unused interrupt registration for callback completeness.
     * @param[in] context Non-owning callback context; unused.
     * @param[in] pin Linux line offset; unused.
     * @param[in] edge Requested interrupt edge; unused.
     * @param[in] debounceMs Requested debounce interval in milliseconds; unused.
     * @param[in] handlerContext Non-owning application context; unused.
     * @param[in] handler Application interrupt handler; unused.
     */
    void registerInterrupt(contextpointer context,
                           uint8 pin,
                           XWalkGpioEdge edge,
                           uint32 debounceMs,
                           contextpointer handlerContext,
                           gpiointerrupthandler handler)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(edge);
        static_cast<void>(debounceMs);
        static_cast<void>(handlerContext);
        static_cast<void>(handler);
    }

    /**
     * @brief Accepts an unused interrupt cancellation for callback completeness.
     * @param[in] context Non-owning callback context; unused.
     * @param[in] pin Linux line offset; unused.
     */
    void cancelInterrupt(contextpointer context, uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }

    /**
     * @brief Creates the complete interface-group GPIO callback table.
     * @return Callback table bound to the deterministic fake operations.
     */
    XWalkGpioCallbacks gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
    }

} /* namespace xwalk::hal::test::interface_group */
