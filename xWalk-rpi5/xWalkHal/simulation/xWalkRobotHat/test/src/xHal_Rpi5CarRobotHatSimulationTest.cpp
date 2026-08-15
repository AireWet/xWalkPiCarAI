/******************************************************************************
 * @file        xHal_Rpi5CarRobotHatSimulationTest.cpp
 * @brief       Verifies deterministic Robot HAT simulation and fault injection.
 * @project     xWalk Firmware
 * @module      xWalkRobotHatSimulationTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xHal_Rpi5CarRobotHatSimulationTestSupport.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

namespace
{
    using xwalk::hal::simulation::XWalkRobotHatOperation;
    using xwalk::hal::simulation::XWalkRobotHatSimulation;

    void testHardwareIndependentComposition()
    {
        XWalkRobotHatSimulation backend;
        xwalk::hal::XWalkI2c i2c(&backend,
                                 &XWalkRobotHatSimulation::probe,
                                 &XWalkRobotHatSimulation::writeRegister,
                                 &XWalkRobotHatSimulation::read,
                                 &XWalkRobotHatSimulation::readRegister,
                                 &XWalkRobotHatSimulation::tryWriteRegister);
        backend.setAdcValue(2U, 2048U);
        xwalk::hal::XWalkAdc adc(i2c, 2U, 0x14U);
        assert(adc.read() == 2048U);
        backend.setBatteryVoltage(7.4);
        xwalk::hal::XWalkAdc batteryAdc(i2c, 4U, 0x14U);
        const double batteryVoltage = batteryAdc.readVoltage() * 3.0;
        assert((batteryVoltage > 7.39) && (batteryVoltage < 7.41));
        backend.setGrayscaleValues({101U, 202U, 303U});
        xwalk::hal::XWalkAdc leftGrayscale(i2c, 0U, 0x14U);
        xwalk::hal::XWalkAdc centerGrayscale(i2c, 1U, 0x14U);
        assert((leftGrayscale.read() == 101U) && (centerGrayscale.read() == 202U));
        backend.setUltrasonicDistance(42.5);
        assert(backend.ultrasonicDistance() == 42.5);

        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm pwm(i2c, 12U, 0x14U, timerState);
        pwm.setPulseWidthPercent(25.0);
        assert(backend.registerValue(0x14U, 0x2CU).size() == 2U);

        xwalk::hal::XWalkGpio gpio(&backend, XWalkRobotHatSimulation::gpioCallbacks(), "D4");
        gpio.write(true);
        assert(backend.gpioValue(23U));

        xwalk::hal::XWalkCamera camera(&backend, &XWalkRobotHatSimulation::capture);
        assert(camera.capture("simulated.jpg") == "simulated.jpg");
        assert(xwalk::hal::test::robothat::countEvents(backend, XWalkRobotHatOperation::CameraCapture) == 1U);
        assert(xwalk::hal::test::robothat::hasDeterministicOrdering(backend));

        backend.clearEvents();
        backend.setLogicalDelay(XWalkRobotHatOperation::I2cProbe, 0x14U, 7U);
        assert(i2c.probe(0x14U));
        const auto delayedEvents = backend.events();
        assert((delayedEvents.size() == 1U) && (delayedEvents[0U].sequence == 1U) &&
               (delayedEvents[0U].logicalTime == 7U));

        backend.setCameraFrames({{1U, 2U, 3U}, {4U, 5U}});
        XWalkHal::bytevector frame;
        assert(backend.nextCameraFrame(frame) && (frame.size() == 3U));
        assert(backend.nextCameraFrame(frame) && (frame.size() == 2U));
        assert(!backend.nextCameraFrame(frame) && frame.empty());
    }

    void testTargetedFailureInjection()
    {
        XWalkRobotHatSimulation backend;
        xwalk::hal::XWalkI2c i2c(&backend,
                                 &XWalkRobotHatSimulation::probe,
                                 &XWalkRobotHatSimulation::writeRegister,
                                 &XWalkRobotHatSimulation::read,
                                 &XWalkRobotHatSimulation::readRegister,
                                 &XWalkRobotHatSimulation::tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm pwm(i2c, 12U, 0x14U, timerState);
        xwalk::hal::test::expectFailure(
            []()
            {
                XWalkRobotHatSimulation failingBackend;
                xwalk::hal::XWalkI2c failingI2c(&failingBackend,
                                                &XWalkRobotHatSimulation::probe,
                                                &XWalkRobotHatSimulation::writeRegister,
                                                &XWalkRobotHatSimulation::read,
                                                &XWalkRobotHatSimulation::readRegister,
                                                &XWalkRobotHatSimulation::tryWriteRegister);
                xwalk::hal::XWalkPwmTimerState failingTimerState;
                xwalk::hal::XWalkPwm failingPwm(failingI2c, 12U, 0x14U, failingTimerState);
                failingBackend.failNext(XWalkRobotHatOperation::I2cWrite, 0x2CU);
                failingPwm.setPulseWidthPercent(30.0);
            });
        pwm.setPulseWidthPercent(30.0);

        backend.setAdcValue(7U, 777U);
        xwalk::hal::XWalkAdc adc(i2c, 7U, 0x14U);
        xwalk::hal::test::expectFailure(
            []()
            {
                XWalkRobotHatSimulation failingBackend;
                xwalk::hal::XWalkI2c failingI2c(&failingBackend,
                                                &XWalkRobotHatSimulation::probe,
                                                &XWalkRobotHatSimulation::writeRegister,
                                                &XWalkRobotHatSimulation::read,
                                                &XWalkRobotHatSimulation::readRegister,
                                                &XWalkRobotHatSimulation::tryWriteRegister);
                xwalk::hal::XWalkAdc failingAdc(failingI2c, 7U, 0x14U);
                failingBackend.failNext(XWalkRobotHatOperation::I2cRead, 0x14U);
                static_cast<void>(failingAdc.read());
            });
        assert(adc.read() == 777U);

        xwalk::hal::XWalkGpio gpio(&backend, XWalkRobotHatSimulation::gpioCallbacks(), "D4");
        xwalk::hal::test::expectFailure(
            []()
            {
                XWalkRobotHatSimulation failingBackend;
                xwalk::hal::XWalkGpio failingGpio(&failingBackend, XWalkRobotHatSimulation::gpioCallbacks(), "D4");
                failingBackend.failNext(XWalkRobotHatOperation::GpioWrite, 23U);
                failingGpio.write(true);
            });
        gpio.write(true);

        xwalk::hal::XWalkCamera camera(&backend, &XWalkRobotHatSimulation::capture);
        static_cast<void>(camera);
        xwalk::hal::test::expectFailure(
            []()
            {
                XWalkRobotHatSimulation failingBackend;
                xwalk::hal::XWalkCamera failingCamera(&failingBackend, &XWalkRobotHatSimulation::capture);
                failingBackend.failNext(XWalkRobotHatOperation::CameraCapture, 0U);
                static_cast<void>(failingCamera.capture("failed.jpg"));
            });
    }

} /* namespace */

int main()
{
    testHardwareIndependentComposition();
    testTargetedFailureInjection();
    xwalk::hal::test::robothat::runLogicalModelTests();
    return 0;
}
