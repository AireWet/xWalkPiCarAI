/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlTest.cpp
 * @brief       Verifies xWalk board control through in-memory backends.
 *
 * @details
 * Tests pin output, MCU-reset sequencing, battery scaling, speaker priming,
 * priming failure, and injected hardware-role validation without hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Test
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

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test state and callbacks private to this translation unit.
 */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records GPIO configuration and ordered logical output writes. */
struct TestGpioBackend
{
    uint8 pin{}; /**< Most recently configured GPIO line. */
    boolean physicalValue{}; /**< Most recently driven physical level. */
    fixedarray<boolean, 8U> writes{}; /**< Bounded ordered physical output writes. */
    size writeCount{}; /**< Number of valid entries in `writes`. */
};

/** @brief Supplies deterministic ADC bytes through the I2C callback interface. */
struct TestI2cBackend
{
    bytevector sampleBytes{0x0FU, 0xFFU}; /**< Next raw 12-bit ADC sample bytes. */
};

/** @brief Records speaker priming and optionally reports a backend failure. */
struct TestSpeakerPrime
{
    uint32 durationMs{}; /**< Most recently requested priming duration. */
    uint32 callCount{}; /**< Number of completed callback entries. */
    boolean fail{}; /**< `true` to throw during the next callback. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Records one GPIO configuration request.
 *
 * @param[in,out] context
 * Non-null test GPIO backend.
 *
 * @param[in] pin
 * GPIO line selected by the GPIO object.
 *
 * @param[in] mode
 * Requested GPIO direction.
 *
 * @param[in] pull
 * Requested internal pull configuration.
 *
 * @param[in] initialValue
 * Initial physical output level.
 */
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue)
{
    TestGpioBackend& backend = *static_cast<TestGpioBackend*>(context);
    backend.pin = pin;
    backend.physicalValue = initialValue;
    static_cast<void>(mode);
    static_cast<void>(pull);
}

/**
 * @brief Returns the most recently driven physical GPIO level.
 *
 * @param[in,out] context
 * Non-null test GPIO backend.
 *
 * @param[in] pin
 * GPIO line expected to match the configured line.
 *
 * @return
 * Most recently stored physical output level.
 */
boolean readGpio(contextpointer context, uint8 pin)
{
    TestGpioBackend& backend = *static_cast<TestGpioBackend*>(context);
    assert(pin == backend.pin);
    return backend.physicalValue;
}

/**
 * @brief Records one physical GPIO output level.
 *
 * @param[in,out] context
 * Non-null test GPIO backend.
 *
 * @param[in] pin
 * GPIO line expected to match the configured line.
 *
 * @param[in] value
 * Physical output level to record.
 */
void writeGpio(contextpointer context, uint8 pin, boolean value)
{
    TestGpioBackend& backend = *static_cast<TestGpioBackend*>(context);
    assert(pin == backend.pin);
    assert(backend.writeCount < backend.writes.size());
    backend.physicalValue = value;
    backend.writes[backend.writeCount] = value;
    ++backend.writeCount;
}

/**
 * @brief Accepts a test interrupt registration without creating a worker.
 *
 * @param[in,out] context
 * Non-owning test backend context.
 *
 * @param[in] pin
 * GPIO line associated with the registration.
 *
 * @param[in] edge
 * Requested edge selection.
 *
 * @param[in] debounceMs
 * Requested debounce interval in milliseconds.
 *
 * @param[in,out] handlerContext
 * Non-owning application handler context.
 *
 * @param[in] handler
 * Non-null application handler.
 */
void registerInterrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(edge);
    static_cast<void>(debounceMs);
    static_cast<void>(handlerContext);
    static_cast<void>(handler);
}

/**
 * @brief Accepts cancellation for the in-memory GPIO backend.
 *
 * @param[in,out] context
 * Non-owning test backend context.
 *
 * @param[in] pin
 * GPIO line whose registration would be cancelled.
 */
void cancelInterrupt(contextpointer context, uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

/**
 * @brief Creates the complete callback set used by a test GPIO.
 *
 * @return
 * Non-null in-memory GPIO callbacks.
 */
XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
}

/**
 * @brief Reports that any test I2C address is present.
 *
 * @param[in,out] context
 * Non-owning test I2C backend context.
 *
 * @param[in] address
 * Seven-bit address requested by the caller.
 *
 * @return
 * Always `true`.
 */
