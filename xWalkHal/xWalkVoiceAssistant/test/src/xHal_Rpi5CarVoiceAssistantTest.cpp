/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantTest.cpp
 * @brief       Verifies synchronous voice-assistant orchestration.
 *
 * @details
 * Exercises injected pipeline construction, lifecycle callbacks, a complete
 * recognized-speech round, silence handling, response parsing, and state checks.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant Host Test
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarVoiceAssistant.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains host-test state and callbacks private to this translation unit. */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Retains the physical level of one simulated GPIO. */
struct TestGpioBackend
{
    boolean value{}; /**< Most recently configured or written physical level. */
};

/** @brief Supplies stateless simulated I2C operations. */
struct TestI2cBackend
{
    uint32 readCount{}; /**< Number of sequential read requests. */
};

/** @brief Records the speaker-prime operation. */
struct TestPrimeBackend
{
    uint32 callCount{}; /**< Number of priming requests. */
};

/** @brief Supplies deterministic speech-recognition behavior. */
struct TestRecognitionBackend
{
    string result{"where am I"}; /**< Next final recognized text. */
    uint32 listenCount{}; /**< Number of microphone recognition requests. */
    uint32 stopCount{}; /**< Number of recognition cancellation requests. */
};

/** @brief Supplies deterministic model behavior and records forwarded values. */
struct TestModelBackend
{
    string instructions{}; /**< Most recently supplied system instructions. */
    string promptText{}; /**< Most recently supplied prompt. */
    string imagePath{}; /**< Most recently supplied optional image path. */
    string response{"raw response"}; /**< Next final model response. */
    uint32 promptCount{}; /**< Number of model requests. */
};

/** @brief Records synthesized speech requests. */
struct TestOutputBackend
{
    string text{}; /**< Most recently synthesized text. */
    uint32 callCount{}; /**< Number of synthesis requests. */
};

/** @brief Records optional assistant callbacks. */
struct TestHookBackend
{
    string heard{}; /**< Most recently recognized non-empty input. */
    string spoken{}; /**< Most recently completed speech text. */
    uint32 startCount{}; /**< Number of start notifications. */
    uint32 roundCount{}; /**< Number of completed-round notifications. */
    uint32 stopCount{}; /**< Number of stop notifications. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Retains one simulated GPIO configuration level. */
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue)
{
    static_cast<TestGpioBackend*>(context)->value = initialValue;
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
}

/** @brief Returns the retained simulated GPIO level. */
boolean readGpio(contextpointer context, uint8 pin)
{
    static_cast<void>(pin);
    return static_cast<TestGpioBackend*>(context)->value;
}

/** @brief Retains one simulated GPIO output level. */
void writeGpio(contextpointer context, uint8 pin, boolean value)
{
    static_cast<TestGpioBackend*>(context)->value = value;
    static_cast<void>(pin);
}

/** @brief Accepts an unused simulated interrupt registration. */
void registerInterrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(edge);
    static_cast<void>(debounceMs);
    static_cast<void>(handlerContext);
    static_cast<void>(handler);
}

/** @brief Accepts an unused simulated interrupt cancellation. */
void cancelInterrupt(contextpointer context, uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

/** @brief Creates the complete simulated GPIO callback table. */
XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
}

/** @brief Reports every simulated I2C address as present. */
boolean probeI2c(contextpointer context, uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

/** @brief Accepts an unused simulated I2C register write. */
void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
}

/** @brief Records an unused simulated I2C sequential read. */
bytevector readI2c(contextpointer context, uint8 address, size length)
{
    ++static_cast<TestI2cBackend*>(context)->readCount;
    static_cast<void>(address);
    static_cast<void>(length);
    return {};
}

/** @brief Records speaker priming without producing audio. */
void primeSpeaker(contextpointer context, uint32 durationMs)
{
    ++static_cast<TestPrimeBackend*>(context)->callCount;
    static_cast<void>(durationMs);
}

/** @brief Reports that the simulated recognition backend is ready. */
boolean recognitionReady(contextpointer context)
{
    static_cast<void>(context);
    return true;
}

