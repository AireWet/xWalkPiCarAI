/******************************************************************************
 * @file        xHal_Rpi5CarAudioAlsaTest.cpp
 * @brief       Verifies shared ALSA ownership with injected software
 *operations.
 *
 * @details
 * Exercises negotiation, mixer ownership, short writes, bounded recovery,
 * stream limits, validation, explicit close, and deterministic destruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio ALSA Software Test
 *
 * @author      Joxy John
 * @date        2026-08-01
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

#include "xHal_Rpi5CarAudioAlsa.h"
#include "xHal_Rpi5CarAudioAlsaTestSupport.h"
#include "xHal_Rpi5CarAudioHandler.h"
#include "xHal_Rpi5CarAudioHostStub.h"
#include "xHal_Rpi5CarAudioSimulationArguments.h"
#include "xHal_Rpi5CarAudioSimulationConfig.h"

#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains injected ALSA state and software tests private to this
 * translation unit. */
namespace
{

    using namespace xwalk::hal;
    using xwalk::hal::test::audio::TestAudioOperations;
    using xwalk::hal::test::audio::testConfiguration;
    using xwalk::hal::test::audio::testOperations;

    /******************************************************************************
     * Private function definitions
     ******************************************************************************/

    /** @brief Verifies mixer ownership, negotiation, short writes, recovery, and
     * explicit close. */
    void testOwnershipAndWrites()
    {
        TestAudioOperations state;
        {
            XWalkAudioAlsa audio(&state, testOperations(), "test-pcm", "test-mixer", "PCM");
            assert(state.openMixerCount == 1U);
            assert(state.mixerDevice == "test-mixer");

            const XWalkAudioStreamConfiguration configuration = testConfiguration();
            audiopcmhandle stream = audio.openStream(configuration);
            assert(stream != nullptr);
            assert(audio.openStreamCount() == 1U);
            assert(state.pcmDevice == "test-pcm");
            assert(state.configuration.sampleRateHz == 44'100U);
            assert(state.configuration.periodFrames == 256U);

            const size frameCount = 4U;
            const size bytesPerFrame = 4U;
            const bytevector silence(frameCount * bytesPerFrame, 0U);
            audio.writeFrames(stream, silence, frameCount);
            assert(state.recoverCount == 1U);
            assert(state.writeCount == 3U);
            assert(state.byteOffset == 8U);
            assert(state.requestedFrames == 2U);

            audio.setVolume(37U);
            assert(state.mixerElement == "PCM");
            assert(state.volumePercent == 37U);
            audio.closeStream(stream);
            assert(audio.openStreamCount() == 0U);
            assert(state.closePcmCount == 1U);
        }
        assert(state.closeMixerCount == 1U);
    }

    /** @brief Verifies that destruction closes every retained PCM handle before the
     * mixer. */
    void testDestructorCleanup()
    {
        TestAudioOperations state;
        {
            XWalkAudioAlsa audio(&state, testOperations(), "pcm", "mixer", "PCM");
            static_cast<void>(audio.openStream(testConfiguration()));
            static_cast<void>(audio.openStream(testConfiguration()));
            assert(audio.openStreamCount() == 2U);
        }
        assert(state.closePcmCount == 2U);
        assert(state.closeMixerCount == 1U);
    }

    /** @brief Verifies the fixed eight-stream ownership limit. */
    void testStreamLimit()
    {
        TestAudioOperations state;
        XWalkAudioAlsa audio(&state, testOperations(), "pcm", "mixer", "PCM");
        for (size index = 0U; index < XHAL_RPI5CAR_AUDIO_MAXIMUM_STREAM_COUNT; ++index)
        {
            static_cast<void>(audio.openStream(testConfiguration()));
        }
        assert(audio.openStreamCount() == XHAL_RPI5CAR_AUDIO_MAXIMUM_STREAM_COUNT);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(audio.openStream(testConfiguration()));
            });
    }

    /** @brief Verifies callback, configuration, ownership, payload, and recovery
     * failures. */
    void testValidationAndFailures()
    {
        TestAudioOperations state;
        XWalkAudioAlsaOperations incompleteOperations = testOperations();
        incompleteOperations.writePcm = nullptr;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkAudioAlsa audio(&state, incompleteOperations, "pcm", "mixer", "PCM");
                static_cast<void>(audio);
            });

        XWalkAudioAlsa audio(&state, testOperations(), "pcm", "mixer", "PCM");
        XWalkAudioStreamConfiguration invalidConfiguration = testConfiguration();
        invalidConfiguration.channelCount = 0U;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(audio.openStream(invalidConfiguration));
            });

        audiopcmhandle stream = audio.openStream(testConfiguration());
        xwalk::hal::test::expectFailure(
            [&]()
            {
                audio.writeFrames(stream, bytevector(3U, 0U), 1U);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                const size excessiveFrames = static_cast<size>(testConfiguration().periodFrames) + 1U;
                audio.writeFrames(stream, bytevector(excessiveFrames * 4U, 0U), excessiveFrames);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                audio.setVolume(101U);
            });
        audio.closeStream(stream);

        TestAudioOperations failedRecoveryState;
        failedRecoveryState.recoverSucceeds = false;
        XWalkAudioAlsa failedRecoveryAudio(&failedRecoveryState, testOperations(), "pcm", "mixer", "PCM");
        audiopcmhandle failedStream = failedRecoveryAudio.openStream(testConfiguration());
        xwalk::hal::test::expectFailure(
            [&]()
            {
                failedRecoveryAudio.writeFrames(failedStream, bytevector(4U, 0U), 1U);
            });
        failedRecoveryAudio.closeStream(failedStream);
    }

    /** @brief Verifies the standalone handler through the device-free host mirror.
     */
    void testHostSimulation()
    {
        sim::XWalkAudioHostStub hostStub;
        XWalkAudioAlsa audio(&hostStub, hostStub.operations(), "host-pcm", "host-mixer", "PCM");
        const sim::XWalkAudioHandler handler;
        assert(handler.run(audio) == 0);
        assert(hostStub.writtenFrameCount() == 256U);
        assert(hostStub.volumePercent() == 50U);
    }

    /** @brief Verifies default, help, selector, and malformed simulation arguments.
     */
    void testSimulationArguments()
    {
        char binaryName[] = "xWalkAudioSimulation";
        charpointer defaultArguments[]{binaryName};
        const sim::XWalkAudioSimulationArguments defaults(1, defaultArguments);
        assert(defaults.valid());
        assert(defaults.helpRequested() == false);

        char helpOption[] = "--help";
        charpointer helpArguments[]{binaryName, helpOption};
        const sim::XWalkAudioSimulationArguments help(2, helpArguments);
        assert(help.valid());
        assert(help.helpRequested());

        char traceOption[] = "--trace";
        char enableSelector[] = "RPI.093.enable";
        charpointer enableArguments[]{binaryName, traceOption, enableSelector};
        const sim::XWalkAudioSimulationArguments enable(3, enableArguments);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());

        char disableSelector[] = "RPI.093.disable";
        charpointer disableArguments[]{binaryName, traceOption, disableSelector};
        const sim::XWalkAudioSimulationArguments disable(3, disableArguments);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());

        char malformedSelector[] = "RPI.Audio.enable";
        charpointer malformedArguments[]{binaryName, traceOption, malformedSelector};
        const sim::XWalkAudioSimulationArguments malformed(3, malformedArguments);
        assert(malformed.valid() == false);
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every injected ALSA software test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int main()
{
    const filesystempath traceConfigurationPath(XWALK_AUDIO_SIMULATION_TRACE_CONFIG_PATH);
    const filesystempath traceLogPath(XWALK_AUDIO_SIMULATION_TRACE_LOG_PATH);
    XWalkTrace::configureGlobal(traceConfigurationPath, traceLogPath);
    XWALK_HAL_TRACE_UID0(RPI .098, "xWalkAudio operation tests started");
    testOwnershipAndWrites();
    testDestructorCleanup();
    testStreamLimit();
    testValidationAndFailures();
    testHostSimulation();
    testSimulationArguments();
    XWALK_HAL_TRACE_UID0(RPI .099, "xWalkAudio operation tests completed");
    return 0;
}
