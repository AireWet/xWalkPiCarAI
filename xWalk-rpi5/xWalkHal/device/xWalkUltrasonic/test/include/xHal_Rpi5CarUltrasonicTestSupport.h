/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicTestSupport.h
 * @brief       Declares reusable ultrasonic GPIO host-test support.
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ULTRASONIC_TEST_SUPPORT_H
#define XHAL_RPI5CAR_ULTRASONIC_TEST_SUPPORT_H
#include "xHal_Rpi5CarGpio.h"
namespace xwalk::hal::test::ultrasonic
{
/** @brief Selects the echo waveform returned by the simulated input pin. */
enum class EchoBehavior : uint8
{
    Pulse = 0U,
    Timeout = 1U,
    Invalid = 2U,
    TimeoutThenInvalid = 3U
};

/** @brief Stores simulated trigger traffic and echo waveform state. */
struct TestBackend
{
    EchoBehavior behavior{EchoBehavior::Pulse};
    uint32 pulseDelayMicroseconds{1'000U};
    uint32 triggerCount{};
    uint32 echoReadCount{};
    XWalkGpioMode triggerMode{XWalkGpioMode::Input};
    XWalkGpioMode echoMode{XWalkGpioMode::Output};
    XWalkGpioPull echoPull{XWalkGpioPull::None};
    bytevector triggerLevels;
};

void configure(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue);
boolean read(contextpointer context, uint8 pin);
void write(contextpointer context, uint8 pin, boolean value);
void interrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler);
void cancelInterrupt(contextpointer context, uint8 pin);
XWalkGpioCallbacks callbacks();
} /* namespace xwalk::hal::test::ultrasonic */
#endif /* XHAL_RPI5CAR_ULTRASONIC_TEST_SUPPORT_H */
