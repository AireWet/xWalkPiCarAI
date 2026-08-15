/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlTestSupport.cpp
 * @brief       Implements reusable xWalkBoardControl host-test support.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBoardControlTestSupport.h"
#include "xHal_Rpi5CarTrace.h"
#include <cassert>
namespace xwalk::hal::test::boardcontrol
{
    void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        TestGpioBackend& backend = *static_cast<TestGpioBackend*>(context);
        backend.pin = pin;
        backend.physicalValue = initialValue;
        static_cast<void>(mode);
        static_cast<void>(pull);
    }
    boolean readGpio(contextpointer context, uint8 pin)
    {
        TestGpioBackend& backend = *static_cast<TestGpioBackend*>(context);
        assert(pin == backend.pin);
        return backend.physicalValue;
    }
    void writeGpio(contextpointer context, uint8 pin, boolean value)
    {
        TestGpioBackend& backend = *static_cast<TestGpioBackend*>(context);
        assert(pin == backend.pin);
        assert(backend.writeCount < backend.writes.size());
        backend.physicalValue = value;
        backend.writes[backend.writeCount] = value;
        ++backend.writeCount;
    }
    void registerInterrupt(contextpointer context,
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
    void cancelInterrupt(contextpointer context, uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }
    XWalkGpioCallbacks gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
    }
    boolean probeI2c(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }
    void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
    }
    bytevector readI2c(contextpointer context, uint8 address, size length)
    {
        TestI2cBackend& backend = *static_cast<TestI2cBackend*>(context);
        static_cast<void>(address);
        assert(length == XHAL_RPI5CAR_ADC_READ_LENGTH);
        return backend.sampleBytes;
    }
    void primeSpeaker(contextpointer context, uint32 durationMs)
    {
        TestSpeakerPrime& state = *static_cast<TestSpeakerPrime*>(context);
        state.durationMs = durationMs;
        ++state.callCount;
        if (state.fail)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Test speaker priming failed");
        }
    }
    boolean probeFirmware(contextpointer context, uint8 address)
    {
        TestFirmwareBus& bus = *static_cast<TestFirmwareBus*>(context);
        bus.probes.push_back(address);
        return bus.respondingAddresses.count(address) != 0U;
    }
    void writeFirmware(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
    }
    bytevector readFirmware(contextpointer context, uint8 address, size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(length);
        return {};
    }
    bytevector readFirmwareRegister(contextpointer context, uint8 address, uint8 reg, size length)
    {
        TestFirmwareBus& bus = *static_cast<TestFirmwareBus*>(context);
        bus.readAddress = address;
        bus.readRegister = reg;
        bus.readLength = length;
        return bus.versionBytes;
    }
    void writeProperty(const filesystempath& path, stringview value, boolean appendNull)
    {
        outputfilestream file(path, FILE_OPEN_WRITE_TRUNCATE);
        assert(file.is_open());
        file << string(value);
        if (appendNull)
        {
            file.put('\0');
        }
        file.close();
        assert(!file.fail());
    }
    void createSupportedNode(const filesystempath& nodePath)
    {
        static_cast<void>(createDirectories(nodePath));
        writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_PROPERTY, "Robot HAT 5", false);
        writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY, "00001902", true);
        writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_VERSION_PROPERTY, "0x00000050", true);
        writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY, XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID, true);
        writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY, "SunFounder", false);
    }
    void removeNode(const filesystempath& nodePath)
    {
        static_cast<void>(removeFilesystemEntry(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_PROPERTY));
        static_cast<void>(removeFilesystemEntry(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY));
        static_cast<void>(removeFilesystemEntry(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_VERSION_PROPERTY));
        static_cast<void>(removeFilesystemEntry(nodePath / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY));
        static_cast<void>(removeFilesystemEntry(nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY));
        static_cast<void>(removeFilesystemEntry(nodePath));
    }
} /* namespace xwalk::hal::test::boardcontrol */
