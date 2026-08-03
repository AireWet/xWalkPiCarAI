/******************************************************************************
 * @file        xHal_Rpi5CarButtonEventSequence.cpp
 * @brief       Implements the physical D0 button-event sequence test.
 *
 * @details
 * Coordinates a D0 pull-up input, combined-edge callback, injected timestamp
 * reporting, and bounded execution without platform-specific dependencies.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarButtonEventSequence.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

/******************************************************************************
 * Public constructor definitions
 ******************************************************************************/

XWalkButtonEventSequence::XWalkButtonEventSequence(XWalkGpio& gpio,
    contextpointer context, sequencewaitcallback wait, sequencetimecallback time,
    sequenceeventcallback event)
    : gpioObject(&gpio), callbackContext(context), waitCallback(wait),
      timeCallback(time), eventCallback(event)
{
    if ((waitCallback == nullptr) || (timeCallback == nullptr) ||
        (eventCallback == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Button-event sequence callbacks must not be null");
    }
}

/******************************************************************************
 * Private member function definitions
 ******************************************************************************/

void XWalkButtonEventSequence::handleEvent() noexcept
{
    const boolean pressed = !pressedValue.load();
    pressedValue.store(pressed);
    eventCallback(callbackContext, pressed, timeCallback(callbackContext));
}

void XWalkButtonEventSequence::eventHandler(contextpointer context) noexcept
{
    static_cast<XWalkButtonEventSequence*>(context)->handleEvent();
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

void XWalkButtonEventSequence::run(uint32 durationSeconds)
{
    if ((durationSeconds == 0U) || (durationSeconds > 3'600U))
    {
        XHAL_THROW_OUT_OF_RANGE("Button-event duration must be from 1 to 3600 seconds");
    }

    gpioObject->irq(this, &XWalkButtonEventSequence::eventHandler,
        XWalkGpioEdge::Both, 10U, XWalkGpioPull::Up);
    waitCallback(callbackContext, durationSeconds * 1'000U);
    gpioObject->close();
}

} /* namespace xwalk::hal::test */
