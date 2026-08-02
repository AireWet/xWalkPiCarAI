/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechTest.cpp
 * @brief       Verifies text-to-speech composition with in-memory backends.
 *
 * @details
 * Checks callback validation, construction-time speaker activation, text
 * forwarding, backend failure propagation, and destructor power behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Test
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

#include "xHal_Rpi5CarTextToSpeech.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains host-test state and callbacks private to this translation unit.
 */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Retains the current physical value written to one test GPIO. */
struct TestGpioBackend
{
    boolean physicalValue{}; /**< Most recently configured or written level. */
    uint32 writeCount{}; /**< Number of explicit physical write operations. */
};

/** @brief Provides the context required by the injected I2C interface. */
struct TestI2cBackend
{
    uint32 readCount{}; /**< Number of sequential reads requested by a consumer. */
};

/** @brief Records speaker priming performed during TTS construction. */
struct TestSpeakerPrime
{
    uint32 callCount{}; /**< Number of completed priming callback entries. */
    uint32 durationMs{}; /**< Most recently requested priming duration in milliseconds. */
};

/** @brief Records speech requests and optionally reports a backend failure. */
struct TestSpeechBackend
{
    string text{}; /**< Owned copy of the most recently requested text. */
    uint32 callCount{}; /**< Number of speech callback entries. */
    boolean fail{}; /**< `true` to throw during the next speech request. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Records one GPIO configuration operation.
 *
 * @param[in,out] context
 * Non-null test GPIO state.
 *
 * @param[in] pin
 * Configured GPIO line.
 *
 * @param[in] mode
 * Requested GPIO direction.
 *
 * @param[in] pull
 * Requested internal bias.
 *
 * @param[in] initialValue
 * Initial physical output level retained by the backend.
 */
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue)
{
    TestGpioBackend& backend = *static_cast<TestGpioBackend*>(context);
    backend.physicalValue = initialValue;
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
}

/**
 * @brief Returns the current test GPIO level.
 *
 * @param[in,out] context
 * Non-null test GPIO state.
 *
 * @param[in] pin
 * Configured GPIO line.
 *
 * @return
 * Most recently configured or written physical level.
 */
boolean readGpio(contextpointer context, uint8 pin)
{
    static_cast<void>(pin);
    return static_cast<TestGpioBackend*>(context)->physicalValue;
}

/**
 * @brief Records one physical GPIO output operation.
 *
 * @param[in,out] context
 * Non-null test GPIO state.
 *
 * @param[in] pin
 * Configured GPIO line.
 *
 * @param[in] value
 * Physical output level to retain.
 */
void writeGpio(contextpointer context, uint8 pin, boolean value)
{
    TestGpioBackend& backend = *static_cast<TestGpioBackend*>(context);
    backend.physicalValue = value;
    ++backend.writeCount;
    static_cast<void>(pin);
}

/**
 * @brief Accepts an unused GPIO interrupt registration.
 *
 * @param[in,out] context
 * Non-owning test GPIO state.
 *
 * @param[in] pin
 * Configured GPIO line.
 *
 * @param[in] edge
 * Requested signal transition.
 *
 * @param[in] debounceMs
 * Requested debounce interval in milliseconds.
 *
 * @param[in,out] handlerContext
 * Non-owning application handler context.
 *
 * @param[in] handler
 * Application handler supplied by the GPIO interface.
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
 * @brief Accepts an unused GPIO interrupt cancellation.
 *
 * @param[in,out] context
 * Non-owning test GPIO state.
 *
 * @param[in] pin
 * Configured GPIO line.
 */
void cancelInterrupt(contextpointer context, uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

/**
 * @brief Creates the complete callback table required by each test GPIO.
 *
 * @return
 * Callback table containing only non-null in-memory operations.
 */
XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
}

/**
 * @brief Reports every test I2C address as present.
 *
 * @param[in,out] context
 * Non-owning test I2C state.
 *
 * @param[in] address
 * Seven-bit address being probed.
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
 * @brief Accepts an unused I2C register write.
 *
 * @param[in,out] context
 * Non-owning test I2C state.
 *
 * @param[in] address
 * Seven-bit destination address.
 *
 * @param[in] reg
 * Destination register.
 *
 * @param[in] data
 * Payload supplied by the interface.
 */
