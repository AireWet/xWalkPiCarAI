/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveTestSupport.cpp
 * @brief       Implements reusable SelfDrive host-test callbacks.
 * @project     xWalk Firmware
 * @module      xWalkSelfDrive Host Test Support
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarSelfDriveTestSupport.h"

namespace xwalk::agent::test::selfdrive
{

    agent::boolean probe(agent::contextpointer context, agent::uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }

    void
    writeRegister(agent::contextpointer context, agent::uint8 address, agent::uint8 reg, const agent::bytevector& data)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
    }

    agent::boolean tryWriteRegister(agent::contextpointer context,
                                    agent::uint8 address,
                                    agent::uint8 reg,
                                    const agent::bytevector& data) noexcept
    {
        writeRegister(context, address, reg, data);
        return true;
    }

    agent::bytevector readBus(agent::contextpointer context, agent::uint8 address, agent::size length)
    {
        static_cast<void>(address);
        static_cast<void>(length);
        return static_cast<TestBus*>(context)->sample;
    }

    void configureGpio(agent::contextpointer context,
                       agent::uint8 pin,
                       XWalkHal::XWalkGpioMode mode,
                       XWalkHal::XWalkGpioPull pull,
                       agent::boolean initialValue)
    {
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
        static_cast<TestGpio*>(context)->value = initialValue;
    }

    agent::boolean readGpio(agent::contextpointer context, agent::uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<TestGpio*>(context)->value;
    }

    void writeGpio(agent::contextpointer context, agent::uint8 pin, agent::boolean value)
    {
        static_cast<void>(pin);
        static_cast<TestGpio*>(context)->value = value;
    }

    void interruptGpio(agent::contextpointer context,
                       agent::uint8 pin,
                       XWalkHal::XWalkGpioEdge edge,
                       agent::uint32 debounceMs,
                       agent::contextpointer handlerContext,
                       XWalkHal::gpiointerrupthandler handler)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(edge);
        static_cast<void>(debounceMs);
        static_cast<void>(handlerContext);
        static_cast<void>(handler);
    }

    void cancelInterrupt(agent::contextpointer context, agent::uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }

    XWalkHal::XWalkGpioCallbacks gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelInterrupt};
    }

    void enableOutput(agent::contextpointer context)
    {
        static_cast<TestBackend*>(context)->outputEnabled = true;
    }

    void playSound(agent::contextpointer context, agent::stringview filename, agent::optionalfloat64 volume)
    {
        static_cast<void>(context);
        static_cast<void>(filename);
        static_cast<void>(volume);
    }

    void playSoundBackground(agent::contextpointer context, agent::stringview filename, agent::optionalfloat64 volume)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        backend.backgroundFiles.emplace_back(filename);
        backend.backgroundVolumes.push_back(volume.has_value() ? *volume : -1.0);
    }

    void playMusic(agent::contextpointer context,
                   agent::stringview filename,
                   agent::int32 loops,
                   agent::float64 startSeconds)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        backend.musicFile = filename;
        backend.musicLoops = loops;
        backend.musicStartSeconds = startSeconds;
    }

    void setMusicVolume(agent::contextpointer context, agent::float64 volume)
    {
        static_cast<TestBackend*>(context)->musicVolume = volume;
    }

    void controlMusic(agent::contextpointer context)
    {
        ++static_cast<TestBackend*>(context)->musicControlCount;
    }

    agent::float64 soundLength(agent::contextpointer context, agent::stringview filename)
    {
        static_cast<void>(context);
        static_cast<void>(filename);
        return 0.0;
    }

    void playTone(agent::contextpointer context,
                  const agent::bytevector& data,
                  agent::uint32 sampleRateHz,
                  agent::uint8 channelCount)
    {
        static_cast<void>(context);
        static_cast<void>(data);
        static_cast<void>(sampleRateHz);
        static_cast<void>(channelCount);
    }

    agent::boolean delayMilliseconds(agent::contextpointer context, agent::uint32 durationMs) noexcept
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        backend.delays.push_back(durationMs);
        const agent::boolean motorsMoving = static_cast<agent::boolean>((backend.motors->left().speed() != 0.0) ||
                                                                        (backend.motors->right().speed() != 0.0));
        if (backend.failDelayWhileMoving && motorsMoving)
        {
            return false;
        }
        backend.clockMs += durationMs;
        if (backend.expireWatchdogWhileMoving && motorsMoving)
        {
            backend.clockMs += 500U;
            backend.expireWatchdogWhileMoving = false;
        }
        static_cast<void>(backend.motors->checkWatchdog());
        return true;
    }

    agent::boolean continueOperation(agent::contextpointer context)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        const agent::boolean shouldContinue = backend.continueQueries < backend.continueQueryLimit;
        ++backend.continueQueries;
        return shouldContinue;
    }

    agent::uint64 clockMilliseconds(agent::contextpointer context) noexcept
    {
        return static_cast<TestBackend*>(context)->clockMs;
    }

} /* namespace xwalk::agent::test::selfdrive */
