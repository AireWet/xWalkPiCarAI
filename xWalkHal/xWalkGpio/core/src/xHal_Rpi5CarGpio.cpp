/******************************************************************************
 * @file        xHal_Rpi5CarGpio.cpp
 * @brief       Implements hardware-independent GPIO operations.
 *
 * @details
 * Forwards configuration, digital I/O, and interrupt operations through validated backend callbacks.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio
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

#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Reconfigures the line direction and internal bias.
 *
 * @param[in] mode
 * Requested input or output direction.
 *
 * @param[in] pull
 * Requested internal bias configuration.
 *
 * @post
 * The stored mode and pull match the successfully configured backend state.
 */
void XWalkGpio::setup(XWalkGpioMode mode, XWalkGpioPull pull)
{
    close();
    callbacksValue.configure(contextValue, pinValue, mode, pull, physicalValue(outputValue));
    modeValue = mode;
    pullValue = pull;
}

/**
 * @brief Reads the logical GPIO level.
 *
 * @return
 * Logical level after applying the configured polarity.
 *
 * @post
 * The pin is configured as an input.
 */
boolean XWalkGpio::read()
{
    if (modeValue != XWalkGpioMode::Input)
    {
        setup(XWalkGpioMode::Input, pullValue);
    }
    return physicalValue(callbacksValue.read(contextValue, pinValue));
}

/**
 * @brief Drives the logical GPIO level.
 *
 * @param[in] value
 * Logical level to drive after polarity conversion.
 *
 * @return
 * The requested logical level.
 *
 * @post
 * The pin is configured as an output and stores `value` as its latest output state.
 */
boolean XWalkGpio::write(boolean value)
{
    if (modeValue != XWalkGpioMode::Output)
    {
        setup(XWalkGpioMode::Output, pullValue);
    }
    callbacksValue.write(contextValue, pinValue, physicalValue(value));
    outputValue = value;
    return value;
}

/**
 * @brief Drives and returns the logical active level.
 *
 * @return
 * Always `true` after a successful write.
 */
boolean XWalkGpio::on()
{
    return write(true);
}

/**
 * @brief Drives and returns the logical inactive level.
 *
 * @return
 * Always `false` after a successful write.
 */
boolean XWalkGpio::off()
{
    return write(false);
}

/**
 * @brief Drives and returns the logical high level.
 *
 * @return
 * Always `true` after a successful write.
 */
boolean XWalkGpio::high()
{
    return on();
}

/**
 * @brief Drives and returns the logical low level.
 *
 * @return
 * Always `false` after a successful write.
 */
boolean XWalkGpio::low()
{
    return off();
}

/**
 * @brief Registers a debounced GPIO edge handler.
 *
 * @param[in,out] handlerContext
 * Non-owning application context forwarded to `handler`.
 *
 * @param[in] handler
 * Non-null application handler.
 *
 * @param[in] edge
 * Signal transition that triggers the handler.
 *
 * @param[in] debounceMs
 * Minimum interval between accepted events in milliseconds.
 *
 * @param[in] pull
 * Internal bias applied while the interrupt input is claimed.
 *
 * @throws std::invalid_argument
 * If `handler` is null.
 */
void XWalkGpio::irq(contextpointer handlerContext, gpiointerrupthandler handler, XWalkGpioEdge edge,
    uint32 debounceMs, XWalkGpioPull pull)
{
    if (handler == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("GPIO interrupt handler must not be null");
    }
    close();
    setup(XWalkGpioMode::Input, pull);
    callbacksValue.interrupt(contextValue, pinValue, edge, debounceMs, handlerContext, handler);
    interruptActive = true;
}

/**
 * @brief Cancels the active interrupt handler when one is registered.
 *
 * @post
 * No interrupt registration remains associated with this object.
 */
void XWalkGpio::close()
{
    if (interruptActive)
    {
        callbacksValue.cancelInterrupt(contextValue, pinValue);
        interruptActive = false;
    }
}

/**
 * @brief Provides the Python-compatible alias for `close()`.
 *
 * @post
 * No interrupt registration remains associated with this object.
 */
void XWalkGpio::deinit()
{
    close();
}

/**
 * @brief Returns the Linux GPIO line offset.
 *
 * @return
 * Validated line offset from the Robot HAT pin dictionary.
 */
uint8 XWalkGpio::pin() const noexcept
{
    return pinValue;
}

/**
 * @brief Returns the current input or output mode.
 *
 * @return
 * Most recently configured mode.
 */
XWalkGpioMode XWalkGpio::mode() const noexcept
{
    return modeValue;
}

/**
 * @brief Returns the current internal pull configuration.
 *
 * @return
 * Most recently configured pull setting.
 */
XWalkGpioPull XWalkGpio::pull() const noexcept
{
    return pullValue;
}

/**
 * @brief Returns an owned Linux-style name for the GPIO line.
 *
 * @return
 * Name in the form `GPIO<number>`.
 */
string XWalkGpio::name() const
{
    return common::createGpioName(pinValue);
}

} /* namespace xwalk::hal */
