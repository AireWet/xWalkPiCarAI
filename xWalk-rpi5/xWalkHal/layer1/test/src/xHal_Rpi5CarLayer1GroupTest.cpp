/******************************************************************************
 * @file        xHal_Rpi5CarLayer1GroupTest.cpp
 * @brief       Verifies collaboration among Layer 1 modules.
 *
 * @details
 * Composes robot, board, media, speech, model, and assistant coordinators over
 * deterministic callbacks without audio devices, models, networks, or hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalk Layer 1 Group Test
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

#include "xHal_Rpi5CarBoardControlTestSupport.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarGptTestSupport.h"
#include "xHal_Rpi5CarLanguageModelTestSupport.h"
#include "xHal_Rpi5CarLayer1GroupTestSupport.h"
#include "xHal_Rpi5CarMusicTestSupport.h"
#include "xHal_Rpi5CarRobotTestSupport.h"
#include "xHal_Rpi5CarSpeakerTestSupport.h"
#include "xHal_Rpi5CarVoiceAssistantTestSupport.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains Layer 1 group scenarios private to this translation unit. */
namespace
{

    using namespace xwalk::hal;

    /** @brief Returns one isolated build-local test path. */
    filesystempath layer1TestPath(stringview name)
    {
        const filesystempath directory(XWALK_LAYER1_GROUP_TEST_DATA_DIRECTORY);
        const boolean created = createDirectories(directory);
        static_cast<void>(created);
        return directory / string(name);
    }

    using VoicePipelineFixture = xwalk::hal::test::layer1::VoicePipelineFixture;

