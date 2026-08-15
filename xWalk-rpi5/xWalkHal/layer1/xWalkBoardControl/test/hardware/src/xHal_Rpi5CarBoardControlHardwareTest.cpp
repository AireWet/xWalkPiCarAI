/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlHardwareTest.cpp
 * @brief       Provides an opt-in Robot HAT board-control hardware smoke test.
 *
 * @details
 * Composes Linux GPIO and I2C backends, injects board-control dependencies, and
 * requests only the inactive speaker state when explicitly executed.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Hardware Test
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
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarI2cLinux.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains hardware-test callbacks private to this translation unit.
 */
namespace
{

    /**
     * @brief Provides the required callback binding without generating test audio.
     *
     * @param[in,out] context
     * Unused nullable context.
     *
     * @param[in] durationMs
     * Unused priming duration because this test never enables the speaker.
     *
     * @warning
     * This callback must not be used to enable physical speaker power.
     */
    void unusedSpeakerPrime(XWalkHal::contextpointer context, XWalkHal::uint32 durationMs)
    {
        static_cast<void>(context);
        static_cast<void>(durationMs);
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Composes physical dependencies and drives speaker power inactive.
 *
 * @return
 * Zero after the GPIO operation completes; an exception reports hardware failure.
 *
 * @warning
 * Running this function opens `/dev/i2c-1` and drives GPIO 20 low.
 */
XWalkHal::int32 main()
{
    XWalkHal::XWalkI2cLinux i2cBackend;
    XWalkHal::XWalkI2c i2c(&i2cBackend,
                           XHAL_I2C_PROBE_CALLBACK(XWalkHal::XWalkI2cLinux),
                           XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux),
                           XHAL_I2C_READ_CALLBACK(XWalkHal::XWalkI2cLinux));
    XWalkHal::XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL, XHAL_RPI5CAR_ADC_ADDRESS_1);

    XWalkHal::XWalkGpioLinux resetBackend;
    const XWalkHal::XWalkGpioCallbacks callbacks = XHAL_GPIO_CALLBACKS(XWalkHal::XWalkGpioLinux);
    XWalkHal::XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");

    XWalkHal::XWalkGpioLinux speakerBackend;
    XWalkHal::XWalkGpio speakerGpio(&speakerBackend, callbacks, XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
    XWalkHal::XWalkBoardControl control(resetGpio, speakerGpio, batteryAdc, nullptr, &unusedSpeakerPrime);
    control.disableSpeaker();
    return 0;
}
