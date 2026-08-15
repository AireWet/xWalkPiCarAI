/******************************************************************************
 * @file        xHal_Rpi5CarServoTestTrace.cpp
 * @brief       Verifies persistent xWalkServo trace selector behavior.
 * @project     xWalk Firmware
 * @module      xWalkServo Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarServoTestFunctions.h"
#include "xHal_Rpi5CarServoSimulationArguments.h"
#include <cassert>
namespace xwalk::hal::test
{
    void testServoTraceSelection()
    {
        char executable[] = "xWalkServoTest";
        char option[] = "--trace";
        char enableSelector[] = "RPI.187.enable";
        char disableSelector[] = "RPI.187.disable";
        char malformedSelector[] = "RPI.invalid.enable";
        charpointer enableArguments[]{executable, option, enableSelector};
        sim::XWalkServoSimulationArguments enable(3, enableArguments);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());
        charpointer disableArguments[]{executable, option, disableSelector};
        sim::XWalkServoSimulationArguments disable(3, disableArguments);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());
        charpointer malformedArguments[]{executable, option, malformedSelector};
        const sim::XWalkServoSimulationArguments malformed(3, malformedArguments);
        assert(malformed.valid() == false);
    }
} /* namespace xwalk::hal::test */
