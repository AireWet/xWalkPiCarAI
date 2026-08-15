/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextAlsaTest.cpp
 * @brief       Verifies bounded capture and recognition without a microphone.
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Host Test
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 * @copyright Copyright (c) 2026 Joxy John. All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarSpeechToTextAlsaTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TestBackend = ::xwalk::source_types::xhal_rpi5carspeechtotextalsatest::TestBackend;

/** @brief Contains deterministic test operations. */
namespace
{

    XWalkHal::speechcapturehandle openCapture(XWalkHal::contextpointer context,
                                              XWalkHal::stringview device,
                                              XWalkHal::uint32 rate,
                                              XWalkHal::uint8 channels,
                                              XWalkHal::uint32 period)
    {
        assert(device == "test-mic");
        assert(rate == 16'000U && channels == 1U && period == 1'024U);
        return &static_cast<TestBackend*>(context)->token;
    }

    XWalkHal::int32 readCapture(XWalkHal::contextpointer context,
                                XWalkHal::speechcapturehandle handle,
                                XWalkHal::bytevector& data,
                                XWalkHal::size frames)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        assert(handle == &backend.token);
        if (backend.delayRead)
        {
            backend.readStarted.store(true);
            std::this_thread::sleep_for(XWalkHal::millisecondduration(20));
        }
        ++backend.readCount;
        if (backend.failFirstRead && backend.readCount == 1U)
        {
            return -32;
        }
        data.assign(frames * 2U, 0x22U);
        backend.capturedFrames += frames;
        return static_cast<XWalkHal::int32>(frames);
    }

    XWalkHal::boolean
    recoverCapture(XWalkHal::contextpointer context, XWalkHal::speechcapturehandle handle, XWalkHal::int32 error)
    {
        assert(handle == &static_cast<TestBackend*>(context)->token);
        return error == -32;
    }

    void closeCapture(XWalkHal::contextpointer context, XWalkHal::speechcapturehandle handle)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        assert(handle == &backend.token);
        ++backend.closeCount;
    }

    XWalkHal::boolean ready(XWalkHal::contextpointer context)
    {
        static_cast<void>(context);
        return true;
    }

    XWalkHal::string recognizePcm(XWalkHal::contextpointer context,
                                  const XWalkHal::bytevector& pcm,
                                  XWalkHal::uint32 rate,
                                  XWalkHal::uint8 channels)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        backend.recognizedBytes = pcm.size();
        assert(rate == 16'000U && channels == 1U);
        return "recognized microphone";
    }

    XWalkHal::string recognizeFile(XWalkHal::contextpointer context, XWalkHal::stringview path)
    {
        static_cast<void>(context);
        assert(path == "sample.wav");
        return "recognized file";
    }

    void cancel(XWalkHal::contextpointer context)
    {
        ++static_cast<TestBackend*>(context)->cancelCount;
    }

    XWalkHal::XWalkSpeechToTextAlsaOperations operations()
    {
        return {
            &openCapture, &readCapture, &recoverCapture, &closeCapture, &ready, &recognizePcm, &recognizeFile, &cancel};
    }

    void testCaptureAndRecognition()
    {
        TestBackend backend;
        backend.failFirstRead = true;
        XWalkHal::XWalkSpeechToTextAlsa adapter(&backend, operations(), "test-mic");
        {
            XWalkHal::XWalkSpeechToText speech(&adapter, adapter.callbacks());
            assert(speech.isReady());
            assert(speech.listen(100U) == "recognized microphone");
            assert(backend.capturedFrames == 1'600U);
            assert(backend.recognizedBytes == 3'200U);
            assert(backend.closeCount == 1U);
            assert(speech.transcribeFile("sample.wav") == "recognized file");
            speech.stop();
        }
        assert(backend.cancelCount == 2U);
    }

    void testValidation()
    {
        TestBackend backend;
        XWalkHal::XWalkSpeechToTextAlsaOperations incomplete = operations();
        incomplete.recognizePcm = nullptr;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkSpeechToTextAlsa adapter(&backend, incomplete, "test-mic");
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkSpeechToTextAlsa adapter(&backend, operations(), "");
            });
    }

    void testCancellation()
    {
        TestBackend backend;
        backend.delayRead = true;
        XWalkHal::XWalkSpeechToTextAlsa adapter(&backend, operations(), "test-mic");
        XWalkHal::XWalkSpeechToText speech(&adapter, adapter.callbacks());
        XWalkHal::string transcript{"not cancelled"};
        XWalkHal::threadhandle listener(
            [&]()
            {
                transcript = speech.listen(1'000U);
            });
        for (XWalkHal::uint32 waitCount{}; !backend.readStarted.load() && (waitCount < 1'000U); ++waitCount)
        {
            std::this_thread::sleep_for(XWalkHal::millisecondduration(1));
        }
        assert(backend.readStarted.load());
        speech.stop();
        listener.join();
        assert(transcript.empty());
        assert(backend.closeCount == 1U);
        assert(backend.cancelCount == 1U);
    }
} /* namespace */

/** @brief Runs all speech capture adapter host tests. */
XWalkHal::int32 main()
{
    testCaptureAndRecognition();
    testValidation();
    testCancellation();
    return 0;
}
