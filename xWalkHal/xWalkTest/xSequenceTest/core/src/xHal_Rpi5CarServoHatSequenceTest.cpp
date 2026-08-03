/******************************************************************************
 * @file        xHal_Rpi5CarServoHatSequenceTest.cpp
 * @brief       Verifies the Robot HAT servo sequence in memory.
 *
 * @details
 * Checks reset behavior, all 16 servo channels, ten-to-zero angle movement,
 * timing, five-channel ADC reporting, and sample bounds without hardware.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Host Test
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

#include "xHal_Rpi5CarServoHatSequence.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory callbacks and verification scenario. */
namespace
{

/** @brief Records the ordered values written to one simulated GPIO line. */
struct GpioState
{
    /** @brief First two logical output values written by the reset operation. */
    XWalkHal::boolean writes[2U]{};
    /** @brief Total number of simulated GPIO writes. */
    XWalkHal::uint32 writeCount{};
};

/** @brief Counts writes performed through the simulated Robot HAT I2C bus. */
struct I2cState
{
    /** @brief Total number of simulated register writes. */
    XWalkHal::uint32 writeCount{};
};

/** @brief Collects observable sequence callbacks and PWM state. */
struct SequenceState
{
    /** @brief Non-owning PWM pointers ordered by channels zero through 15. */
    XWalkHal::fixedarray<XWalkHal::XWalkPwm*, 16U> pwmObjects{};
    /** @brief Ordered callback wait durations in milliseconds. */
    XWalkHal::uint32vector durations;
    /** @brief Servo channels reported before their movement. */
    XWalkHal::uint32vector reportedServoChannels;
    /** @brief PWM counts observed after each servo angle command. */
    XWalkHal::uint32vector observedPulseWidths;
    /** @brief Most recently reported ADC values ordered by channel. */
    xwalk::hal::test::servohatreadings lastReadings{};
    /** @brief Number of complete five-channel ADC reports. */
    XWalkHal::uint32 adcReportCount{};
    /** @brief Servo channel whose next PWM state is observed. */
    XWalkHal::uint8 currentServoChannel{16U};
};

/**
 * @brief Accepts one simulated GPIO configuration.
 *
 * @param[in,out] context Unused backend context.
 * @param[in] pin Unused GPIO line offset.
 * @param[in] mode Unused GPIO direction.
 * @param[in] pull Unused GPIO pull selection.
 * @param[in] initialValue Unused initial output value.
 */
void configureGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull,
    XWalkHal::boolean initialValue)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
    static_cast<void>(initialValue);
}

/**
 * @brief Returns the fixed simulated GPIO input value.
 *
 * @param[in,out] context Unused backend context.
 * @param[in] pin Unused GPIO line offset.
 *
 * @return Always `false`.
 */
XWalkHal::boolean readGpio(
    XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    return false;
}

/**
 * @brief Records a simulated GPIO output value.
 *
 * @param[in,out] context Non-null `GpioState` pointer.
 * @param[in] pin Unused GPIO line offset.
 * @param[in] value Logical output value to record.
 */
void writeGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::boolean value)
{
    GpioState& state = *static_cast<GpioState*>(context);
    static_cast<void>(pin);
    if (state.writeCount < 2U)
    {
        state.writes[state.writeCount] = value;
    }
    ++state.writeCount;
}

/**
 * @brief Accepts an unused simulated interrupt registration.
 *
 * @param[in,out] context Unused backend context.
 * @param[in] pin Unused GPIO line offset.
 * @param[in] edge Unused interrupt edge.
 * @param[in] debounceMilliseconds Unused debounce duration in milliseconds.
 * @param[in,out] handlerContext Unused application context.
 * @param[in] handler Unused application handler.
 */
void interruptGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioEdge edge, XWalkHal::uint32 debounceMilliseconds,
    XWalkHal::contextpointer handlerContext,
    XWalkHal::gpiointerrupthandler handler)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(edge);
    static_cast<void>(debounceMilliseconds);
    static_cast<void>(handlerContext);
    static_cast<void>(handler);
}

/**
 * @brief Accepts an unused simulated interrupt cancellation.
 *
 * @param[in,out] context Unused backend context.
 * @param[in] pin Unused GPIO line offset.
 */
void cancelGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

