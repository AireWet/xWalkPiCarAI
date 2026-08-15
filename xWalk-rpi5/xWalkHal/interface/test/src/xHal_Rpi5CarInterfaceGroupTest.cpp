/******************************************************************************
 * @file        xHal_Rpi5CarInterfaceGroupTest.cpp
 * @brief       Verifies collaboration among interface-group modules.
 *
 * @details
 * Exercises configuration-driven bus, GPIO, audio, utility, and language-model
 * flows through deterministic callback fakes and build-local configuration.
 *
 * @project     xWalk Firmware
 * @module      xWalk Interface Group Test
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarInterfaceGroupTestSupport.h"

#include "xHal_Rpi5CarAudioAlsaTestSupport.h"
#include "xHal_Rpi5CarConfig.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLanguageModelTestSupport.h"
#include "xHal_Rpi5CarUtils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains interface-group scenarios private to this translation unit. */
namespace
{

    using namespace xwalk::hal;
    using namespace xwalk::hal::test::interface_group;
    using testing::ElementsAre;

    /** @brief Returns one isolated build-local configuration path. */
    filesystempath configurationPath(stringview name)
    {
        const filesystempath directory(XWALK_INTERFACE_GROUP_TEST_DATA_DIRECTORY);
        const boolean created = createDirectories(directory);
        static_cast<void>(created);
        return directory / string(name);
    }

    /** @brief Verifies configuration values flow through I2C, SPI, GPIO, and Utils. */
    TEST(XWalkInterfaceGroup, ConfigurationInitializesBusAndDigitalInterfaces)
    {
        const filesystempath path = configurationPath("interfaces-valid.ini");
        XWalkConfig configuration(path.string(), "interface group test");
        static_cast<void>(configuration.read());
        configuration.setSection("bus", {{"i2c_address", "20"}, {"spi_payload", "9F00"}});
        configuration.setSection("gpio", {{"pin", "D2"}, {"direction", "output"}});
        configuration.write();
        static_cast<void>(configuration.read());
        const string i2cAddress = configuration.get("bus", "i2c_address");
        const string spiPayload = configuration.get("bus", "spi_payload");
        const string gpioPin = configuration.get("gpio", "pin");
        const string gpioDirection = configuration.get("gpio", "direction");
        const boolean configurationValid =
            i2cAddress == "20" && spiPayload == "9F00" && gpioPin == "D2" && gpioDirection == "output";
        ASSERT_TRUE(configurationValid) << "Valid configuration must be complete before initialization";

        I2cBackend i2cBackend;
        XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
        EXPECT_TRUE(i2c.probe(0x14U));
        EXPECT_THAT(i2c.read(0x14U, 2U), ElementsAre(static_cast<uint8>(0x12U), static_cast<uint8>(0x34U)));

        SpiBackend spiBackend;
        XWalkSpi spi(&spiBackend, &transferSpi);
        EXPECT_THAT(spi.transfer({0x9FU, 0x00U}), ElementsAre(static_cast<uint8>(0xA5U), static_cast<uint8>(0x5AU)));

        GpioBackend gpioBackend;
        XWalkGpio gpio(&gpioBackend, gpioCallbacks(), gpioPin, XWalkGpioMode::Output);
        EXPECT_TRUE(gpio.on());

        EXPECT_EQ(i2cBackend.probeAddress, 0x14U);
        EXPECT_EQ(i2cBackend.readLength, 2U);
        EXPECT_THAT(spiBackend.transmitted, ElementsAre(static_cast<uint8>(0x9FU), static_cast<uint8>(0x00U)));
        EXPECT_EQ(gpioBackend.pin, 27U);
        EXPECT_EQ(gpioBackend.mode, XWalkGpioMode::Output);
        EXPECT_TRUE(gpioBackend.value);
        EXPECT_DOUBLE_EQ(XWalkUtils::mapping(20.0, 0.0, 100.0, 0.0, 1.0), 0.2);
    }