boolean probeI2c(contextpointer context, uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

/**
 * @brief Accepts an ADC command write.
 *
 * @param[in,out] context
 * Non-owning test I2C backend context.
 *
 * @param[in] address
 * Seven-bit destination address.
 *
 * @param[in] reg
 * ADC command byte.
 *
 * @param[in] data
 * ADC command payload.
 */
void writeI2c(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
}

/**
 * @brief Returns deterministic ADC sample bytes.
 *
 * @param[in,out] context
 * Non-null test I2C backend context.
 *
 * @param[in] address
 * Seven-bit source address.
 *
 * @param[in] length
 * Requested number of bytes.
 *
 * @return
 * Configured raw ADC sample bytes.
 */
bytevector readI2c(contextpointer context, uint8 address, size length)
{
    TestI2cBackend& backend = *static_cast<TestI2cBackend*>(context);
    static_cast<void>(address);
    assert(length == XHAL_RPI5CAR_ADC_READ_LENGTH);
    return backend.sampleBytes;
}

/**
 * @brief Records one speaker priming request.
 *
 * @param[in,out] context
 * Non-null speaker-prime test state.
 *
 * @param[in] durationMs
 * Requested priming duration in milliseconds.
 *
 * @throws std::runtime_error
 * If the test state requests callback failure.
 */
void primeSpeaker(contextpointer context, uint32 durationMs)
{
    TestSpeakerPrime& state = *static_cast<TestSpeakerPrime*>(context);
    state.durationMs = durationMs;
    ++state.callCount;
    if (state.fail)
    {
        XHAL_THROW_RUNTIME_ERROR("Test speaker priming failed");
    }
}

/**
 * @brief Verifies reset, battery scaling, pin output, and speaker sequencing.
 */
void testOperations()
{
    TestGpioBackend resetBackend;
    TestGpioBackend speakerBackend;
    TestI2cBackend i2cBackend;
    TestSpeakerPrime primeState;
    const XWalkGpioCallbacks callbacks = gpioCallbacks();
    XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
    XWalkGpio speakerGpio(&speakerBackend, callbacks,
        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
    XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
    XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
        XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkBoardControl control(resetGpio, speakerGpio, batteryAdc,
        &primeState, &primeSpeaker);

    control.setPin(resetGpio, true);
    assert(resetBackend.physicalValue);
    resetBackend.writeCount = 0U;
    control.resetMcu();
    assert(resetBackend.writeCount == 2U);
    assert(!resetBackend.writes[0U]);
    assert(resetBackend.writes[1U]);

    const float64 voltageDifference = XHAL_ABSOLUTE_VALUE(control.batteryVoltage() - 9.9);
    assert(voltageDifference < 0.000001);

    control.enableSpeaker();
    assert(speakerBackend.physicalValue);
    assert(primeState.callCount == 1U);
    assert(primeState.durationMs == XHAL_RPI5CAR_BOARD_CONTROL_SPEAKER_PRIME_DURATION_MS);
    control.disableSpeaker();
    assert(!speakerBackend.physicalValue);
}

/**
 * @brief Verifies speaker failure and dependency-role validation.
 */
void testFailureAndValidation()
{
    TestGpioBackend resetBackend;
    TestGpioBackend speakerBackend;
    TestGpioBackend wrongResetBackend;
    TestI2cBackend i2cBackend;
    TestSpeakerPrime primeState;
    const XWalkGpioCallbacks callbacks = gpioCallbacks();
    XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
    XWalkGpio wrongResetGpio(&wrongResetBackend, callbacks, "LED");
    XWalkGpio speakerGpio(&speakerBackend, callbacks,
        XHAL_RPI5CAR_DEVICE_V5_SPEAKER_ENABLE_PIN);
    XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
    XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
        XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkBoardControl control(resetGpio, speakerGpio, batteryAdc,
        &primeState, &primeSpeaker);

    primeState.fail = true;
    xwalk::hal::test::expectFailure([&]()
    {
        control.enableSpeaker();
    });
    assert(!speakerBackend.physicalValue);

    xwalk::hal::test::expectFailure([&]()
    {
        XWalkBoardControl invalidControl(wrongResetGpio, speakerGpio, batteryAdc,
            &primeState, &primeSpeaker);
        static_cast<void>(invalidControl);
    });

    xwalk::hal::test::expectFailure([&]()
    {
        XWalkBoardControl invalidControl(resetGpio, speakerGpio, batteryAdc,
            &primeState, nullptr);
        static_cast<void>(invalidControl);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side xWalk board-control tests.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int32 main()
{
    testOperations();
    testFailureAndValidation();
    return 0;
}
