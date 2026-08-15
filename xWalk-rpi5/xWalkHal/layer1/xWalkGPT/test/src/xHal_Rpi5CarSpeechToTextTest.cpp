/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextTest.cpp
 * @brief       Verifies speech recognition through named in-memory adapters.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Test
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarGptTestSupport.h"
#include "xHal_Rpi5CarGptSimulationArguments.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include <cassert>
namespace
{
    using namespace xwalk::hal;
    using namespace xwalk::hal::test::gpt;
    void testOperations()
    {
        TestRecognitionBackend backend;
        {
            XWalkSpeechToText speech(&backend, recognitionCallbacks());
            assert(speech.isReady());
            assert(backend.readyCount == 1U);
            assert(speech.listen() == "microphone result");
            assert(backend.timeoutMs == XHAL_RPI5CAR_SPEECH_TO_TEXT_DEFAULT_TIMEOUT_MS);
            assert(speech.listen(1'500U) == "microphone result");
            assert(backend.timeoutMs == 1'500U);
            assert(backend.listenCount == 2U);
            assert(speech.transcribeFile("sample.wav") == "file result");
            assert(backend.filePath == "sample.wav");
            assert(backend.fileCount == 1U);
            speech.stop();
            assert(backend.stopCount == 1U);
        }
        assert(backend.stopCount == 2U);
    }
    void testInputValidation()
    {
        TestRecognitionBackend backend;
        XWalkSpeechToText speech(&backend, recognitionCallbacks());
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(speech.listen(0U));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(speech.listen(XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS + 1U));
            });
        assert(backend.listenCount == 0U);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(speech.transcribeFile(""));
            });
        assert(backend.fileCount == 0U);
    }
    void testBackendFailures()
    {
        TestRecognitionBackend backend;
        XWalkSpeechToText speech(&backend, recognitionCallbacks());
        backend.failReady = true;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(speech.isReady());
            });
        backend.failListen = true;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(speech.listen());
            });
        backend.failFile = true;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(speech.transcribeFile("fail.wav"));
            });
        backend.failStop = true;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                speech.stop();
            });
        backend.failStop = false;
    }
    void testCallbackValidation()
    {
        TestRecognitionBackend backend;
        const fixedarray<XWalkSpeechToTextCallbacks, 4U> incomplete{{{nullptr, &listen, &transcribeFile, &stop},
                                                                     {&ready, nullptr, &transcribeFile, &stop},
                                                                     {&ready, &listen, nullptr, &stop},
                                                                     {&ready, &listen, &transcribeFile, nullptr}}};
        for (const XWalkSpeechToTextCallbacks& callbacks : incomplete)
        {
            xwalk::hal::test::expectFailure(
                [&]()
                {
                    XWalkSpeechToText speech(&backend, callbacks);
                    static_cast<void>(speech);
                });
        }
        assert(backend.stopCount == 0U);
    }
    /** @brief Verifies GPT simulation trace argument boundaries. */
    void testSimulationArguments()
    {
        char binary[] = "xWalkGptSimulation";
        char help[] = "--help";
        char shortHelp[] = "-h";
        char trace[] = "--trace";
        char enable[] = "RPI.enable";
        char disable[] = "all.disable";
        char json[] = "trace.json";
        char malformed[] = "RPI.Camera.enable";
        char unknown[] = "--verbose";
        charpointer defaults[]{binary};
        charpointer helpValues[]{binary, help};
        charpointer shortHelpValues[]{binary, shortHelp};
        charpointer enableValues[]{binary, trace, enable};
        charpointer disableValues[]{binary, trace, disable};
        charpointer jsonValues[]{binary, trace, json};
        charpointer malformedValues[]{binary, trace, malformed};
        charpointer unknownValues[]{binary, unknown, enable};
        const sim::XWalkGptSimulationArguments defaultArguments(1, defaults);
        const sim::XWalkGptSimulationArguments helpArguments(2, helpValues);
        const sim::XWalkGptSimulationArguments shortHelpArguments(2, shortHelpValues);
        const sim::XWalkGptSimulationArguments enableArguments(3, enableValues);
        const sim::XWalkGptSimulationArguments disableArguments(3, disableValues);
        const sim::XWalkGptSimulationArguments jsonArguments(3, jsonValues);
        const sim::XWalkGptSimulationArguments malformedArguments(3, malformedValues);
        const sim::XWalkGptSimulationArguments unknownArguments(3, unknownValues);
        const sim::XWalkGptSimulationArguments nullArguments(3, nullptr);
        assert(defaultArguments.valid() && defaultArguments.applyTraceUpdate());
        assert(helpArguments.valid() && helpArguments.helpRequested());
        assert(shortHelpArguments.valid() && shortHelpArguments.helpRequested());
        assert(enableArguments.valid() && enableArguments.applyTraceUpdate());
        assert(disableArguments.valid() && disableArguments.applyTraceUpdate());
        assert(jsonArguments.valid());
        assert(!malformedArguments.valid());
        assert(!unknownArguments.valid());
        assert(!nullArguments.valid());
    }
} /* namespace */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_GPT_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_GPT_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .366, "xWalkSpeechToText host tests started");
    testOperations();
    testInputValidation();
    testBackendFailures();
    testCallbackValidation();
    testSimulationArguments();
    XWALK_HAL_TRACE_UID0(RPI .367, "xWalkSpeechToText host tests completed");
    return 0;
}