void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
}

/**
 * @brief Supplies an unused sequential I2C response.
 *
 * @param[in,out] context
 * Non-null test I2C state.
 *
 * @param[in] address
 * Seven-bit source address.
 *
 * @param[in] length
 * Requested byte count.
 *
 * @return
 * Empty payload because the ADC is not sampled by this test.
 */
bytevector readI2c(contextpointer context, uint8 address, size length)
{
    TestI2cBackend& backend = *static_cast<TestI2cBackend*>(context);
    ++backend.readCount;
    static_cast<void>(address);
    static_cast<void>(length);
    return {};
}

/**
 * @brief Records speaker priming performed during TTS construction.
 *
 * @param[in,out] context
 * Non-null speaker-prime test state.
 *
 * @param[in] durationMs
 * Requested priming duration in milliseconds.
 */
void primeSpeaker(contextpointer context, uint32 durationMs)
{
    TestSpeakerPrime& prime = *static_cast<TestSpeakerPrime*>(context);
    ++prime.callCount;
    prime.durationMs = durationMs;
}

/**
 * @brief Records one speech request or raises the configured test failure.
 *
 * @param[in,out] context
 * Non-null speech-backend test state.
 *
 * @param[in] text
 * Speech text copied before the callback returns.
 *
 * @throws std::runtime_error
 * If the test state requests backend failure.
 */
void speakText(contextpointer context, stringview text)
{
    TestSpeechBackend& backend = *static_cast<TestSpeechBackend*>(context);
    ++backend.callCount;
    backend.text = string(text);
    if (backend.fail)
    {
        XHAL_THROW_RUNTIME_ERROR("Test text-to-speech backend failed");
    }
}

/**
 * @brief Verifies speaker activation, text delivery, and retained power state.
 */
void testCompositionAndSpeech()
{
    TestGpioBackend resetBackend;
    TestGpioBackend speakerBackend;
    TestI2cBackend i2cBackend;
    TestSpeakerPrime prime;
    TestSpeechBackend speech;
    const XWalkGpioCallbacks callbacks = gpioCallbacks();
    XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
    XWalkGpio speakerGpio(&speakerBackend, callbacks,
        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
    XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
    XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
        XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkBoardControl control(resetGpio, speakerGpio, batteryAdc,
        &prime, &primeSpeaker);

    {
        XWalkTextToSpeech textToSpeech(control, &speech, &speakText);
        assert(speakerBackend.physicalValue);
        assert(prime.callCount == 1U);
        assert(prime.durationMs == XHAL_RPI5CAR_BOARD_CONTROL_SPEAKER_PRIME_DURATION_MS);

        textToSpeech.speak("Robot ready");
        assert(speech.callCount == 1U);
        assert(speech.text == "Robot ready");
        textToSpeech.speak("");
        assert(speech.callCount == 2U);
        assert(speech.text.empty());

        speech.fail = true;
        xwalk::hal::test::expectFailure([&]()
        {
            textToSpeech.speak("failure");
        });
    }

    assert(speakerBackend.physicalValue);
}

/** @brief Verifies a null backend is rejected before speaker activation. */
void testCallbackValidation()
{
    TestGpioBackend resetBackend;
    TestGpioBackend speakerBackend;
    TestI2cBackend i2cBackend;
    TestSpeakerPrime prime;
    const XWalkGpioCallbacks callbacks = gpioCallbacks();
    XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
    XWalkGpio speakerGpio(&speakerBackend, callbacks,
        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
    XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
    XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
        XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkBoardControl control(resetGpio, speakerGpio, batteryAdc,
        &prime, &primeSpeaker);

    xwalk::hal::test::expectFailure([&]()
    {
        XWalkTextToSpeech textToSpeech(control, nullptr, nullptr);
        static_cast<void>(textToSpeech);
    });
    assert(!speakerBackend.physicalValue);
    assert(prime.callCount == 0U);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every host-side text-to-speech test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int32 main()
{
    testCompositionAndSpeech();
    testCallbackValidation();
    return 0;
}
