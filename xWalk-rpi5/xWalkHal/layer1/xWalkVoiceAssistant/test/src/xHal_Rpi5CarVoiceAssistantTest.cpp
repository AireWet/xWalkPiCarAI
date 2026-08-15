/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantTest.cpp
 * @brief       Verifies synchronous voice-assistant orchestration.
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarVoiceAssistant.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarVoiceAssistantSimulationArguments.h"
#include "xHal_Rpi5CarVoiceAssistantSimulationConfig.h"
#include "xHal_Rpi5CarVoiceAssistantTestSupport.h"
#include <cassert>
namespace xwalk::hal::test::voiceassistant
{
    /** @brief Verifies lifecycle, full-round, silence, parser, and state behavior.
     */
    void testVoiceAssistantPipeline()
    {
        TestGpioBackend resetBackend;
        TestGpioBackend speakerBackend;
        TestI2cBackend i2cBackend;
        TestPrimeBackend primeBackend;
        TestRecognitionBackend recognitionBackend;
        TestModelBackend modelBackend;
        TestOutputBackend outputBackend;
        TestHookBackend hookBackend;
        const XWalkGpioCallbacks gpioCallbackTable = gpioCallbacks();
        XWalkGpio resetGpio(&resetBackend, gpioCallbackTable, "MCURST");
        XWalkGpio speakerGpio(&speakerBackend, gpioCallbackTable, XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
        XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
        XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc, &primeBackend, &primeSpeaker);
        XWalkSpeechToText speechToText(&recognitionBackend, recognitionCallbacks());
        XWalkLanguageModel languageModel(&modelBackend, modelCallbacks());
        XWalkTextToSpeech textToSpeech(boardControl, &outputBackend, &synthesizeText);
        XWalkVoiceAssistantConfiguration configuration{"Answer briefly", "Ready"};
        XWalkVoiceAssistantCallbacks callbacks{};
        callbacks.onStart = &onStart;
        callbacks.onHeard = &onHeard;
        callbacks.parseResponse = &parseResponse;
        callbacks.afterSay = &afterSay;
        callbacks.onRoundComplete = &onRoundComplete;
        callbacks.onStop = &onStop;
        XWalkVoiceAssistant assistant(
            speechToText, languageModel, textToSpeech, configuration, &hookBackend, callbacks);
        assert(modelBackend.instructions == "Answer briefly");
        assert(!assistant.isRunning());
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(assistant.runRound());
            });
        assistant.start();
        assistant.start();
        assert(assistant.isRunning());
        assert(hookBackend.startCount == 1U);
        assert(outputBackend.text == "Ready");
        assert(outputBackend.callCount == 1U);
        assert(assistant.runRound(1'500U, "frame.jpg") == "parsed: raw response");
        assert(recognitionBackend.listenCount == 1U);
        assert(modelBackend.promptText == "where am I");
        assert(modelBackend.imagePath == "frame.jpg");
        assert(hookBackend.heard == "where am I");
        assert(hookBackend.spoken == "parsed: raw response");
        assert(outputBackend.text == "parsed: raw response");
        assert(hookBackend.roundCount == 1U);
        recognitionBackend.result.clear();
        assert(assistant.runRound().empty());
        assert(modelBackend.promptCount == 1U);
        assert(outputBackend.callCount == 2U);
        assert(hookBackend.roundCount == 2U);
        assistant.stop();
        assistant.stop();
        assert(!assistant.isRunning());
        assert(recognitionBackend.stopCount == 1U);
        assert(hookBackend.stopCount == 1U);
    }
} /* namespace xwalk::hal::test::voiceassistant */
int main(int count, char* values[])
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_LOG_PATH);
    const xwalk::hal::sim::XWalkVoiceAssistantSimulationArguments arguments(count, values);
    if (!arguments.valid() || !arguments.applyTraceUpdate())
    {
        return 2;
    }
    XWALK_HAL_TRACE_UID0(RPI .381, "xWalkVoiceAssistant host tests started");
    xwalk::hal::test::voiceassistant::testVoiceAssistantPipeline();
    XWALK_HAL_TRACE_UID0(RPI .382, "xWalkVoiceAssistant host tests completed");
    return 0;
}
