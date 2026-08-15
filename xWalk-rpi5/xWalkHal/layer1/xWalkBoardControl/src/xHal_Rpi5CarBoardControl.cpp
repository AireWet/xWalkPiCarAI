/******************************************************************************
 * @file        xHal_Rpi5CarBoardControl.cpp
 * @brief       Implements Robot HAT board-level control operations.
 *
 * @details
 * Performs GPIO output changes, timed MCU reset, scaled battery acquisition,
 * and speaker-enable sequencing through application-owned dependencies.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarTrace.h"

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
     * @brief Drives a caller-owned GPIO to one logical level.
     *
     * @param[in,out] gpio
     * Non-owning GPIO reference configured as an output by this operation.
     *
     * @param[in] value
     * Logical output level to drive.
     */
    void XWalkBoardControl::setPin(XWalkGpio& gpio, boolean value)
    {
        static_cast<void>(gpio.write(value));
        XWALK_HAL_TRACE_UID2(
            RPI .322, "Board-control GPIO %u set to logical value %u", gpio.pin(), static_cast<uint32>(value));
    }

    /**
     * @brief Pulses the Robot HAT MCU reset signal.
     *
     * @details
     * Drives reset low for 10 milliseconds, then high for 10 milliseconds.
     *
     * @post
     * The reset line is logically high after successful completion.
     *
     * @warning
     * A GPIO failure after reset is asserted can leave the MCU held in reset.
     */
    void XWalkBoardControl::resetMcu()
    {
        setPin(*mcuResetGpioPointer, false);
        common::sleepMilliseconds(XHAL_RPI5CAR_BOARD_CONTROL_RESET_INTERVAL_MS);
        setPin(*mcuResetGpioPointer, true);
        common::sleepMilliseconds(XHAL_RPI5CAR_BOARD_CONTROL_RESET_INTERVAL_MS);
        XWALK_HAL_TRACE_UID0(RPI .323, "Robot HAT MCU reset sequence completed");
    }

    /**
     * @brief Acquires the estimated Robot HAT battery voltage.
     *
     * @return
     * ADC voltage multiplied by the board divider ratio, in volts.
     */
    float64 XWalkBoardControl::batteryVoltage()
    {
        const float64 measuredVoltage = batteryAdcPointer->readVoltage();
        const float64 dividerRatio = static_cast<float64>(XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_DIVIDER_RATIO);
        const float64 batteryVoltageValue = measuredVoltage * dividerRatio;
        XWALK_HAL_TRACE_UID1(RPI .324, "Robot HAT battery voltage is %.3f volts", batteryVoltageValue);
        return batteryVoltageValue;
    }

    /**
     * @brief Enables speaker power and primes the audio output.
     *
     * @post
     * Speaker power remains enabled after priming succeeds.
     */
    void XWalkBoardControl::enableSpeaker()
    {
        setPin(*speakerEnableGpioPointer, true);
        primeSpeakerOutput();
        XWALK_HAL_TRACE_UID0(RPI .325, "Robot HAT speaker power enabled and primed");
    }

    /**
     * @brief Disables physical speaker power.
     *
     * @post
     * The speaker-enable GPIO is logically low after successful completion.
     */
    void XWalkBoardControl::disableSpeaker()
    {
        setPin(*speakerEnableGpioPointer, false);
    }

} /* namespace xwalk::hal */
