/******************************************************************************
 * @file        xHal_Rpi5CarRobotHatSimulation.h
 * @brief       Declares the deterministic, device-free Robot HAT simulator.
 * @project     xWalk Firmware
 * @module      xWalkRobotHatSimulation
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_ROBOT_HAT_SIMULATION_H
#define XHAL_RPI5CAR_ROBOT_HAT_SIMULATION_H

#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarI2c.h"

#include <array>
#include <map>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

namespace xwalk::hal::simulation
{

    /** @brief Identifies one externally visible simulator operation. */
    enum class XWalkRobotHatOperation : uint8
    {
        I2cProbe,
        I2cWrite,
        I2cRead,
        I2cReadRegister,
        GpioConfigure,
        GpioRead,
        GpioWrite,
        GpioInterrupt,
        GpioCancelInterrupt,
        CameraCapture
    };

    /** @brief Records one simulator operation in deterministic execution order. */
    struct XWalkRobotHatEvent
    {
            uint64 sequence{};    /**< Contiguous operation sequence number, beginning at one. */
            uint64 logicalTime{}; /**< Deterministic logical time after configured delay application. */
            XWalkRobotHatOperation operation{XWalkRobotHatOperation::I2cProbe}; /**< Recorded operation. */
            uint32 target{};         /**< Address, register, GPIO line, or zero when not applicable. */
            uint32 value{};          /**< Secondary numeric argument or logical output value. */
            boolean succeeded{true}; /**< Whether the configured operation succeeded. */
            bytevector data{};       /**< Owned I2C payload or returned bytes. */
            string text{};           /**< Owned path or diagnostic detail. */
    };

    /**
     * @brief Implements deterministic callbacks for the production I2C, GPIO, and camera interfaces.
     * @details No callback opens a device, starts a process, sleeps, or reads wall-clock time.
     */
    class XWalkRobotHatSimulation final
    {
        private:
            struct Failure
            {
                    XWalkRobotHatOperation operation{XWalkRobotHatOperation::I2cProbe};
                    uint32 target{};
                    uint32 remaining{};
            };

            mutable std::mutex mutexValue{};
            uint64 nextSequenceValue{1U};
            uint64 logicalTimeValue{};
            std::set<uint8> presentAddressesValue{0x14U};
            std::map<uint16, bytevector> registersValue{};
            std::map<uint8, uint8> adcCommandValue{};
            std::array<uint16, 8U> adcValuesValue{};
            std::array<boolean, 256U> gpioValuesValue{};
            std::array<XWalkGpioMode, 256U> gpioModesValue{};
            std::vector<XWalkRobotHatEvent> eventsValue{};
            std::vector<Failure> failuresValue{};
            std::map<std::pair<XWalkRobotHatOperation, uint32>, uint32> delaysValue{};
            boolean cameraAvailableValue{true};
            std::vector<bytevector> cameraFramesValue{};
            size nextCameraFrameValue{};
            float64 ultrasonicDistanceCentimetersValue{100.0};

            static XWalkRobotHatSimulation& from(contextpointer context);
            static uint16 registerKey(uint8 address, uint8 reg) noexcept;
            boolean consumeFailure(XWalkRobotHatOperation operation, uint32 target) noexcept;
            void record(XWalkRobotHatOperation operation,
                        uint32 target,
                        uint32 value,
                        boolean succeeded,
                        const bytevector& data = {},
                        stringview text = {});

        public:
            XWalkRobotHatSimulation() = default;
            ~XWalkRobotHatSimulation() = default;
            XWalkRobotHatSimulation(const XWalkRobotHatSimulation&) = delete;
            XWalkRobotHatSimulation& operator=(const XWalkRobotHatSimulation&) = delete;
            XWalkRobotHatSimulation(XWalkRobotHatSimulation&&) = delete;
            XWalkRobotHatSimulation& operator=(XWalkRobotHatSimulation&&) = delete;

            /** @brief Returns callbacks suitable for constructing XWalkGpio objects. */
            static XWalkGpioCallbacks gpioCallbacks() noexcept;
            /** @brief Adds or removes one responding seven-bit I2C address. */
            void setI2cPresent(uint8 address, boolean present);
            /** @brief Sets one deterministic twelve-bit ADC sample. */
            void setAdcValue(uint8 channel, uint16 value);
            /** @brief Sets the conventional left, center, and right grayscale ADC samples A0 through A2. */
            void setGrayscaleValues(const std::array<uint16, 3U>& values);
            /** @brief Converts a zero-to-9.9-volt Robot HAT battery estimate to the A4 ADC sample. */
            void setBatteryVoltage(float64 voltage);
            /** @brief Sets a deterministic ultrasonic distance without performing GPIO timing. */
            void setUltrasonicDistance(float64 distanceCentimeters);
            /** @brief Returns the configured deterministic ultrasonic distance. */
            float64 ultrasonicDistance() const noexcept;
            /** @brief Sets a physical GPIO input or output level. */
            void setGpioValue(uint8 pin, boolean value);
            /** @brief Controls whether simulated camera capture succeeds. */
            void setCameraAvailable(boolean available) noexcept;
            /** @brief Replaces the deterministic sequence of non-empty encoded simulated frames. */
            void setCameraFrames(const std::vector<bytevector>& frames);
            /** @brief Returns the next simulated frame, or `false` at normal end-of-sequence. */
            boolean nextCameraFrame(bytevector& frame);
            /** @brief Configures logical delay ticks for one operation and target without sleeping. */
            void setLogicalDelay(XWalkRobotHatOperation operation, uint32 target, uint32 ticks);
            /** @brief Makes the next matching operations fail. */
            void failNext(XWalkRobotHatOperation operation, uint32 target, uint32 count = 1U);
            /** @brief Removes all pending injected failures. */
            void clearFailures() noexcept;
            /** @brief Returns a stable copy of recorded events. */
            std::vector<XWalkRobotHatEvent> events() const;
            /** @brief Clears recorded events and restarts logical timestamps at one. */
            void clearEvents() noexcept;
            /** @brief Returns the last payload written to an I2C register. */
            bytevector registerValue(uint8 address, uint8 reg) const;
            /** @brief Returns one physical GPIO level. */
            boolean gpioValue(uint8 pin) const;

            static boolean probe(contextpointer context, uint8 address);
            static void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
            static boolean
            tryWriteRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data) noexcept;
            static bytevector read(contextpointer context, uint8 address, size length);
            static bytevector readRegister(contextpointer context, uint8 address, uint8 reg, size length);
            static void configureGpio(
                contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue);
            static boolean readGpio(contextpointer context, uint8 pin);
            static void writeGpio(contextpointer context, uint8 pin, boolean value);
            static void interruptGpio(contextpointer context,
                                      uint8 pin,
                                      XWalkGpioEdge edge,
                                      uint32 debounceMs,
                                      contextpointer handlerContext,
                                      gpiointerrupthandler handler);
            static void cancelInterruptGpio(contextpointer context, uint8 pin);
            static boolean
            capture(contextpointer context, stringview outputPath, const XWalkCameraConfiguration& configuration);
    };

} /* namespace xwalk::hal::simulation */

#endif /* XHAL_RPI5CAR_ROBOT_HAT_SIMULATION_H */
