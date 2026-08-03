/******************************************************************************
 * @file        xHal_Rpi5CarButtonEventSequenceLinux.cpp
 * @brief       Implements Linux callbacks for the D0 button-event sequence.
 *
 * @details
 * Provides wall-clock timestamps, bounded sleeping, and console output for
 * physical Robot HAT execution.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Hardware
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

#include "xHal_Rpi5CarButtonEventSequenceLinux.h"

#include "xHal_Rpi5CarButtonEventSequence.h"
#include "xHal_Rpi5CarCommon.h"

#include <chrono>
#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

void XWalkButtonEventSequenceLinux::run(XWalkGpio& gpio, uint32 durationSeconds)
{
    XWalkButtonEventSequence buttonEventSequence(
        gpio, this, &XWalkButtonEventSequenceLinux::wait,
        &XWalkButtonEventSequenceLinux::time,
        &XWalkButtonEventSequenceLinux::event);
    announce(durationSeconds);
    buttonEventSequence.run(durationSeconds);
}

void XWalkButtonEventSequenceLinux::announce(uint32 durationSeconds) const
{
    std::cout << "Monitoring D0 (GPIO17) for " << durationSeconds
              << " seconds; press and release the connected button." << std::endl;
}

void XWalkButtonEventSequenceLinux::wait(
    contextpointer context, uint32 durationMilliseconds)
{
    static_cast<void>(context);
    common::sleepMilliseconds(durationMilliseconds);
}

float64 XWalkButtonEventSequenceLinux::time(contextpointer context)
{
    static_cast<void>(context);
    const auto elapsed = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::duration<float64>>(elapsed).count();
}

void XWalkButtonEventSequenceLinux::event(
    contextpointer context, boolean pressed, float64 timestampSeconds)
{
    static_cast<void>(context);
    std::cout << (pressed ? "Pressed - " : "Released - ")
              << timestampSeconds << std::endl;
}

} /* namespace xwalk::hal::test */
