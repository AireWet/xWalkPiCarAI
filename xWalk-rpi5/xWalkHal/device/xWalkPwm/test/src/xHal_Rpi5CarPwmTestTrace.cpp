/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestTrace.cpp
 * @brief       Verifies persistent xWalkPwm trace selector behavior.
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarPwmTestFunctions.h"
#include "xHal_Rpi5CarPwmSimulationArguments.h"
#include <cassert>
namespace xwalk::hal::test
{
    void testTraceSelection()
    {
        char executable[] = "xWalkPwmTest";
        char option[] = "--trace";
        char enableSelector[] = "RPI.169.enable";
        char disableSelector[] = "RPI.169.disable";
        char malformedSelector[] = "RPI.invalid.enable";
        charpointer enableArguments[]{executable, option, enableSelector};
        sim::XWalkPwmSimulationArguments enable(3, enableArguments);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());
        charpointer disableArguments[]{executable, option, disableSelector};
        sim::XWalkPwmSimulationArguments disable(3, disableArguments);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());
        charpointer malformedArguments[]{executable, option, malformedSelector};
        const sim::XWalkPwmSimulationArguments malformed(3, malformedArguments);
        assert(malformed.valid() == false);
    }
} /* namespace xwalk::hal::test */
