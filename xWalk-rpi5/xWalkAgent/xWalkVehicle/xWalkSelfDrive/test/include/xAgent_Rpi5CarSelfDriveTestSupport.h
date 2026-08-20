/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveTestSupport.h
 * @brief       Declares reusable SelfDrive host-test state and callbacks.
 * @project     xWalk Firmware
 * @module      xWalkSelfDrive Host Test Support
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_SELF_DRIVE_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_SELF_DRIVE_TEST_SUPPORT_H

#include "xAgent_Rpi5CarSelfDrive.h"
#include "xHal_Rpi5CarI2c.h"

namespace xwalk::agent::test::selfdrive
{

    /** @brief Provides deterministic I2C samples and accepts register writes. */
    struct TestBus
    {
            agent::bytevector sample{0x03U, 0xE8U};
    };

    /** @brief Stores one simulated GPIO level. */
    struct TestGpio
    {
            agent::boolean value{};
    };

    /** @brief Records deterministic timing, audio, cancellation, and motor state. */
    struct TestBackend
    {
            agent::uint32vector delays{};
            agent::stringvector backgroundFiles{};
            agent::float64vector backgroundVolumes{};
            agent::string musicFile{};
            agent::int32 musicLoops{};
            agent::float64 musicStartSeconds{};
            agent::float64 musicVolume{};
            agent::uint32 musicControlCount{};
            agent::boolean outputEnabled{};
            agent::uint32 continueQueries{};
            agent::uint32 continueQueryLimit{1'000'000U};
            hal::XWalkMotors* motors{nullptr};
            agent::boolean failDelayWhileMoving{};
            agent::boolean expireWatchdogWhileMoving{};
            agent::uint64 clockMs{};
    };

    agent::boolean probe(agent::contextpointer context, agent::uint8 address);
    void
    writeRegister(agent::contextpointer context, agent::uint8 address, agent::uint8 reg, const agent::bytevector& data);
    agent::boolean tryWriteRegister(agent::contextpointer context,
                                    agent::uint8 address,
                                    agent::uint8 reg,
                                    const agent::bytevector& data) noexcept;
    agent::bytevector readBus(agent::contextpointer context, agent::uint8 address, agent::size length);
    void configureGpio(agent::contextpointer context,
                       agent::uint8 pin,
                       XWalkHal::XWalkGpioMode mode,
                       XWalkHal::XWalkGpioPull pull,
                       agent::boolean initialValue);
    agent::boolean readGpio(agent::contextpointer context, agent::uint8 pin);
    void writeGpio(agent::contextpointer context, agent::uint8 pin, agent::boolean value);
    void interruptGpio(agent::contextpointer context,
                       agent::uint8 pin,
                       XWalkHal::XWalkGpioEdge edge,
                       agent::uint32 debounceMs,
                       agent::contextpointer handlerContext,
                       XWalkHal::gpiointerrupthandler handler);
    void cancelInterrupt(agent::contextpointer context, agent::uint8 pin);
    XWalkHal::XWalkGpioCallbacks gpioCallbacks();
    void enableOutput(agent::contextpointer context);
    void playSound(agent::contextpointer context, agent::stringview filename, agent::optionalfloat64 volume);
    void playSoundBackground(agent::contextpointer context, agent::stringview filename, agent::optionalfloat64 volume);
    void playMusic(agent::contextpointer context,
                   agent::stringview filename,
                   agent::int32 loops,
                   agent::float64 startSeconds);
    void setMusicVolume(agent::contextpointer context, agent::float64 volume);
    void controlMusic(agent::contextpointer context);
    agent::float64 soundLength(agent::contextpointer context, agent::stringview filename);
    void playTone(agent::contextpointer context,
                  const agent::bytevector& data,
                  agent::uint32 sampleRateHz,
                  agent::uint8 channelCount);
    agent::boolean delayMilliseconds(agent::contextpointer context, agent::uint32 durationMs) noexcept;
    agent::boolean continueOperation(agent::contextpointer context);
    agent::uint64 clockMilliseconds(agent::contextpointer context) noexcept;

} /* namespace xwalk::agent::test::selfdrive */

#endif /* XAGENT_RPI5CAR_SELF_DRIVE_TEST_SUPPORT_H */
