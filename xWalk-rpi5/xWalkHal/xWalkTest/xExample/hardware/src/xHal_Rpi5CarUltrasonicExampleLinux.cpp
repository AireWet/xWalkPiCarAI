/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicExampleLinux.cpp
 * @brief       Implements Linux GPIO composition for the ultrasonic example.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
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

#include "xHal_Rpi5CarUltrasonicExampleLinux.h"

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarGpioLinux.h"

#include "xHal_Rpi5CarTrace.h"
#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains Linux adapters for ported example programs.
 */
namespace xwalk::hal::example
{

    /**
     * @brief Runs bounded physical D2-trigger and D3-echo ranging.
     * @param[in] sampleCount Sample count from one through 18,000.
     * @param[in] gpioDevice Linux GPIO character-device path.
     * @param[in] chipName Optional exact kernel chip name.
     * @param[in] chipLabel Optional exact kernel chip label.
     * @warning Pulses Robot HAT GPIO27 and reads physical GPIO22.
     */
    void
    XWalkUltrasonicExampleLinux::run(uint32 sampleCount, cstring gpioDevice, stringview chipName, stringview chipLabel)
    {
        XWalkGpioLinux triggerBackend(gpioDevice, chipName, chipLabel, 27U);
        XWalkGpioLinux echoBackend(gpioDevice, chipName, chipLabel, 22U);
        const XWalkGpioCallbacks triggerCallbacks = XHAL_GPIO_CALLBACKS(XWalkGpioLinux);
        const XWalkGpioCallbacks echoCallbacks = XHAL_GPIO_CALLBACKS(XWalkGpioLinux);
        XWalkGpio trigger(&triggerBackend, triggerCallbacks, "D2");
        XWalkGpio echo(&echoBackend, echoCallbacks, "D3");
        XWalkUltrasonic sensor(trigger, echo);
        sensorObject = &sensor;
        const XWalkUltrasonicExampleCallbacks exampleCallbacks{&read, &wait, &report};
        XWalkUltrasonicExample example(this, exampleCallbacks);
        try
        {
            example.run(sampleCount);
            sensorObject = nullptr;
        }
        catch (...)
        {
            sensorObject = nullptr;
            throw;
        }
    }

    /**
     * @brief Resolves one callback context with a bound ultrasonic sensor.
     * @param[in,out] context Non-null Linux adapter context.
     * @return Referenced adapter.
     * @throws std::invalid_argument If the adapter or sensor binding is invalid.
     */
    XWalkUltrasonicExampleLinux& XWalkUltrasonicExampleLinux::adapter(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Ultrasonic Linux context must not be null");
        }
        XWalkUltrasonicExampleLinux& self = *static_cast<XWalkUltrasonicExampleLinux*>(context);
        if (self.sensorObject == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Ultrasonic Linux adapter has no bound sensor");
        }
        return self;
    }

    /** @brief Reads one distance in centimeters from the bound sensor. */
    float64 XWalkUltrasonicExampleLinux::read(contextpointer context)
    {
        return adapter(context).sensorObject->read();
    }

    /**
     * @brief Waits for one source-compatible sample interval.
     * @param[in,out] context Non-null Linux adapter context.
     * @param[in] durationMilliseconds Requested duration in milliseconds.
     */
    void XWalkUltrasonicExampleLinux::wait(contextpointer context, uint32 durationMilliseconds)
    {
        static_cast<void>(adapter(context));
        common::sleepMilliseconds(durationMilliseconds);
    }

    /**
     * @brief Prints one source-compatible distance and refreshes the current line.
     * @param[in,out] context Non-null Linux adapter context.
     * @param[in] distanceCentimeters Measured distance in centimeters.
     */
    void XWalkUltrasonicExampleLinux::report(contextpointer context, float64 distanceCentimeters)
    {
        static_cast<void>(adapter(context));
        std::cout << "\r\x1b[KDistance: " << distanceCentimeters << " cm" << std::flush;
    }

} /* namespace xwalk::hal::example */
