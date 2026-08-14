/******************************************************************************
 * @file        xHal_Rpi5CarGptHostStub.h
 * @brief       Declares device-free xWalkGPT simulation callbacks.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_GPT_HOST_STUB_H
#define XHAL_RPI5CAR_GPT_HOST_STUB_H
#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarTextToSpeech.h"
namespace xwalk::hal::sim
{
/** @brief Simulates recognition, speech output, GPIO, I2C, and speaker priming. */
class XWalkGptHostStub final
{
    private:
        boolean gpioValueValue{}; uint32 primeCountValue{};
        uint32 spokenCountValue{}; uint32 stopCountValue{};
    public:
        static boolean ready(contextpointer context);
        static string listen(contextpointer context, uint32 timeoutMs);
        static string transcribe(contextpointer context, stringview path);
        static void stop(contextpointer context);
        static void speak(contextpointer context, stringview text);
        static void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
            XWalkGpioPull pull, boolean initialValue);
        static boolean readGpio(contextpointer context, uint8 pin);
        static void writeGpio(contextpointer context, uint8 pin, boolean value);
        static void interruptGpio(contextpointer context, uint8 pin, XWalkGpioEdge edge,
            uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler);
        static void cancelGpio(contextpointer context, uint8 pin);
        static boolean probeI2c(contextpointer context, uint8 address);
        static void writeI2c(contextpointer context, uint8 address, uint8 reg,
            const bytevector& data);
        static bytevector readI2c(contextpointer context, uint8 address, size length);
        static void primeSpeaker(contextpointer context, uint32 durationMs);
        static XWalkSpeechToTextCallbacks recognitionCallbacks();
        static XWalkGpioCallbacks gpioCallbacks();
        boolean gpioValue() const noexcept;
        uint32 primeCount() const noexcept;
        uint32 spokenCount() const noexcept;
        uint32 stopCount() const noexcept;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_GPT_HOST_STUB_H */