    /** @brief Verifies board readiness precedes configured Robot initialization and movement. */
    TEST(XWalkLayer1Group, BoardControlAndRobotInitializationUseInjectedDependencies)
    {
        using namespace xwalk::hal::test;
        boardcontrol::TestGpioBackend resetBackend;
        boardcontrol::TestGpioBackend speakerBackend;
        boardcontrol::TestI2cBackend boardBus;
        boardcontrol::TestSpeakerPrime primeBackend;
        const XWalkGpioCallbacks gpioCallbackTable = boardcontrol::gpioCallbacks();
        XWalkGpio resetGpio(&resetBackend, gpioCallbackTable, "MCURST");
        XWalkGpio speakerGpio(&speakerBackend, gpioCallbackTable, XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
        XWalkI2c boardI2c(&boardBus, &boardcontrol::probeI2c, &boardcontrol::writeI2c, &boardcontrol::readI2c);
        XWalkAdc batteryAdc(boardI2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc, &primeBackend, &boardcontrol::primeSpeaker);
        boardControl.resetMcu();
        EXPECT_EQ(resetBackend.writeCount, 2U);
        EXPECT_NEAR(boardControl.batteryVoltage(), 9.9, 0.000001);

        const filesystempath configurationPath = layer1TestPath("robot.conf");
        static_cast<void>(removeFilesystemEntry(configurationPath));
        robot::TestBus robotBus;
        XWalkI2c robotI2c(&robotBus, &robot::probe, &robot::writeRegister, &robot::read);
        XWalkPwmTimerState timerState;
        XWalkPwm firstPwm(robotI2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
        XWalkPwm secondPwm(robotI2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
        XWalkServo firstServo(firstPwm);
        XWalkServo secondServo(secondPwm);
        XWalkConfigStore store(configurationPath.string());
        store.set("walker_servo_offset_list", "[5,-5]");
        XWalkRobot robot(store, "walker", 0U);
        robot.addServo(firstServo, 10.0);
        robot.addServo(secondServo, -10.0);
        robot.initialize({1U, 0U});
        robot.reset({15.0, -15.0});

        EXPECT_TRUE(robot.initialized());
        EXPECT_THAT(robot.offsets(), testing::ElementsAre(5.0, -5.0));
        EXPECT_THAT(robot.servoPositions(), testing::ElementsAre(15.0, -15.0));
        EXPECT_GT(robotBus.writeCount, 0U);

        XWalkRobot invalidRobot(store, "invalid", 0U);
        EXPECT_THROW(invalidRobot.initialize(), std::invalid_argument);
    }

    /** @brief Verifies Music generation and Speaker decoding share only fake audio boundaries. */
    TEST(XWalkLayer1Group, MusicAndSpeakerWorkflowRemainsSilentAndDeviceFree)
    {
        using namespace xwalk::hal::test;
        music::TestBackend musicBackend;
        XWalkMusic music(&musicBackend, music::musicCallbacks());
        music.playToneFor(440.0, 0.01);
        ASSERT_EQ(musicBackend.toneCount, 1U);
        ASSERT_FALSE(musicBackend.pcmData.empty());
        EXPECT_EQ(musicBackend.sampleRateHz, 44'100U);
        EXPECT_EQ(musicBackend.channelCount, 1U);

        const filesystempath audioPath = layer1TestPath("group-tone.wav");
        speaker::createTestFile(audioPath);
        speaker::TestBackend speakerBackend;
        speakerBackend.decodedFrameCount = 1U;
        speakerBackend.writeDelayEnabled = false;
        {
            XWalkSpeaker speaker(&speakerBackend, speaker::speakerCallbacks());
            const string taskId = speaker.play(audioPath.string());
            EXPECT_FALSE(taskId.empty());
            ASSERT_TRUE(speaker::waitForWriteCount(speakerBackend, 1U))
                << "The fake speaker backend did not receive decoded audio";
        }
        EXPECT_EQ(speakerBackend.decodeCount, 1U);
        EXPECT_EQ(speakerBackend.openCount, 1U);
        EXPECT_GT(speakerBackend.writeCount, 0U);
        EXPECT_EQ(speakerBackend.closeCount, 1U);

        speakerBackend.invalidAudio = true;
        XWalkSpeaker invalidSpeaker(&speakerBackend, speaker::speakerCallbacks());
        EXPECT_THROW(static_cast<void>(invalidSpeaker.play(audioPath.string())), std::runtime_error);
        EXPECT_THROW(music.playToneFor(440.0, -1.0), std::out_of_range);
    }

    /** @brief Verifies the complete speech, model, and speech-output round. */
    TEST(XWalkLayer1Group, VoiceAssistantRunsOrderedSpeechModelAndOutputFlow)
    {
        VoicePipelineFixture fixture;
        EXPECT_TRUE(fixture.speakerBackend.physicalValue)
            << "Text-to-speech construction must prepare board speaker output";
        EXPECT_EQ(fixture.primeBackend.callCount, 1U);
        fixture.assistant.start();
        EXPECT_EQ(fixture.outputBackend.text, "Ready");

        const string response = fixture.assistant.runRound(1'500U, "frame.jpg");
        EXPECT_EQ(response, "model response");
        EXPECT_EQ(fixture.recognitionBackend.timeoutMs, 1'500U);
        EXPECT_EQ(fixture.modelBackend.promptText, "microphone result");
        EXPECT_EQ(fixture.modelBackend.promptImagePath, "frame.jpg");
        EXPECT_EQ(fixture.outputBackend.text, "model response");
        EXPECT_EQ(fixture.recognitionBackend.listenCount, 1U);
        EXPECT_EQ(fixture.modelBackend.promptCount, 1U);
        EXPECT_EQ(fixture.outputBackend.callCount, 2U);

        fixture.assistant.stop();
        EXPECT_FALSE(fixture.assistant.isRunning());
        EXPECT_EQ(fixture.recognitionBackend.stopCount, 1U);
    }

    /** @brief Verifies silence and earlier critical failures suppress later speech output. */
    TEST(XWalkLayer1Group, VoiceAssistantStopsDataFlowAfterCriticalFailures)
    {
        VoicePipelineFixture silentFixture;
        silentFixture.recognitionBackend.listenResult.clear();
        silentFixture.assistant.start();
        const uint32 outputCountBeforeSilence = silentFixture.outputBackend.callCount;
        EXPECT_TRUE(silentFixture.assistant.runRound().empty());
        EXPECT_EQ(silentFixture.modelBackend.promptCount, 0U);
        EXPECT_EQ(silentFixture.outputBackend.callCount, outputCountBeforeSilence);

        VoicePipelineFixture recognitionFailureFixture;
        recognitionFailureFixture.recognitionBackend.failListen = true;
        recognitionFailureFixture.assistant.start();
        const uint32 outputCountBeforeRecognitionFailure = recognitionFailureFixture.outputBackend.callCount;
        EXPECT_THROW(static_cast<void>(recognitionFailureFixture.assistant.runRound()), std::runtime_error);
        EXPECT_EQ(recognitionFailureFixture.modelBackend.promptCount, 0U);
        EXPECT_EQ(recognitionFailureFixture.outputBackend.callCount, outputCountBeforeRecognitionFailure);

        VoicePipelineFixture modelFailureFixture;
        modelFailureFixture.modelBackend.failPrompt = true;
        modelFailureFixture.assistant.start();
        const uint32 outputCountBeforeModelFailure = modelFailureFixture.outputBackend.callCount;
        EXPECT_THROW(static_cast<void>(modelFailureFixture.assistant.runRound()), std::runtime_error);
        EXPECT_EQ(modelFailureFixture.outputBackend.callCount, outputCountBeforeModelFailure);

        VoicePipelineFixture outputFailureFixture;
        outputFailureFixture.outputBackend.fail = true;
        EXPECT_THROW(outputFailureFixture.assistant.start(), std::runtime_error);
        EXPECT_TRUE(outputFailureFixture.assistant.isRunning())
            << "The assistant enters running state before delivering its welcome prompt";
        outputFailureFixture.outputBackend.fail = false;
        outputFailureFixture.assistant.stop();
        EXPECT_FALSE(outputFailureFixture.assistant.isRunning());
    }

} /* namespace */