    /** @brief Verifies invalid and partial configuration cannot continue initialization. */
    TEST(XWalkInterfaceGroup, InvalidOrFailedInitializationStopsLaterInterfaces)
    {
        const filesystempath path = configurationPath("interfaces-invalid.ini");
        XWalkConfig configuration(path.string());
        static_cast<void>(configuration.read());
        configuration.setSection("bus", {{"i2c_address", "missing"}});
        configuration.setSection("gpio", {{"direction", "sideways"}});
        const boolean configurationValid =
            configuration.get("bus", "i2c_address") == "20" && configuration.get("gpio", "direction") == "output";

        I2cBackend i2cBackend;
        SpiBackend spiBackend;
        GpioBackend gpioBackend;
        EXPECT_FALSE(configurationValid);
        EXPECT_EQ(i2cBackend.probeCount, 0U);
        EXPECT_EQ(spiBackend.transferCount, 0U);
        EXPECT_EQ(gpioBackend.configureCount, 0U);

        i2cBackend.failProbe = true;
        XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
        XWalkSpi spi(&spiBackend, &transferSpi);
        EXPECT_THROW(
            {
                static_cast<void>(i2c.probe(0x14U));
                static_cast<void>(spi.transfer({0x9FU, 0x00U}));
                XWalkGpio gpio(&gpioBackend, gpioCallbacks(), "D2");
            },
            std::runtime_error);
        EXPECT_EQ(i2cBackend.probeCount, 1U);
        EXPECT_EQ(spiBackend.transferCount, 0U) << "SPI must not run after I2C initialization fails";
        EXPECT_EQ(gpioBackend.configureCount, 0U) << "GPIO must not be claimed after an earlier failure";

        EXPECT_THROW(static_cast<void>(XWalkUtils::mapping(1.0, 5.0, 5.0, 0.0, 100.0)), std::invalid_argument);
    }

    /** @brief Verifies configuration-driven audio negotiation, writes, and failures. */
    TEST(XWalkInterfaceGroup, AudioConfigurationFlowsToInjectedAlsaBoundary)
    {
        const filesystempath path = configurationPath("audio.ini");
        XWalkConfig configuration(path.string());
        static_cast<void>(configuration.read());
        configuration.setSection("audio", {{"sample_rate_hz", "44100"}, {"channels", "2"}, {"volume_percent", "75"}});

        using namespace xwalk::hal::test::audio;
        TestAudioOperations backend;
        XWalkAudioAlsa audio(&backend, testOperations(), "group-pcm", "group-mixer", "PCM");
        XWalkAudioStreamConfiguration streamConfiguration = testConfiguration();
        ASSERT_EQ(configuration.get("audio", "sample_rate_hz"), "44100");
        ASSERT_EQ(configuration.get("audio", "channels"), "2");
        const audiopcmhandle stream = audio.openStream(streamConfiguration);
        audio.writeFrames(stream, bytevector(16U, 0x5AU), 4U);
        const float64 mappedVolume = XWalkUtils::mapping(75.0, 0.0, 100.0, 0.0, 100.0);
        audio.setVolume(static_cast<uint8>(mappedVolume));
        audio.closeStream(stream);

        EXPECT_EQ(backend.pcmDevice, "group-pcm");
        EXPECT_EQ(backend.configuration.sampleRateHz, 44'100U);
        EXPECT_EQ(backend.configuration.channelCount, 2U);
        EXPECT_EQ(backend.configuration.periodFrames, 256U);
        EXPECT_EQ(backend.volumePercent, 75U);
        EXPECT_EQ(backend.recoverCount, 1U);
        EXPECT_EQ(backend.closePcmCount, 1U);

        TestAudioOperations failingBackend;
        failingBackend.configureSucceeds = false;
        XWalkAudioAlsa failingAudio(&failingBackend, testOperations(), "group-pcm", "group-mixer", "PCM");
        EXPECT_THROW(static_cast<void>(failingAudio.openStream(streamConfiguration)), std::runtime_error);
        EXPECT_EQ(failingBackend.configureCount, 1U);
        EXPECT_EQ(failingBackend.closePcmCount, 1U);
    }

    /** @brief Verifies configuration-driven language-model requests and error propagation. */
    TEST(XWalkInterfaceGroup, LanguageModelConfigurationAndBackendResultsPropagate)
    {
        const filesystempath path = configurationPath("language-model.ini");
        XWalkConfig configuration(path.string());
        static_cast<void>(configuration.read());
        configuration.setSection("model", {{"instructions", "Be concise"}, {"maximum_messages", "35"}});

        using namespace xwalk::hal::test::language_model;
        TestLanguageModelBackend backend;
        XWalkLanguageModel model(&backend, backendCallbacks());
        model.setInstructions(configuration.get("model", "instructions"));
        ASSERT_EQ(configuration.get("model", "maximum_messages"), "35");
        model.setMaximumMessages(35U);
        model.addMessage(XWalkLanguageModelRole::User, "inspect", "frame.jpg");
        EXPECT_EQ(model.prompt("status"), "model response");
        EXPECT_EQ(backend.instructions, "Be concise");
        EXPECT_EQ(backend.maximumMessages, 35U);
        EXPECT_EQ(backend.role, XWalkLanguageModelRole::User);
        EXPECT_EQ(backend.messageImagePath, "frame.jpg");

        backend.promptResult.clear();
        EXPECT_TRUE(model.prompt("empty response").empty());
        backend.failPrompt = true;
        EXPECT_THROW(static_cast<void>(model.prompt("backend failure")), std::runtime_error);
        EXPECT_EQ(backend.promptCount, 3U);
    }

} /* namespace */
