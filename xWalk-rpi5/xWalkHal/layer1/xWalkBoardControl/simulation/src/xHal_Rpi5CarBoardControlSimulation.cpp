/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlSimulation.cpp
 * @brief       Implements the device-free xWalkBoardControl simulation.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBoardControlSimulation.h"
#include "xHal_Rpi5CarBoardControlHostStub.h"
#include "xHal_Rpi5CarBoardControlSimulationConfig.h"
#include "xHal_Rpi5CarDevice.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    void writeSimulationProperty(const filesystempath& path, stringview value, boolean appendNull)
    {
        outputfilestream file(path, FILE_OPEN_WRITE_TRUNCATE);
        if (!file.is_open())
        {
            XWALK_HAL_ERROR(XWALK_EXCEPTION, "Simulation property could not be opened");
        }
        file << string(value);
        if (appendNull)
        {
            file.put('\0');
        }
    }
    void createSimulationDeviceTree(const filesystempath& root, const filesystempath& node)
    {
        static_cast<void>(createDirectories(node));
        writeSimulationProperty(node / XHAL_RPI5CAR_DEVICE_PRODUCT_PROPERTY, "Robot HAT 5", false);
        writeSimulationProperty(node / XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY, "00001902", true);
        writeSimulationProperty(node / XHAL_RPI5CAR_DEVICE_PRODUCT_VERSION_PROPERTY, "0x00000050", true);
        writeSimulationProperty(node / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY, XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID, true);
        writeSimulationProperty(node / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY, "SunFounder", false);
        static_cast<void>(root);
    }
    void removeSimulationDeviceTree(const filesystempath& root, const filesystempath& node)
    {
        static_cast<void>(removeFilesystemEntry(node / XHAL_RPI5CAR_DEVICE_PRODUCT_PROPERTY));
        static_cast<void>(removeFilesystemEntry(node / XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY));
        static_cast<void>(removeFilesystemEntry(node / XHAL_RPI5CAR_DEVICE_PRODUCT_VERSION_PROPERTY));
        static_cast<void>(removeFilesystemEntry(node / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY));
        static_cast<void>(removeFilesystemEntry(node / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY));
        static_cast<void>(removeFilesystemEntry(node));
        static_cast<void>(removeFilesystemEntry(root));
    }
    int32 runBoardControlSimulation()
    {
        XWalkBoardControlHostStub resetBackend;
        XWalkBoardControlHostStub speakerBackend;
        XWalkBoardControlHostStub busBackend;
        const XWalkGpioCallbacks callbacks = XWalkBoardControlHostStub::gpioCallbacks();
        XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
        XWalkGpio speakerGpio(&speakerBackend, callbacks, XHAL_RPI5CAR_DEVICE_V5_SPEAKER_ENABLE_PIN);
        XWalkI2c i2c(&busBackend,
                     &XWalkBoardControlHostStub::probeI2c,
                     &XWalkBoardControlHostStub::writeI2c,
                     &XWalkBoardControlHostStub::readI2c,
                     &XWalkBoardControlHostStub::readRegisterI2c);
        XWalkAdc adc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkBoardControl control(
            resetGpio, speakerGpio, adc, &speakerBackend, &XWalkBoardControlHostStub::primeSpeaker);
        control.resetMcu();
        const float64 voltage = control.batteryVoltage();
        control.enableSpeaker();
        control.disableSpeaker();
        XWalkFirmwareInfo firmware(i2c);
        const XWalkFirmwareVersion version = firmware.read();
        const filesystempath root(XWALK_BOARD_CONTROL_SIMULATION_DEVICE_TREE_PATH);
        const filesystempath node = root / "hat-simulation";
        createSimulationDeviceTree(root, node);
        const XWalkDevice device(root.string());
        const boolean detected = device.information().detected;
        removeSimulationDeviceTree(root, node);
        const boolean valid = resetBackend.gpioValue() && !speakerBackend.gpioValue() &&
                              (speakerBackend.primeCount() == 1U) && (voltage > 9.8) && (voltage < 10.0) &&
                              (version.major == 2U) && (version.minor == 5U) && (version.patch == 5U) && detected;
        XWALK_HAL_TRACE_UID0(RPI .330, "xWalkBoardControl host simulation completed");
        return valid ? 0 : 1;
    }
} /* namespace xwalk::hal::sim */
