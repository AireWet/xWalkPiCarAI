/******************************************************************************
 * @file        xHal_Rpi5CarGpioDeviceFactoryHardware.cpp
 * @brief       Creates the physical Linux GPIO device implementation.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Hardware Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarGpioDeviceFactory.h"
#include "xHal_Rpi5CarGpioDeviceLinux.h"
#include "xHal_Rpi5CarTrace.h"

namespace xwalk::hal::sim
{

    owningpointer<XWalkGpioDevice> createGpioDevice()
    {
        XWALK_HAL_TRACE_UID0(RPI .079, "Creating physical Linux GPIO device");
        return std::make_unique<XWalkGpioDeviceLinux>();
    }

} /* namespace xwalk::hal::sim */
