/******************************************************************************
 * @file        xHal_Rpi5CarRobotHatSimulation.cpp
 * @brief       Implements the deterministic, device-free Robot HAT simulator.
 * @project     xWalk Firmware
 * @module      xWalkRobotHatSimulation
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xHal_Rpi5CarRobotHatSimulation.h"

#include "xHal_Rpi5CarTrace.h"
#include <cmath>

namespace xwalk::hal::simulation
{

    XWalkRobotHatSimulation& XWalkRobotHatSimulation::from(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Robot HAT simulation context must not be null");
        }
        return *static_cast<XWalkRobotHatSimulation*>(context);
    }

    uint16 XWalkRobotHatSimulation::registerKey(uint8 address, uint8 reg) noexcept
    {
        return static_cast<uint16>((static_cast<uint16>(address) << 8U) | reg);
    }

    boolean XWalkRobotHatSimulation::consumeFailure(XWalkRobotHatOperation operation, uint32 target) noexcept
    {
        for (Failure& failure : failuresValue)
        {
            if ((failure.operation == operation) && (failure.target == target) && (failure.remaining > 0U))
            {
                --failure.remaining;
                return true;
            }
        }
        return false;
    }

    void XWalkRobotHatSimulation::record(XWalkRobotHatOperation operation,
                                         uint32 target,
                                         uint32 value,
                                         boolean succeeded,
                                         const bytevector& data,
                                         stringview text)
    {
        const auto delay = delaysValue.find({operation, target});
        const uint32 delayTicks = (delay == delaysValue.end()) ? 0U : delay->second;
        logicalTimeValue += delayTicks;
        eventsValue.push_back(
            {nextSequenceValue, logicalTimeValue, operation, target, value, succeeded, data, string(text)});
        ++nextSequenceValue;
        ++logicalTimeValue;
    }

    XWalkGpioCallbacks XWalkRobotHatSimulation::gpioCallbacks() noexcept
    {
        return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelInterruptGpio};
    }

    void XWalkRobotHatSimulation::setI2cPresent(uint8 address, boolean present)
    {
        if (address > 0x7FU)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "simulated I2C address must be seven bit");
        }
        const std::lock_guard<std::mutex> lock(mutexValue);
        if (present)
        {
            presentAddressesValue.insert(address);
        }
        else
        {
            presentAddressesValue.erase(address);
        }
    }

    void XWalkRobotHatSimulation::setAdcValue(uint8 channel, uint16 value)
    {
        if (channel >= adcValuesValue.size())
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "simulated ADC channel must be between zero and seven");
        }
        if (value > 4095U)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "simulated ADC value must be twelve bit");
        }
        const std::lock_guard<std::mutex> lock(mutexValue);
        adcValuesValue[channel] = value;
    }

    void XWalkRobotHatSimulation::setGrayscaleValues(const std::array<uint16, 3U>& values)
    {
        for (size index = 0U; index < values.size(); ++index)
        {
            setAdcValue(static_cast<uint8>(index), values[index]);
        }
    }

    void XWalkRobotHatSimulation::setBatteryVoltage(float64 voltage)
    {
        constexpr float64 maximumBatteryVoltage{9.9};
        constexpr float64 maximumAdcCount{4095.0};
        if (!std::isfinite(voltage))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "simulated battery voltage must be finite");
        }
        if ((voltage < 0.0) || (voltage > maximumBatteryVoltage))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "simulated battery voltage must be between zero and 9.9 volts");
        }
        const uint16 sample = static_cast<uint16>(std::lround((voltage / maximumBatteryVoltage) * maximumAdcCount));
        setAdcValue(4U, sample);
    }

    void XWalkRobotHatSimulation::setUltrasonicDistance(float64 distanceCentimeters)
    {
        if (!std::isfinite(distanceCentimeters))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "simulated ultrasonic distance must be finite");
        }
        if ((distanceCentimeters < 0.0) || (distanceCentimeters > 500.0))
        {
            XWALK_HAL_ERROR(XWALK_RANGE,
                            "simulated ultrasonic distance must be "
                            "between zero and 500 centimeters");
        }
        const std::lock_guard<std::mutex> lock(mutexValue);
        ultrasonicDistanceCentimetersValue = distanceCentimeters;
    }

    float64 XWalkRobotHatSimulation::ultrasonicDistance() const noexcept
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        return ultrasonicDistanceCentimetersValue;
    }

    void XWalkRobotHatSimulation::setGpioValue(uint8 pin, boolean value)
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        gpioValuesValue[pin] = value;
    }

    void XWalkRobotHatSimulation::setCameraAvailable(boolean available) noexcept
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        cameraAvailableValue = available;
    }

    void XWalkRobotHatSimulation::setCameraFrames(const std::vector<bytevector>& frames)
    {
        for (const bytevector& frame : frames)
        {
            if (frame.empty())
            {
                XWALK_HAL_ERROR(XWALK_INVAL, "simulated camera frames must not be empty");
            }
        }
        const std::lock_guard<std::mutex> lock(mutexValue);
        cameraFramesValue = frames;
        nextCameraFrameValue = 0U;
    }

    boolean XWalkRobotHatSimulation::nextCameraFrame(bytevector& frame)
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        const boolean failed = consumeFailure(XWalkRobotHatOperation::CameraCapture, 0U) || !cameraAvailableValue;
        if (failed)
        {
            record(XWalkRobotHatOperation::CameraCapture, 0U, 0U, false);
            XWALK_HAL_ERROR(XWALK_RUNTIME, "injected or unavailable simulated camera frame");
        }
        if (nextCameraFrameValue >= cameraFramesValue.size())
        {
            frame.clear();
            record(XWalkRobotHatOperation::CameraCapture, 0U, 0U, true, {}, "end-of-sequence");
            return false;
        }
        frame = cameraFramesValue[nextCameraFrameValue];
        ++nextCameraFrameValue;
        record(XWalkRobotHatOperation::CameraCapture, 0U, static_cast<uint32>(frame.size()), true, frame);
        return true;
    }

    void XWalkRobotHatSimulation::setLogicalDelay(XWalkRobotHatOperation operation, uint32 target, uint32 ticks)
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        delaysValue[{operation, target}] = ticks;
    }

    void XWalkRobotHatSimulation::failNext(XWalkRobotHatOperation operation, uint32 target, uint32 count)
    {
        if (count == 0U)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "simulated failure count must not be zero");
        }
        const std::lock_guard<std::mutex> lock(mutexValue);
        failuresValue.push_back({operation, target, count});
    }

    void XWalkRobotHatSimulation::clearFailures() noexcept
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        failuresValue.clear();
    }

    std::vector<XWalkRobotHatEvent> XWalkRobotHatSimulation::events() const
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        return eventsValue;
    }

    void XWalkRobotHatSimulation::clearEvents() noexcept
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        eventsValue.clear();
        nextSequenceValue = 1U;
        logicalTimeValue = 0U;
    }

    bytevector XWalkRobotHatSimulation::registerValue(uint8 address, uint8 reg) const
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        const auto value = registersValue.find(registerKey(address, reg));
        return (value == registersValue.end()) ? bytevector{} : value->second;
    }

    boolean XWalkRobotHatSimulation::gpioValue(uint8 pin) const
    {
        const std::lock_guard<std::mutex> lock(mutexValue);
        return gpioValuesValue[pin];
    }

    boolean XWalkRobotHatSimulation::probe(contextpointer context, uint8 address)
    {
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::I2cProbe, address);
        const boolean present = !failed && (simulation.presentAddressesValue.count(address) != 0U);
        simulation.record(XWalkRobotHatOperation::I2cProbe, address, 0U, present);
        return present;
    }

    void
    XWalkRobotHatSimulation::writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::I2cWrite, reg);
        simulation.record(XWalkRobotHatOperation::I2cWrite, reg, address, !failed, data);
        if (failed)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "injected Robot HAT I2C write failure");
        }
        if (simulation.presentAddressesValue.count(address) == 0U)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "simulated Robot HAT I2C device is unavailable");
        }
        simulation.registersValue[registerKey(address, reg)] = data;
        if ((reg >= 0x10U) && (reg <= 0x17U))
        {
            simulation.adcCommandValue[address] = reg;
        }
    }

    boolean XWalkRobotHatSimulation::tryWriteRegister(contextpointer context,
                                                      uint8 address,
                                                      uint8 reg,
                                                      const bytevector& data) noexcept
    {
        if (context == nullptr)
        {
            return false;
        }
        XWalkRobotHatSimulation& simulation = *static_cast<XWalkRobotHatSimulation*>(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::I2cWrite, reg) ||
                               (simulation.presentAddressesValue.count(address) == 0U);
        simulation.record(XWalkRobotHatOperation::I2cWrite, reg, address, !failed, data);
        if (!failed)
        {
            simulation.registersValue[registerKey(address, reg)] = data;
        }
        return !failed;
    }

    bytevector XWalkRobotHatSimulation::read(contextpointer context, uint8 address, size length)
    {
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::I2cRead, address);
        if (failed || (simulation.presentAddressesValue.count(address) == 0U))
        {
            simulation.record(XWalkRobotHatOperation::I2cRead, address, static_cast<uint32>(length), false);
            XWALK_HAL_ERROR(XWALK_RUNTIME, "injected or unavailable Robot HAT I2C read");
        }
        bytevector result(length, 0U);
        const auto command = simulation.adcCommandValue.find(address);
        if ((command != simulation.adcCommandValue.end()) && (length >= 2U))
        {
            const uint8 channel = static_cast<uint8>(7U - (command->second & 0x07U));
            const uint16 value = simulation.adcValuesValue[channel];
            result[0U] = static_cast<uint8>((value >> 8U) & 0xFFU);
            result[1U] = static_cast<uint8>(value & 0xFFU);
        }
        simulation.record(XWalkRobotHatOperation::I2cRead, address, static_cast<uint32>(length), true, result);
        return result;
    }

    bytevector XWalkRobotHatSimulation::readRegister(contextpointer context, uint8 address, uint8 reg, size length)
    {
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::I2cReadRegister, reg);
        if (failed)
        {
            simulation.record(XWalkRobotHatOperation::I2cReadRegister, reg, static_cast<uint32>(length), false);
            XWALK_HAL_ERROR(XWALK_RUNTIME, "injected Robot HAT I2C register read failure");
        }
        bytevector result(length, 0U);
        for (size index = 0U; index < length; ++index)
        {
            const uint8 current = static_cast<uint8>(reg + static_cast<uint8>(index));
            const auto value = simulation.registersValue.find(registerKey(address, current));
            if ((value != simulation.registersValue.end()) && !value->second.empty())
            {
                result[index] = value->second[0U];
            }
        }
        simulation.record(XWalkRobotHatOperation::I2cReadRegister, reg, static_cast<uint32>(length), true, result);
        return result;
    }

    void XWalkRobotHatSimulation::configureGpio(
        contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        static_cast<void>(pull);
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::GpioConfigure, pin);
        simulation.record(XWalkRobotHatOperation::GpioConfigure, pin, static_cast<uint32>(mode), !failed);
        if (failed)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "injected Robot HAT GPIO configure failure");
        }
        simulation.gpioModesValue[pin] = mode;
        if (mode == XWalkGpioMode::Output)
        {
            simulation.gpioValuesValue[pin] = initialValue;
        }
    }

    boolean XWalkRobotHatSimulation::readGpio(contextpointer context, uint8 pin)
    {
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::GpioRead, pin);
        simulation.record(XWalkRobotHatOperation::GpioRead, pin, simulation.gpioValuesValue[pin], !failed);
        if (failed)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "injected Robot HAT GPIO read failure");
        }
        return simulation.gpioValuesValue[pin];
    }

    void XWalkRobotHatSimulation::writeGpio(contextpointer context, uint8 pin, boolean value)
    {
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::GpioWrite, pin);
        simulation.record(XWalkRobotHatOperation::GpioWrite, pin, value, !failed);
        if (failed)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "injected Robot HAT GPIO write failure");
        }
        simulation.gpioValuesValue[pin] = value;
    }

    void XWalkRobotHatSimulation::interruptGpio(contextpointer context,
                                                uint8 pin,
                                                XWalkGpioEdge edge,
                                                uint32 debounceMs,
                                                contextpointer handlerContext,
                                                gpiointerrupthandler handler)
    {
        static_cast<void>(edge);
        static_cast<void>(handlerContext);
        static_cast<void>(handler);
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::GpioInterrupt, pin);
        simulation.record(XWalkRobotHatOperation::GpioInterrupt, pin, debounceMs, !failed);
        if (failed)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "injected Robot HAT GPIO interrupt failure");
        }
    }

    void XWalkRobotHatSimulation::cancelInterruptGpio(contextpointer context, uint8 pin)
    {
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed = simulation.consumeFailure(XWalkRobotHatOperation::GpioCancelInterrupt, pin);
        simulation.record(XWalkRobotHatOperation::GpioCancelInterrupt, pin, 0U, !failed);
        if (failed)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "injected Robot HAT GPIO interrupt cancellation failure");
        }
    }

    boolean XWalkRobotHatSimulation::capture(contextpointer context,
                                             stringview outputPath,
                                             const XWalkCameraConfiguration& configuration)
    {
        XWalkRobotHatSimulation& simulation = from(context);
        const std::lock_guard<std::mutex> lock(simulation.mutexValue);
        const boolean failed =
            simulation.consumeFailure(XWalkRobotHatOperation::CameraCapture, 0U) || !simulation.cameraAvailableValue;
        simulation.record(
            XWalkRobotHatOperation::CameraCapture, 0U, configuration.widthPixels, !failed, {}, outputPath);
        return !failed;
    }

} /* namespace xwalk::hal::simulation */
