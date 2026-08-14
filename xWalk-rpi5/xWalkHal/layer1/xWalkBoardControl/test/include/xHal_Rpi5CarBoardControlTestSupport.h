/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlTestSupport.h
 * @brief       Declares reusable xWalkBoardControl host-test support.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_BOARD_CONTROL_TEST_SUPPORT_H
#define XHAL_RPI5CAR_BOARD_CONTROL_TEST_SUPPORT_H
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarDevice.h"
#include "xHal_Rpi5CarFirmwareInfo.h"
namespace xwalk::hal::test::boardcontrol
{
/** @brief Records GPIO configuration and bounded logical output writes. */
struct TestGpioBackend
{
    uint8 pin{};
    boolean physicalValue{};
    fixedarray<boolean, 8U> writes{};
    size writeCount{};
};
/** @brief Supplies deterministic ADC bytes through an in-memory I2C adapter. */
struct TestI2cBackend
{
    bytevector sampleBytes{0x0FU, 0xFFU};
};
/** @brief Records speaker priming and optionally injects failure. */
struct TestSpeakerPrime
{
    uint32 durationMs{};
    uint32 callCount{};
    boolean fail{};
};
/** @brief Supplies deterministic firmware probe and register-read responses. */
struct TestFirmwareBus
{
    byteset respondingAddresses{};
    bytevector probes{};
    bytevector versionBytes{2U, 5U, 5U};
    uint8 readAddress{};
    uint8 readRegister{};
    size readLength{};
};
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue);
boolean readGpio(contextpointer context, uint8 pin);
void writeGpio(contextpointer context, uint8 pin, boolean value);
void registerInterrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler);
void cancelInterrupt(contextpointer context, uint8 pin);
XWalkGpioCallbacks gpioCallbacks();
boolean probeI2c(contextpointer context, uint8 address);
void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
bytevector readI2c(contextpointer context, uint8 address, size length);
void primeSpeaker(contextpointer context, uint32 durationMs);
boolean probeFirmware(contextpointer context, uint8 address);
void writeFirmware(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data);
bytevector readFirmware(contextpointer context, uint8 address, size length);
bytevector readFirmwareRegister(contextpointer context, uint8 address, uint8 reg,
    size length);
void writeProperty(const filesystempath& path, stringview value, boolean appendNull);
void createSupportedNode(const filesystempath& nodePath);
void removeNode(const filesystempath& nodePath);
} /* namespace xwalk::hal::test::boardcontrol */
#endif /* XHAL_RPI5CAR_BOARD_CONTROL_TEST_SUPPORT_H */
