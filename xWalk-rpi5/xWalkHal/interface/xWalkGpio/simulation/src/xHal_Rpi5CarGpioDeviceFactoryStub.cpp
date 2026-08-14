/******************************************************************************
 * @file        xHal_Rpi5CarGpioDeviceFactoryStub.cpp
 * @brief       Creates the device-free GPIO simulation implementation.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarGpioDeviceFactory.h"
#include "xHal_Rpi5CarGpioHostStub.h"
#include "xHal_Rpi5CarTrace.h"

namespace xwalk::hal::sim {

owningpointer<XWalkGpioDevice> createGpioDevice() {
  XWALK_HAL_TRACE_UID0(RPI .078, "Creating host-mirror GPIO device");
  return std::make_unique<XWalkGpioHostStub>();
}

} /* namespace xwalk::hal::sim */
