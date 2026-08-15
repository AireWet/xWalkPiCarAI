/******************************************************************************
 * @file        xHal_Rpi5CarGptHostStub.cpp
 * @brief       Implements device-free xWalkGPT simulation callbacks.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarGptHostStub.h"
namespace xwalk::hal::sim
{
    boolean XWalkGptHostStub::ready(contextpointer context)
    {
        static_cast<void>(context);
        return true;
    }
    string XWalkGptHostStub::listen(contextpointer context, uint32 timeoutMs)
    {
        static_cast<void>(context);
        static_cast<void>(timeoutMs);
        return "simulated recognition";
    }
    string XWalkGptHostStub::transcribe(contextpointer context, stringview path)
    {
        static_cast<void>(context);
        static_cast<void>(path);
        return "simulated transcription";
    }
    void XWalkGptHostStub::stop(contextpointer context)
    {
        ++static_cast<XWalkGptHostStub*>(context)->stopCountValue;
    }
    void XWalkGptHostStub::speak(contextpointer context, stringview text)
    {
        static_cast<void>(text);
        ++static_cast<XWalkGptHostStub*>(context)->spokenCountValue;
    }
    void XWalkGptHostStub::configureGpio(
        contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        static_cast<XWalkGptHostStub*>(context)->gpioValueValue = initialValue;
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
    }
    boolean XWalkGptHostStub::readGpio(contextpointer context, uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<XWalkGptHostStub*>(context)->gpioValueValue;
    }
    void XWalkGptHostStub::writeGpio(contextpointer context, uint8 pin, boolean value)
    {
        static_cast<void>(pin);
        static_cast<XWalkGptHostStub*>(context)->gpioValueValue = value;
    }
    void XWalkGptHostStub::interruptGpio(contextpointer context,
                                         uint8 pin,
                                         XWalkGpioEdge edge,
                                         uint32 debounceMs,
                                         contextpointer handlerContext,
                                         gpiointerrupthandler handler)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(edge);
        static_cast<void>(debounceMs);
        static_cast<void>(handlerContext);
        static_cast<void>(handler);
    }
    void XWalkGptHostStub::cancelGpio(contextpointer context, uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }
    boolean XWalkGptHostStub::probeI2c(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }
    void XWalkGptHostStub::writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
    }
    bytevector XWalkGptHostStub::readI2c(contextpointer context, uint8 address, size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return bytevector(length, 0U);
    }
    void XWalkGptHostStub::primeSpeaker(contextpointer context, uint32 durationMs)
    {
        if (durationMs == XHAL_RPI5CAR_BOARD_CONTROL_SPEAKER_PRIME_DURATION_MS)
        {
            ++static_cast<XWalkGptHostStub*>(context)->primeCountValue;
        }
    }
    XWalkSpeechToTextCallbacks XWalkGptHostStub::recognitionCallbacks()
    {
        return {&ready, &listen, &transcribe, &stop};
    }
    XWalkGpioCallbacks XWalkGptHostStub::gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelGpio};
    }
    boolean XWalkGptHostStub::gpioValue() const noexcept
    {
        return gpioValueValue;
    }
    uint32 XWalkGptHostStub::primeCount() const noexcept
    {
        return primeCountValue;
    }
    uint32 XWalkGptHostStub::spokenCount() const noexcept
    {
        return spokenCountValue;
    }
    uint32 XWalkGptHostStub::stopCount() const noexcept
    {
        return stopCountValue;
    }
} /* namespace xwalk::hal::sim */
