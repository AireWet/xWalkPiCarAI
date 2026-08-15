/******************************************************************************
 * @file        xHal_Rpi5CarInitAnglesSequenceTest.cpp
 * @brief       Verifies initialization angles with in-memory HAL backends.
 *
 * @details
 * Checks MCU reset ordering, three-servo registration, requested logical
 * angles, and PWM communication without accessing physical hardware.
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

#include "xHal_Rpi5CarInitAnglesSequence.h"

#include <cassert>
#include "xHal_Rpi5CarInitAnglesSequenceTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using GpioState = ::xwalk::source_types::xhal_rpi5carinitanglessequencetest::GpioState;
using I2cState = ::xwalk::source_types::xhal_rpi5carinitanglessequencetest::I2cState;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

    void configureGpio(XWalkHal::contextpointer context,
                       XWalkHal::uint8 pin,
                       XWalkHal::XWalkGpioMode mode,
                       XWalkHal::XWalkGpioPull pull,
                       XWalkHal::boolean initialValue)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
        static_cast<void>(initialValue);
    }

    XWalkHal::boolean readGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        return false;
    }

    void writeGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value)
    {
        GpioState& state = *static_cast<GpioState*>(context);
        static_cast<void>(pin);
        if (state.writeCount < 2U)
        {
            state.writes[state.writeCount] = value;
        }
        ++state.writeCount;
    }

    void interruptGpio(XWalkHal::contextpointer context,
                       XWalkHal::uint8 pin,
                       XWalkHal::XWalkGpioEdge edge,
                       XWalkHal::uint32 debounceMilliseconds,
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

    void cancelGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }

    XWalkHal::XWalkGpioCallbacks gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelGpio};
    }

    XWalkHal::boolean probeI2c(XWalkHal::contextpointer context, XWalkHal::uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }

    void writeI2c(XWalkHal::contextpointer context,
                  XWalkHal::uint8 address,
                  XWalkHal::uint8 reg,
                  const XWalkHal::bytevector& data)
    {
        I2cState& state = *static_cast<I2cState*>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
        ++state.writeCount;
    }

    XWalkHal::bytevector readI2c(XWalkHal::contextpointer context, XWalkHal::uint8 address, XWalkHal::size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return XWalkHal::bytevector(length, 0U);
    }

    void unusedSpeakerPrime(XWalkHal::contextpointer context, XWalkHal::uint32 durationMilliseconds)
    {
        static_cast<void>(context);
        static_cast<void>(durationMilliseconds);
    }

    void runTest(XWalkHal::stringview configurationPath)
    {
        GpioState resetState;
        GpioState speakerState;
        I2cState i2cState;
        const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
        XWalkHal::XWalkGpio resetGpio(&resetState, callbacks, "MCURST");
        XWalkHal::XWalkGpio speakerGpio(&speakerState, callbacks, XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
        XWalkHal::XWalkI2c i2c(&i2cState, &probeI2c, &writeI2c, &readI2c);
        XWalkHal::XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkHal::XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc, nullptr, &unusedSpeakerPrime);
        XWalkHal::XWalkPwmTimerState timerState;
        XWalkHal::XWalkPwm firstPwm(i2c, 10U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
        XWalkHal::XWalkPwm secondPwm(i2c, 11U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
        XWalkHal::XWalkPwm thirdPwm(i2c, 12U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
        XWalkHal::XWalkServo firstServo(firstPwm);
        XWalkHal::XWalkServo secondServo(secondPwm);
        XWalkHal::XWalkServo thirdServo(thirdPwm);
        XWalkHal::XWalkConfigStore store(configurationPath);
        store.set("init_angles_servo_offset_list", "[0,0,0]");
        XWalkHal::XWalkRobot robot(store, "init_angles", 0U);

        xwalk::hal::test::XWalkInitAnglesSequence sequence(boardControl, robot, firstServo, secondServo, thirdServo);
        sequence.run();

        assert(resetState.writeCount == 2U);
        assert(!resetState.writes[0U]);
        assert(resetState.writes[1U]);
        assert(robot.initialized());
        assert(robot.servoCount() == 3U);
        assert(robot.servoPositions() == XWalkHal::float64vector({10.0, 45.0, -45.0}));
        assert(i2cState.writeCount > 0U);
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/** @brief Runs the host-safe initialization-angle verification. */
int xWalkInitAnglesSequenceHostTest(int argumentCount, char* argumentValues[])
{
    if (argumentCount != 2)
    {
        return 1;
    }
    const XWalkHal::filesystempath configurationPath(argumentValues[1]);
    XWalkHal::filesystempath replacementPath = configurationPath;
    replacementPath += ".tmp";
    static_cast<void>(XWalkHal::removeFilesystemEntry(configurationPath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(replacementPath));
    runTest(configurationPath.string());
    static_cast<void>(XWalkHal::removeFilesystemEntry(configurationPath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(replacementPath));
    return 0;
}