/** @brief Returns the configured simulated recognition result. */
string recognizeSpeech(contextpointer context, uint32 timeoutMs)
{
    TestRecognitionBackend& backend = *static_cast<TestRecognitionBackend*>(context);
    ++backend.listenCount;
    static_cast<void>(timeoutMs);
    return backend.result;
}

/** @brief Returns the configured recognition result for an unused file request. */
string transcribeFile(contextpointer context, stringview filePath)
{
    static_cast<void>(filePath);
    return static_cast<TestRecognitionBackend*>(context)->result;
}

/** @brief Records one recognition cancellation request. */
void stopRecognition(contextpointer context)
{
    ++static_cast<TestRecognitionBackend*>(context)->stopCount;
}

/** @brief Creates the complete simulated recognition callback table. */
XWalkSpeechToTextCallbacks recognitionCallbacks()
{
    return {&recognitionReady, &recognizeSpeech, &transcribeFile, &stopRecognition};
}

/** @brief Retains system instructions supplied to the simulated model. */
void setInstructions(contextpointer context, stringview instructions)
{
    static_cast<TestModelBackend*>(context)->instructions = string(instructions);
}

/** @brief Accepts unused model welcome text. */
void setWelcome(contextpointer context, stringview welcome)
{
    static_cast<void>(context);
    static_cast<void>(welcome);
}

/** @brief Accepts an unused retained-message limit. */
void setMaximumMessages(contextpointer context, uint32 maximumMessages)
{
    static_cast<void>(context);
    static_cast<void>(maximumMessages);
}

/** @brief Accepts an unused conversation-history message. */
void addMessage(contextpointer context, XWalkLanguageModelRole role,
    stringview content, stringview imagePath)
{
    static_cast<void>(context);
    static_cast<void>(role);
    static_cast<void>(content);
    static_cast<void>(imagePath);
}

/** @brief Records a model request and returns its configured response. */
string promptModel(contextpointer context, stringview promptText, stringview imagePath)
{
    TestModelBackend& backend = *static_cast<TestModelBackend*>(context);
    ++backend.promptCount;
    backend.promptText = string(promptText);
    backend.imagePath = string(imagePath);
    return backend.response;
}

/** @brief Creates the complete simulated language-model callback table. */
XWalkLanguageModelCallbacks modelCallbacks()
{
    return {&setInstructions, &setWelcome, &setMaximumMessages, &addMessage, &promptModel};
}

/** @brief Records synthesized output text. */
void synthesizeText(contextpointer context, stringview text)
{
    TestOutputBackend& backend = *static_cast<TestOutputBackend*>(context);
    ++backend.callCount;
    backend.text = string(text);
}

/** @brief Records one assistant start notification. */
void onStart(contextpointer context)
{
    ++static_cast<TestHookBackend*>(context)->startCount;
}

/** @brief Records non-empty recognized input. */
void onHeard(contextpointer context, stringview text)
{
    static_cast<TestHookBackend*>(context)->heard = string(text);
}

/** @brief Prefixes the raw model response for observable parser coverage. */
string parseResponse(contextpointer context, stringview response)
{
    static_cast<void>(context);
    return string("parsed: ").append(response);
}

/** @brief Records completed speech text. */
void afterSay(contextpointer context, stringview text)
{
    static_cast<TestHookBackend*>(context)->spoken = string(text);
}

/** @brief Records one successful round completion. */
void onRoundComplete(contextpointer context)
{
    ++static_cast<TestHookBackend*>(context)->roundCount;
}

/** @brief Records one assistant stop notification. */
void onStop(contextpointer context)
{
    ++static_cast<TestHookBackend*>(context)->stopCount;
}

/** @brief Verifies lifecycle, full-round, silence, parser, and state behavior. */
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
    XWalkGpio speakerGpio(&speakerBackend, gpioCallbackTable,
        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
    XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
    XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
        XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc,
        &primeBackend, &primeSpeaker);
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
    XWalkVoiceAssistant assistant(speechToText, languageModel, textToSpeech,
        configuration, &hookBackend, callbacks);

    assert(modelBackend.instructions == "Answer briefly");
    assert(!assistant.isRunning());
    xwalk::hal::test::expectFailure([&]()
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

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every host-side voice-assistant test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int main()
{
    testVoiceAssistantPipeline();
    return 0;
}
