/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlLifecycle.cpp
 * @brief       Implements xWalk board-control validation and lifecycle behavior.
 *
 * @details
 * Validates injected GPIO, ADC, and speaker-priming roles and retains them as
 * non-owning bindings without creating or releasing hardware dependencies.
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
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates all injected hardware roles and callback binding.
 *
 * @param[in] mcuResetGpio
 * GPIO required to use the Robot HAT MCU-reset line.
 *
 * @param[in] speakerEnableGpio
 * GPIO required to use a supported board speaker-enable line.
 *
 * @param[in] batteryAdc
 * ADC required to use battery channel A4.
 *
 * @param[in] speakerPrimeCallback
 * Callback required to prime speaker output.
 *
 * @throws std::invalid_argument
 * If a hardware object is bound to the wrong role or the callback is null.
 */
void XWalkBoardControl::validateDependencies(const XWalkGpio& mcuResetGpio,
    const XWalkGpio& speakerEnableGpio, const XWalkAdc& batteryAdc,
    boardspeakerprimecallback speakerPrimeCallback)
{
    const hal::boolean mcuResetGpioPinDifferent =
        static_cast<hal::boolean>(
            mcuResetGpio.pin() != XHAL_RPI5CAR_BOARD_CONTROL_MCU_RESET_PIN);
    if (mcuResetGpioPinDifferent)
    {
        XHAL_THROW_INVALID_ARGUMENT("Board control requires the MCU-reset GPIO");
    }

    const uint8 speakerPin = speakerEnableGpio.pin();
    const boolean isLegacySpeakerPin =
        speakerPin == XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN;
    const boolean isVersionFiveSpeakerPin =
        speakerPin == XHAL_RPI5CAR_DEVICE_V5_SPEAKER_ENABLE_PIN;
    if (!isLegacySpeakerPin && !isVersionFiveSpeakerPin)
    {
        XHAL_THROW_INVALID_ARGUMENT("Board control speaker GPIO is unsupported");
    }

    const hal::boolean batteryAdcChannelDifferent =
        static_cast<hal::boolean>(
            batteryAdc.channel() != XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL);
    if (batteryAdcChannelDifferent)
    {
        XHAL_THROW_INVALID_ARGUMENT("Board control requires battery ADC channel A4");
    }
    if (speakerPrimeCallback == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Board control speaker-prime callback is null");
    }
}

/**
 * @brief Primes the powered speaker for the required bounded duration.
 *
 * @pre
 * The speaker-enable line is active and the callback context is valid.
 */
void XWalkBoardControl::primeSpeakerOutput()
{
    speakerPrimeCallbackValue(speakerPrimeContextPointer,
        XHAL_RPI5CAR_BOARD_CONTROL_SPEAKER_PRIME_DURATION_MS);
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a controller from application-owned dependencies.
 *
 * @param[in] mcuResetGpio
 * MCU-reset GPIO that must outlive this controller.
 *
 * @param[in] speakerEnableGpio
 * Board-selected speaker GPIO that must outlive this controller.
 *
 * @param[in] batteryAdc
 * ADC channel A4 object that must outlive this controller.
 *
 * @param[in,out] speakerPrimeContext
 * Nullable non-owning callback context. A non-null object must outlive this
 * controller, and null requires explicit callback support.
 *
 * @param[in] speakerPrimeCallback
 * Non-null callback used after speaker power is enabled.
 *
 * @throws std::invalid_argument
 * If a hardware object is bound to the wrong role or the callback is null.
 */
XWalkBoardControl::XWalkBoardControl(XWalkGpio& mcuResetGpio,
    XWalkGpio& speakerEnableGpio, XWalkAdc& batteryAdc,
    contextpointer speakerPrimeContext, boardspeakerprimecallback speakerPrimeCallback):
    mcuResetGpioPointer(&mcuResetGpio),
    speakerEnableGpioPointer(&speakerEnableGpio),
    batteryAdcPointer(&batteryAdc),
    speakerPrimeContextPointer(speakerPrimeContext),
    speakerPrimeCallbackValue(speakerPrimeCallback)
{
    validateDependencies(mcuResetGpio, speakerEnableGpio, batteryAdc,
        speakerPrimeCallback);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the controller without changing or releasing hardware.
 */
XWalkBoardControl::~XWalkBoardControl() = default;

} /* namespace xwalk::hal */