/**
 * @brief Builds the complete simulated GPIO callback set.
 *
 * @return Callback bindings used by the reset and speaker GPIO objects.
 */
XWalkHal::XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio,
        &interruptGpio, &cancelGpio};
}

/**
 * @brief Reports every simulated Robot HAT I2C address as available.
 *
 * @param[in,out] context Unused backend context.
 * @param[in] address Unused seven-bit I2C address.
 *
 * @return Always `true`.
 */
XWalkHal::boolean probeI2c(
    XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

/**
 * @brief Counts a simulated I2C register write.
 *
 * @param[in,out] context Non-null `I2cState` pointer.
 * @param[in] address Unused seven-bit I2C address.
 * @param[in] reg Unused register address.
 * @param[in] data Unused register payload.
 */
void writeI2c(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data)
{
    I2cState& state = *static_cast<I2cState*>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    ++state.writeCount;
}

/**
 * @brief Returns one deterministic two-byte ADC payload.
 *
 * @param[in,out] context Unused backend context.
 * @param[in] address Unused seven-bit I2C address.
 * @param[in] length Required payload length of two bytes.
 *
 * @return Bytes `0x01` and `0x02`, representing the raw value 258.
 */
XWalkHal::bytevector readI2c(XWalkHal::contextpointer context,
    XWalkHal::uint8 address, XWalkHal::size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    assert(length == 2U);
    return {0x01U, 0x02U};
}

/**
 * @brief Accepts the unused board-control speaker-prime request.
 *
 * @param[in,out] context Unused callback context.
 * @param[in] durationMilliseconds Unused prime duration in milliseconds.
 */
void unusedSpeakerPrime(
    XWalkHal::contextpointer context, XWalkHal::uint32 durationMilliseconds)
{
    static_cast<void>(context);
    static_cast<void>(durationMilliseconds);
}

/**
 * @brief Records a wait and the active servo PWM count when applicable.
 *
 * @param[in,out] context Non-null `SequenceState` pointer.
 * @param[in] durationMilliseconds Requested wait duration in milliseconds.
 */
void wait(XWalkHal::contextpointer context,
    XWalkHal::uint32 durationMilliseconds)
{
    SequenceState& state = *static_cast<SequenceState*>(context);
    state.durations.push_back(durationMilliseconds);
    if (durationMilliseconds == 100U)
    {
        const XWalkHal::size channel =
            static_cast<XWalkHal::size>(state.currentServoChannel);
        assert(channel < state.pwmObjects.size());
        state.observedPulseWidths.push_back(
            state.pwmObjects[channel]->pulseWidth());
    }
}

/**
 * @brief Records the servo channel reported before movement.
 *
 * @param[in,out] context Non-null `SequenceState` pointer.
 * @param[in] channel Servo PWM channel in the range zero through 15.
 */
void reportServo(XWalkHal::contextpointer context, XWalkHal::uint8 channel)
{
    SequenceState& state = *static_cast<SequenceState*>(context);
    state.currentServoChannel = channel;
    state.reportedServoChannels.push_back(static_cast<XWalkHal::uint32>(channel));
}

/**
 * @brief Records one complete ordered ADC report.
 *
 * @param[in,out] context Non-null `SequenceState` pointer.
 * @param[in] readings Raw ADC readings ordered by channels zero through four.
 */
void reportAdc(XWalkHal::contextpointer context,
    const xwalk::hal::test::servohatreadings& readings)
{
    SequenceState& state = *static_cast<SequenceState*>(context);
    state.lastReadings = readings;
    ++state.adcReportCount;
}

/**
 * @brief Exercises the complete sequence against in-memory HAL backends.
 *
 * @post Every assertion covering reset, servo, ADC, timing, and bounds passes.
 */
void runTest()
{
    GpioState resetState;
    GpioState speakerState;
    I2cState i2cState;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    XWalkHal::XWalkGpio resetGpio(&resetState, callbacks, "MCURST");
    XWalkHal::XWalkGpio speakerGpio(&speakerState, callbacks,
        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
    XWalkHal::XWalkI2c i2c(&i2cState, &probeI2c, &writeI2c, &readI2c);
    XWalkHal::XWalkAdc adc0(i2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkAdc adc1(i2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkAdc adc2(i2c, 2U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkAdc adc3(i2c, 3U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkAdc adc4(i2c, 4U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkBoardControl boardControl(resetGpio, speakerGpio, adc4,
        nullptr, &unusedSpeakerPrime);

    XWalkHal::XWalkPwmTimerState timerState;
    XWalkHal::XWalkPwm pwm0(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm1(i2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm2(i2c, 2U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm3(i2c, 3U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm4(i2c, 4U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm5(i2c, 5U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm6(i2c, 6U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm7(i2c, 7U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm8(i2c, 8U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm9(i2c, 9U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm10(i2c, 10U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm11(i2c, 11U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm12(i2c, 12U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm13(i2c, 13U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm14(i2c, 14U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm15(i2c, 15U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);

    XWalkHal::XWalkServo servo0(pwm0);
    XWalkHal::XWalkServo servo1(pwm1);
    XWalkHal::XWalkServo servo2(pwm2);
    XWalkHal::XWalkServo servo3(pwm3);
    XWalkHal::XWalkServo servo4(pwm4);
    XWalkHal::XWalkServo servo5(pwm5);
    XWalkHal::XWalkServo servo6(pwm6);
    XWalkHal::XWalkServo servo7(pwm7);
    XWalkHal::XWalkServo servo8(pwm8);
    XWalkHal::XWalkServo servo9(pwm9);
    XWalkHal::XWalkServo servo10(pwm10);
    XWalkHal::XWalkServo servo11(pwm11);
    XWalkHal::XWalkServo servo12(pwm12);
    XWalkHal::XWalkServo servo13(pwm13);
    XWalkHal::XWalkServo servo14(pwm14);
    XWalkHal::XWalkServo servo15(pwm15);

    const xwalk::hal::test::servohatservoarray servos{{
        &servo0, &servo1, &servo2, &servo3, &servo4, &servo5, &servo6, &servo7,
        &servo8, &servo9, &servo10, &servo11, &servo12, &servo13, &servo14,
        &servo15}};
    const xwalk::hal::test::servohatadcarray adcInputs{{
        &adc0, &adc1, &adc2, &adc3, &adc4}};
    SequenceState sequenceState;
    sequenceState.pwmObjects = {{
        &pwm0, &pwm1, &pwm2, &pwm3, &pwm4, &pwm5, &pwm6, &pwm7,
        &pwm8, &pwm9, &pwm10, &pwm11, &pwm12, &pwm13, &pwm14, &pwm15}};
    xwalk::hal::test::XWalkServoHatSequence sequence(boardControl, servos,
        adcInputs, &sequenceState, &wait, &reportServo, &reportAdc);

    sequence.run(1U);

    assert(resetState.writeCount == 2U);
    assert(!resetState.writes[0U]);
    assert(resetState.writes[1U]);
    assert(sequenceState.reportedServoChannels == XWalkHal::uint32vector({
        0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U,
        8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U}));
    assert(sequenceState.observedPulseWidths.size() == 32U);
    for (XWalkHal::size index = 0U;
         index < sequenceState.observedPulseWidths.size(); ++index)
    {
        const XWalkHal::uint32 expectedPulseWidth =
            ((index % 2U) == 0U) ? 329U : 307U;
        assert(sequenceState.observedPulseWidths[index] == expectedPulseWidth);
    }
    assert(sequenceState.durations.size() == 34U);
    assert(sequenceState.durations.front() == 1'000U);
    assert(sequenceState.durations.back() == 1'000U);
    assert(sequenceState.adcReportCount == 1U);
    assert(sequenceState.lastReadings == xwalk::hal::test::servohatreadings({
        258U, 258U, 258U, 258U, 258U}));
    assert(adc0.channel() == 0U);
    assert(adc1.channel() == 1U);
    assert(adc2.channel() == 2U);
    assert(adc3.channel() == 3U);
    assert(adc4.channel() == 4U);
    assert(i2cState.writeCount > 0U);

    XWalkHal::boolean rejectedSamples = false;
    try
    {
        sequence.run(0U);
    }
    catch (const XWalkHal::outofrange&)
    {
        rejectedSamples = true;
    }
    assert(rejectedSamples);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe Robot HAT servo-sequence verification.
 *
 * @return
 * Zero after every assertion passes.
 */
int xWalkServoHatSequenceHostTest()
{
    runTest();
    return 0;
}
