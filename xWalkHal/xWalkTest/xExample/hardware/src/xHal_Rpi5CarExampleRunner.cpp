/******************************************************************************
 * @file        xHal_Rpi5CarExampleRunner.cpp
 * @brief       Implements centralized ported-example CLI dispatch.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarExampleRunner.h"

#include "xHal_Rpi5CarExampleConfig.h"
#include "xHal_Rpi5CarDeepseekExampleLinux.h"
#include "xHal_Rpi5CarDoubaoExampleLinux.h"
#include "xHal_Rpi5CarDoubaoImageExampleLinux.h"
#include "xHal_Rpi5CarGeminiExampleLinux.h"
#include "xHal_Rpi5CarGrokExampleLinux.h"
#include "xHal_Rpi5CarLedExampleLinux.h"
#include "xHal_Rpi5CarOllamaExampleLinux.h"
#include "xHal_Rpi5CarOllamaImageExampleLinux.h"
#include "xHal_Rpi5CarOpenAiImageExampleLinux.h"
#include "xHal_Rpi5CarOpenAiExampleLinux.h"
#include "xHal_Rpi5CarOthersExampleLinux.h"
#include "xHal_Rpi5CarQwenExampleLinux.h"
#include "xHal_Rpi5CarPinInputExampleLinux.h"
#include "xHal_Rpi5CarUltrasonicExampleLinux.h"
#include "xHal_Rpi5CarVoiceAssistantExampleLinux.h"
#include "xHal_Rpi5CarServoExampleLinux.h"
#include "xHal_Rpi5CarSttVoskStreamExampleLinux.h"
#include "xHal_Rpi5CarSttVoskWakeWordThreadExampleLinux.h"
#include "xHal_Rpi5CarSttVoskWakeWordExampleLinux.h"
#include "xHal_Rpi5CarSttVoskWithoutStreamExampleLinux.h"
#include "xHal_Rpi5CarTtsEdgeExampleLinux.h"
#include "xHal_Rpi5CarTtsEspeakExampleLinux.h"
#include "xHal_Rpi5CarTtsOpenAiExampleLinux.h"
#include "xHal_Rpi5CarTtsPico2WaveExampleLinux.h"
#include "xHal_Rpi5CarTtsPiperExampleLinux.h"

#include <charconv>
#include <cstdlib>
#include <iostream>
#include <yaml-cpp/yaml.h>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::example
{

/** @brief Prints every supported example selector and its arguments. */
void XWalkExampleRunner::printUsage()
{
    std::cerr <<
        "usage:\n"
        "  xExample [--config <yaml-file>] <example-name>\n"
        "  xExample [--config=<yaml-file>] <example-name>\n"
        "  formal-argument compatibility forms:\n"
        "  xExample led <gpio-device> <chip-name> <chip-label>\n"
        "  xExample llm-deepseek <maximum-prompts>\n"
        "  xExample llm-doubao <maximum-prompts>\n"
        "  xExample llm-gemini <maximum-prompts>\n"
        "  xExample llm-grok <maximum-prompts>\n"
        "  xExample llm-ollama <maximum-prompts>\n"
        "  xExample llm-ollama-with-image <maximum-prompts> "
        "<camera-connection> <capture-executable> <camera-device>\n"
        "  xExample llm-openai-with-image <maximum-prompts> "
        "<camera-connection> <capture-executable> <camera-device>\n"
        "  xExample llm-openai <maximum-prompts>\n"
        "  xExample llm-others <maximum-prompts> <chat-endpoint> <model>\n"
        "  xExample llm-qwen <maximum-prompts>\n"
        "  xExample pin-input <samples> <gpio-device> "
        "<chip-name> <chip-label>\n"
        "  xExample ultrasonic <samples> <gpio-device> "
        "<chip-name> <chip-label>\n"
        "  xExample voice-assistant <keyboard|wake> <rounds> <timeout-ms> "
        "<microphone-device> <vosk-library> <vosk-model-path> "
        "<piper-executable> <wav-player> <camera-connection> "
        "<capture-executable> <camera-device>\n"
        "  xExample servo <cycles> <i2c-device>\n"
        "  xExample stt-vosk-stream <sessions> <timeout-ms> "
        "<microphone-device> <vosk-library> <model-path>\n"
        "  xExample stt-vosk-wake-word-thread <detections> <maximum-polls> "
        "<listen-timeout-ms> <microphone-device> <vosk-library> <model-path>\n"
        "  xExample stt-vosk-wake-word <maximum-attempts> <listen-timeout-ms> "
        "<microphone-device> <vosk-library> <model-path>\n"
        "  xExample stt-vosk-without-stream <sessions> <listen-timeout-ms> "
        "<microphone-device> <vosk-library> <model-path>\n"
        "  xExample tts-edge <edge-playback-executable>\n"
        "  xExample tts-espeak <espeak-executable>\n"
        "  xExample tts-openai <mp3-playback-executable>\n"
        "  xExample tts-pico2wave <pico2wave-executable> "
        "<wav-playback-executable>\n"
        "  xExample tts-piper <piper-executable> "
        "<wav-playback-executable>\n"
        "  xExample llm-doubao-with-image <maximum-prompts> "
        "<camera-connection> <capture-executable> <camera-device>\n";
}

/**
 * @brief Validates and runs the physical LED example.
 *
 * @param[in] argumentCount Required value of five.
 * @param[in] argumentValues Selector arguments in documented order.
 * @return Zero after completion, or two for invalid input.
 */
int32 XWalkExampleRunner::runLed(int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 5)
    {
        printUsage();
        return 2;
    }

    XWalkLedExampleLinux linuxExample;
    linuxExample.run(argumentValues[2U], argumentValues[3U], argumentValues[4U]);
    return 0;
}

/**
 * @brief Validates and runs bounded remote DeepSeek chat.
 *
 * @param[in] argumentCount Required value of three.
 * @param[in] argumentValues Selector followed by a bounded prompt count.
 * @return Zero after completion, or two for invalid input or a missing key.
 */
int32 XWalkExampleRunner::runDeepseek(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_DEEPSEEK_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    const cstring apiKey = std::getenv("DEEPSEEK_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr << "DEEPSEEK_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkDeepseekExampleLinux linuxExample;
    linuxExample.run(apiKey, maximumPrompts);
    return 0;
}

/**
 * @brief Validates and runs bounded remote Doubao chat.
 * @param[in] argumentCount Required value of three.
 * @param[in] argumentValues Selector followed by a bounded prompt count.
 * @return Zero after completion, or two for invalid input or a missing key.
 */
int32 XWalkExampleRunner::runDoubao(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_DOUBAO_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    const cstring apiKey = std::getenv("DOUBAO_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr << "DOUBAO_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkDoubaoExampleLinux linuxExample;
    linuxExample.run(apiKey, maximumPrompts);
    return 0;
}

/**
 * @brief Validates and runs bounded remote Doubao camera chat.
 *
 * @param[in] argumentCount Required value of six.
 * @param[in] argumentValues Prompt limit and Linux camera arguments.
 * @return Zero after completion, or two for invalid input or a missing key.
 */
int32 XWalkExampleRunner::runDoubaoImage(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 6)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_DOUBAO_IMAGE_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    const cstring apiKey = std::getenv("DOUBAO_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr << "DOUBAO_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkDoubaoImageExampleLinux linuxExample;
    linuxExample.run(apiKey, maximumPrompts, argumentValues[3U],
        argumentValues[4U], argumentValues[5U]);
    return 0;
}

/**
 * @brief Validates and runs bounded remote Gemini chat.
 * @param[in] argumentCount Required value of three.
 * @param[in] argumentValues Selector followed by a bounded prompt count.
 * @return Zero after completion, or two for invalid input or a missing key.
 */
int32 XWalkExampleRunner::runGemini(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_GEMINI_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    const cstring apiKey = std::getenv("GEMINI_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr << "GEMINI_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkGeminiExampleLinux linuxExample;
    linuxExample.run(apiKey, maximumPrompts);
    return 0;
}

/**
 * @brief Validates and runs bounded remote Grok chat.
 * @param[in] argumentCount Required value of three.
 * @param[in] argumentValues Selector followed by a bounded prompt count.
 * @return Zero after completion, or two for invalid input or a missing key.
 */
int32 XWalkExampleRunner::runGrok(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_GROK_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    const cstring apiKey = std::getenv("GROK_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr << "GROK_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkGrokExampleLinux linuxExample;
    linuxExample.run(apiKey, maximumPrompts);
    return 0;
}

/**
 * @brief Validates and runs bounded local Ollama camera chat.
 * @param[in] argumentCount Required value of six.
 * @param[in] argumentValues Prompt limit and Linux camera arguments.
 * @return Zero after completion, or two for invalid input.
 */
int32 XWalkExampleRunner::runOllamaImage(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 6)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_OLLAMA_IMAGE_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    XWalkOllamaImageExampleLinux linuxExample;
    linuxExample.run(maximumPrompts, argumentValues[3U],
        argumentValues[4U], argumentValues[5U]);
    return 0;
}

/**
 * @brief Validates and runs bounded local Ollama text chat.
 * @param[in] argumentCount Required value of three.
 * @param[in] argumentValues Selector followed by a bounded prompt count.
 * @return Zero after completion, or two for invalid input.
 */
int32 XWalkExampleRunner::runOllama(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_OLLAMA_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    XWalkOllamaExampleLinux linuxExample;
    linuxExample.run(maximumPrompts);
    return 0;
}

/**
 * @brief Validates and runs bounded OpenAI camera chat.
 * @param[in] argumentCount Required value of six.
 * @param[in] argumentValues Prompt limit and Linux camera arguments.
 * @return Zero after completion, or two for invalid input or a missing key.
 */
int32 XWalkExampleRunner::runOpenAiImage(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 6)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_OPEN_AI_IMAGE_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    const cstring apiKey = std::getenv("OPENAI_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr <<
            "OPENAI_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkOpenAiImageExampleLinux linuxExample;
    linuxExample.run(apiKey, maximumPrompts, argumentValues[3U],
        argumentValues[4U], argumentValues[5U]);
    return 0;
}

/**
 * @brief Validates and runs bounded OpenAI text chat.
 * @param[in] argumentCount Required value of three.
 * @param[in] argumentValues Selector followed by a bounded prompt count.
 * @return Zero after completion, or two for invalid input or a missing key.
 */
int32 XWalkExampleRunner::runOpenAi(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_OPEN_AI_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    const cstring apiKey = std::getenv("OPENAI_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr <<
            "OPENAI_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkOpenAiExampleLinux linuxExample;
    linuxExample.run(apiKey, maximumPrompts);
    return 0;
}

/**
 * @brief Validates and runs bounded generic compatible-provider chat.
 * @param[in] argumentCount Required value of five.
 * @param[in] argumentValues Prompt limit, complete endpoint, and model.
 * @return Zero after completion, or two for invalid input or a missing key.
 */
int32 XWalkExampleRunner::runOthers(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 5)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_OTHERS_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }
    if ((argumentValues[3U][0U] == '\0') ||
        (argumentValues[4U][0U] == '\0'))
    {
        std::cerr << "chat-endpoint and model must not be empty\n";
        return 2;
    }

    const cstring apiKey = std::getenv("LLM_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr <<
            "LLM_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkOthersExampleLinux linuxExample;
    linuxExample.run(apiKey, argumentValues[3U], argumentValues[4U],
        maximumPrompts);
    return 0;
}

/**
 * @brief Validates and runs bounded Qwen text chat.
 * @param[in] argumentCount Required value of three.
 * @param[in] argumentValues Selector followed by a bounded prompt count.
 * @return Zero after completion, or two for invalid input or a missing key.
 */
int32 XWalkExampleRunner::runQwen(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 maximumPrompts{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), maximumPrompts);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (maximumPrompts == 0U) ||
        (maximumPrompts > XHAL_RPI5CAR_QWEN_EXAMPLE_MAXIMUM_PROMPTS))
    {
        std::cerr << "maximum-prompts must be an integer from 1 to 100\n";
        return 2;
    }

    const cstring apiKey = std::getenv("QWEN_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr <<
            "QWEN_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkQwenExampleLinux linuxExample;
    linuxExample.run(apiKey, maximumPrompts);
    return 0;
}

/**
 * @brief Validates and runs bounded D3 pin-input sampling.
 * @param[in] argumentCount Required value of six.
 * @param[in] argumentValues Sample count and Linux GPIO discovery arguments.
 * @return Zero after completion, or two for invalid input.
 */
int32 XWalkExampleRunner::runPinInput(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 6)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 sampleCount{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), sampleCount);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (sampleCount == 0U) ||
        (sampleCount > XHAL_RPI5CAR_PIN_INPUT_EXAMPLE_MAXIMUM_SAMPLES))
    {
        std::cerr << "samples must be an integer from 1 to 36000\n";
        return 2;
    }

    XWalkPinInputExampleLinux linuxExample;
    linuxExample.run(sampleCount, argumentValues[3U], argumentValues[4U],
        argumentValues[5U]);
    return 0;
}

/** @brief Validates and runs bounded D2/D3 ultrasonic ranging. */
int32 XWalkExampleRunner::runUltrasonic(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 6)
    {
        printUsage();
        return 2;
    }

    const stringview sampleText(argumentValues[2U]);
    uint32 sampleCount{};
    const std::from_chars_result parseResult = std::from_chars(
        sampleText.data(), sampleText.data() + sampleText.size(), sampleCount);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != sampleText.data() + sampleText.size()) ||
        (sampleCount == 0U) ||
        (sampleCount > XHAL_RPI5CAR_ULTRASONIC_EXAMPLE_MAXIMUM_SAMPLES))
    {
        std::cerr << "samples must be an integer from 1 to 18000\n";
        return 2;
    }

    XWalkUltrasonicExampleLinux linuxExample;
    linuxExample.run(sampleCount, argumentValues[3U],
        argumentValues[4U], argumentValues[5U]);
    return 0;
}

/** @brief Validates and runs bounded multimodal voice-assistant rounds. */
int32 XWalkExampleRunner::runVoiceAssistant(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 13)
    {
        printUsage();
        return 2;
    }

    XWalkVoiceAssistantExampleInputMode inputMode{};
    const stringview modeText(argumentValues[2U]);
    if (modeText == "keyboard")
    {
        inputMode = XWalkVoiceAssistantExampleInputMode::Keyboard;
    }
    else if (modeText == "wake")
    {
        inputMode = XWalkVoiceAssistantExampleInputMode::WakeWord;
    }
    else
    {
        std::cerr << "input mode must be keyboard or wake\n";
        return 2;
    }

    const stringview roundText(argumentValues[3U]);
    const stringview timeoutText(argumentValues[4U]);
    uint32 maximumRounds{};
    uint32 timeoutMs{};
    const std::from_chars_result roundResult = std::from_chars(
        roundText.data(), roundText.data() + roundText.size(), maximumRounds);
    const std::from_chars_result timeoutResult = std::from_chars(
        timeoutText.data(), timeoutText.data() + timeoutText.size(), timeoutMs);
    if ((roundResult.ec != std::errc{}) ||
        (roundResult.ptr != roundText.data() + roundText.size()) ||
        (maximumRounds == 0U) ||
        (maximumRounds >
        XHAL_RPI5CAR_VOICE_ASSISTANT_EXAMPLE_MAXIMUM_ROUNDS))
    {
        std::cerr << "rounds must be an integer from 1 to 100\n";
        return 2;
    }
    if ((timeoutResult.ec != std::errc{}) ||
        (timeoutResult.ptr != timeoutText.data() + timeoutText.size()) ||
        (timeoutMs == 0U) || (timeoutMs > 300'000U))
    {
        std::cerr << "timeout-ms must be an integer from 1 to 300000\n";
        return 2;
    }

    const cstring apiKey = std::getenv("OPENAI_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr <<
            "OPENAI_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkVoiceAssistantExampleLinux linuxExample;
    linuxExample.run(apiKey, inputMode, maximumRounds, timeoutMs,
        argumentValues[5U], argumentValues[6U], argumentValues[7U],
        argumentValues[8U], argumentValues[9U], argumentValues[10U],
        argumentValues[11U], argumentValues[12U]);
    return 0;
}

/**
 * @brief Validates and runs bounded channel-one servo sweeps.
 * @param[in] argumentCount Required value of four.
 * @param[in] argumentValues Cycle count followed by the Linux I2C device.
 * @return Zero after completion, or two for invalid input.
 */
int32 XWalkExampleRunner::runServo(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 4)
    {
        printUsage();
        return 2;
    }

    const stringview countText(argumentValues[2U]);
    uint32 cycleCount{};
    const std::from_chars_result parseResult = std::from_chars(
        countText.data(), countText.data() + countText.size(), cycleCount);
    if ((parseResult.ec != std::errc{}) ||
        (parseResult.ptr != countText.data() + countText.size()) ||
        (cycleCount == 0U) ||
        (cycleCount > XHAL_RPI5CAR_SERVO_EXAMPLE_MAXIMUM_CYCLES))
    {
        std::cerr << "cycles must be an integer from 1 to 100\n";
        return 2;
    }

    XWalkServoExampleLinux linuxExample;
    linuxExample.run(argumentValues[3U], cycleCount);
    return 0;
}

/**
 * @brief Validates and runs bounded offline Vosk recognition sessions.
 * @param[in] argumentCount Required value of seven.
 * @param[in] argumentValues Bounds followed by ALSA and Vosk deployment values.
 * @return Zero after completion, or two for invalid input.
 */
int32 XWalkExampleRunner::runSttVoskStream(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 7)
    {
        printUsage();
        return 2;
    }

    const stringview sessionText(argumentValues[2U]);
    const stringview timeoutText(argumentValues[3U]);
    uint32 sessionCount{};
    uint32 timeoutMs{};
    const std::from_chars_result sessionResult = std::from_chars(
        sessionText.data(), sessionText.data() + sessionText.size(), sessionCount);
    const std::from_chars_result timeoutResult = std::from_chars(
        timeoutText.data(), timeoutText.data() + timeoutText.size(), timeoutMs);
    if ((sessionResult.ec != std::errc{}) ||
        (sessionResult.ptr != sessionText.data() + sessionText.size()) ||
        (sessionCount == 0U) ||
        (sessionCount > XHAL_RPI5CAR_STT_VOSK_STREAM_EXAMPLE_MAXIMUM_SESSIONS))
    {
        std::cerr << "sessions must be an integer from 1 to 100\n";
        return 2;
    }
    if ((timeoutResult.ec != std::errc{}) ||
        (timeoutResult.ptr != timeoutText.data() + timeoutText.size()) ||
        (timeoutMs == 0U) ||
        (timeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
    {
        std::cerr << "timeout-ms must be an integer from 1 to 300000\n";
        return 2;
    }

    XWalkSttVoskStreamExampleLinux linuxExample;
    linuxExample.run(sessionCount, timeoutMs, argumentValues[4U],
        argumentValues[5U], argumentValues[6U]);
    return 0;
}

/** @brief Validates and runs bounded threaded Vosk wake-word detection. */
int32 XWalkExampleRunner::runSttVoskWakeWordThread(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 8)
    {
        printUsage();
        return 2;
    }

    const stringview detectionText(argumentValues[2U]);
    const stringview pollText(argumentValues[3U]);
    const stringview timeoutText(argumentValues[4U]);
    uint32 detectionCount{};
    uint32 maximumPolls{};
    uint32 timeoutMs{};
    const std::from_chars_result detectionResult = std::from_chars(
        detectionText.data(), detectionText.data() + detectionText.size(), detectionCount);
    const std::from_chars_result pollResult = std::from_chars(
        pollText.data(), pollText.data() + pollText.size(), maximumPolls);
    const std::from_chars_result timeoutResult = std::from_chars(
        timeoutText.data(), timeoutText.data() + timeoutText.size(), timeoutMs);
    if ((detectionResult.ec != std::errc{}) ||
        (detectionResult.ptr != detectionText.data() + detectionText.size()) ||
        (detectionCount == 0U) ||
        (detectionCount > XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_MAXIMUM_DETECTIONS))
    {
        std::cerr << "detections must be an integer from 1 to 100\n";
        return 2;
    }
    if ((pollResult.ec != std::errc{}) ||
        (pollResult.ptr != pollText.data() + pollText.size()) ||
        (maximumPolls == 0U) ||
        (maximumPolls > XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_MAXIMUM_POLLS))
    {
        std::cerr << "maximum-polls must be an integer from 1 to 1200\n";
        return 2;
    }
    if ((timeoutResult.ec != std::errc{}) ||
        (timeoutResult.ptr != timeoutText.data() + timeoutText.size()) ||
        (timeoutMs == 0U) ||
        (timeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
    {
        std::cerr << "listen-timeout-ms must be an integer from 1 to 300000\n";
        return 2;
    }

    XWalkSttVoskWakeWordThreadExampleLinux linuxExample;
    linuxExample.run(detectionCount, maximumPolls, timeoutMs,
        argumentValues[5U], argumentValues[6U], argumentValues[7U]);
    return 0;
}

/** @brief Validates and runs bounded synchronous Vosk wake detection. */
int32 XWalkExampleRunner::runSttVoskWakeWord(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 7)
    {
        printUsage();
        return 2;
    }

    const stringview attemptText(argumentValues[2U]);
    const stringview timeoutText(argumentValues[3U]);
    uint32 maximumAttempts{};
    uint32 timeoutMs{};
    const std::from_chars_result attemptResult = std::from_chars(
        attemptText.data(), attemptText.data() + attemptText.size(), maximumAttempts);
    const std::from_chars_result timeoutResult = std::from_chars(
        timeoutText.data(), timeoutText.data() + timeoutText.size(), timeoutMs);
    if ((attemptResult.ec != std::errc{}) ||
        (attemptResult.ptr != attemptText.data() + attemptText.size()) ||
        (maximumAttempts == 0U) ||
        (maximumAttempts > XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_MAXIMUM_ATTEMPTS))
    {
        std::cerr << "maximum-attempts must be an integer from 1 to 1200\n";
        return 2;
    }
    if ((timeoutResult.ec != std::errc{}) ||
        (timeoutResult.ptr != timeoutText.data() + timeoutText.size()) ||
        (timeoutMs == 0U) ||
        (timeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
    {
        std::cerr << "listen-timeout-ms must be an integer from 1 to 300000\n";
        return 2;
    }

    XWalkSttVoskWakeWordExampleLinux linuxExample;
    linuxExample.run(maximumAttempts, timeoutMs, argumentValues[4U],
        argumentValues[5U], argumentValues[6U]);
    return 0;
}

/** @brief Validates and runs bounded non-streaming Vosk recognition. */
int32 XWalkExampleRunner::runSttVoskWithoutStream(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 7)
    {
        printUsage();
        return 2;
    }

    const stringview sessionText(argumentValues[2U]);
    const stringview timeoutText(argumentValues[3U]);
    uint32 sessionCount{};
    uint32 timeoutMs{};
    const std::from_chars_result sessionResult = std::from_chars(
        sessionText.data(), sessionText.data() + sessionText.size(), sessionCount);
    const std::from_chars_result timeoutResult = std::from_chars(
        timeoutText.data(), timeoutText.data() + timeoutText.size(), timeoutMs);
    if ((sessionResult.ec != std::errc{}) ||
        (sessionResult.ptr != sessionText.data() + sessionText.size()) ||
        (sessionCount == 0U) ||
        (sessionCount > XHAL_RPI5CAR_STT_VOSK_WITHOUT_STREAM_EXAMPLE_MAXIMUM_SESSIONS))
    {
        std::cerr << "sessions must be an integer from 1 to 100\n";
        return 2;
    }
    if ((timeoutResult.ec != std::errc{}) ||
        (timeoutResult.ptr != timeoutText.data() + timeoutText.size()) ||
        (timeoutMs == 0U) ||
        (timeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
    {
        std::cerr << "listen-timeout-ms must be an integer from 1 to 300000\n";
        return 2;
    }

    XWalkSttVoskWithoutStreamExampleLinux linuxExample;
    linuxExample.run(sessionCount, timeoutMs, argumentValues[4U],
        argumentValues[5U], argumentValues[6U]);
    return 0;
}

/** @brief Validates and runs one live Microsoft Edge TTS request. */
int32 XWalkExampleRunner::runTtsEdge(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    XWalkTtsEdgeExampleLinux linuxExample(argumentValues[2U]);
    linuxExample.run();
    return 0;
}

/** @brief Validates and runs one configured Espeak request. */
int32 XWalkExampleRunner::runTtsEspeak(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    XWalkTtsEspeakExampleLinux linuxExample(argumentValues[2U]);
    linuxExample.run();
    return 0;
}

/** @brief Validates and runs the three fixed OpenAI TTS requests. */
int32 XWalkExampleRunner::runTtsOpenAi(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 3)
    {
        printUsage();
        return 2;
    }

    const cstring apiKey = std::getenv("OPENAI_API_KEY");
    if ((apiKey == nullptr) || (apiKey[0U] == '\0'))
    {
        std::cerr <<
            "OPENAI_API_KEY must be set without placing the key in arguments\n";
        return 2;
    }

    XWalkTtsOpenAiExampleLinux linuxExample(apiKey, argumentValues[2U]);
    linuxExample.run();
    return 0;
}

/** @brief Validates and runs one configured Pico2Wave request. */
int32 XWalkExampleRunner::runTtsPico2Wave(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 4)
    {
        printUsage();
        return 2;
    }

    XWalkTtsPico2WaveExampleLinux linuxExample(
        argumentValues[2U], argumentValues[3U]);
    linuxExample.run();
    return 0;
}

/** @brief Validates and runs one configured Piper request. */
int32 XWalkExampleRunner::runTtsPiper(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount != 4)
    {
        printUsage();
        return 2;
    }

    XWalkTtsPiperExampleLinux linuxExample(
        argumentValues[2U], argumentValues[3U]);
    linuxExample.run();
    return 0;
}

/**
 * @brief Resolves and runs one ported example selector.
 *
 * @param[in] argumentCount Executable, selector, and selector arguments.
 * @param[in] argumentValues Process arguments in selector-specific order.
 * @return Zero after completion, or two for invalid input.
 */
int32 XWalkExampleRunner::runSelection(
    int32 argumentCount, char* argumentValues[])
{
    if (argumentCount < 2)
    {
        printUsage();
        return 2;
    }

    const stringview selection(argumentValues[1U]);
    if (selection == "led")
    {
        return runLed(argumentCount, argumentValues);
    }
    if (selection == "llm-deepseek")
    {
        return runDeepseek(argumentCount, argumentValues);
    }
    if (selection == "llm-doubao")
    {
        return runDoubao(argumentCount, argumentValues);
    }
    if (selection == "llm-doubao-with-image")
    {
        return runDoubaoImage(argumentCount, argumentValues);
    }
    if (selection == "llm-gemini")
    {
        return runGemini(argumentCount, argumentValues);
    }
    if (selection == "llm-grok")
    {
        return runGrok(argumentCount, argumentValues);
    }
    if (selection == "llm-ollama")
    {
        return runOllama(argumentCount, argumentValues);
    }
    if (selection == "llm-ollama-with-image")
    {
        return runOllamaImage(argumentCount, argumentValues);
    }
    if (selection == "llm-openai-with-image")
    {
        return runOpenAiImage(argumentCount, argumentValues);
    }
    if (selection == "llm-openai")
    {
        return runOpenAi(argumentCount, argumentValues);
    }
    if (selection == "llm-others")
    {
        return runOthers(argumentCount, argumentValues);
    }
    if (selection == "llm-qwen")
    {
        return runQwen(argumentCount, argumentValues);
    }
    if (selection == "pin-input")
    {
        return runPinInput(argumentCount, argumentValues);
    }
    if (selection == "ultrasonic")
    {
        return runUltrasonic(argumentCount, argumentValues);
    }
    if (selection == "voice-assistant")
    {
        return runVoiceAssistant(argumentCount, argumentValues);
    }
    if (selection == "servo")
    {
        return runServo(argumentCount, argumentValues);
    }
    if (selection == "stt-vosk-stream")
    {
        return runSttVoskStream(argumentCount, argumentValues);
    }
    if (selection == "stt-vosk-wake-word-thread")
    {
        return runSttVoskWakeWordThread(argumentCount, argumentValues);
    }
    if (selection == "stt-vosk-wake-word")
    {
        return runSttVoskWakeWord(argumentCount, argumentValues);
    }
    if (selection == "stt-vosk-without-stream")
    {
        return runSttVoskWithoutStream(argumentCount, argumentValues);
    }
    if (selection == "tts-edge")
    {
        return runTtsEdge(argumentCount, argumentValues);
    }
    if (selection == "tts-espeak")
    {
        return runTtsEspeak(argumentCount, argumentValues);
    }
    if (selection == "tts-openai")
    {
        return runTtsOpenAi(argumentCount, argumentValues);
    }
    if (selection == "tts-pico2wave")
    {
        return runTtsPico2Wave(argumentCount, argumentValues);
    }
    if (selection == "tts-piper")
    {
        return runTtsPiper(argumentCount, argumentValues);
    }

    std::cerr << "unknown example: " << selection << '\n';
    printUsage();
    return 2;
}

/**
 * @brief Loads and runs one selector from a YAML argument list.
 *
 * @param[in] executable Executable name retained as argument zero.
 * @param[in] selection Exact key below the YAML `examples` mapping.
 * @param[in] configurationPath Readable YAML configuration path.
 * @return Selected example status, or two for invalid YAML or arguments.
 */
int32 XWalkExampleRunner::runConfigured(
    stringview executable, stringview selection, stringview configurationPath)
{
    try
    {
        const YAML::Node root = YAML::LoadFile(string(configurationPath));
        if (!root.IsMap())
        {
            std::cerr << "xExample YAML configuration is invalid for '"
                      << selection << "': " << configurationPath << '\n';
            return 2;
        }
        const YAML::Node schemaVersion = root["schema_version"];
        const YAML::Node examples = root["examples"];
        YAML::Node example;
        if (examples.IsMap())
        {
            for (YAML::const_iterator iterator = examples.begin();
                 iterator != examples.end(); ++iterator)
            {
                if (iterator->first.IsScalar() &&
                    (iterator->first.as<string>() == selection))
                {
                    example = iterator->second;
                    break;
                }
            }
        }
        const YAML::Node arguments = example.IsMap()
            ? example["arguments"] : YAML::Node();
        if (!schemaVersion.IsScalar() ||
            (schemaVersion.as<uint32>() != 1U) || !examples.IsMap() ||
            !example.IsMap() || !arguments.IsSequence())
        {
            std::cerr << "xExample YAML configuration is invalid for '"
                      << selection << "': " << configurationPath << '\n';
            return 2;
        }

        stringvector values{string(executable), string(selection)};
        for (const YAML::Node& argument : arguments)
        {
            if (!argument.IsScalar())
            {
                std::cerr << "xExample YAML arguments must be scalar values: "
                          << configurationPath << '\n';
                return 2;
            }
            values.push_back(argument.as<string>());
        }

        charpointervector argumentPointers;
        argumentPointers.reserve(values.size() + 1U);
        for (string& value : values)
        {
            argumentPointers.push_back(value.data());
        }
        argumentPointers.push_back(nullptr);
        return runSelection(static_cast<int32>(values.size()),
            argumentPointers.data());
    }
    catch (const YAML::Exception& exception)
    {
        std::cerr << "xExample YAML configuration error: "
                  << exception.what() << '\n';
        return 2;
    }
}

/**
 * @brief Resolves optional YAML configuration and runs one example.
 *
 * @param[in] argumentCount Executable, optional YAML path, selector, and optional formal arguments.
 * @param[in] argumentValues Mutable process argument array.
 * @return Selected example status, or two for invalid input.
 */
int32 XWalkExampleRunner::run(int32 argumentCount, char* argumentValues[])
{
    string configurationPath{XHAL_RPI5CAR_EXAMPLE_YAML_PATH};
    stringvector values;
    boolean configurationSeen = false;
    for (int32 index = 0; index < argumentCount; ++index)
    {
        const string argument(argumentValues[index]);
        if ((index > 0) && (argument == "--config"))
        {
            if (configurationSeen || ((index + 1) >= argumentCount))
            {
                std::cerr << "--config requires one YAML path and may appear once\n";
                return 2;
            }
            configurationPath = argumentValues[index + 1];
            configurationSeen = true;
            ++index;
            continue;
        }
        if ((index > 0) && (argument.rfind("--config=", 0U) == 0U))
        {
            if (configurationSeen || (argument.size() == 9U))
            {
                std::cerr << "--config requires one YAML path and may appear once\n";
                return 2;
            }
            configurationPath = argument.substr(9U);
            configurationSeen = true;
            continue;
        }
        values.push_back(argument);
    }

    if (values.size() < 2U)
    {
        printUsage();
        return 2;
    }
    if (values.size() == 2U)
    {
        return runConfigured(values[0U], values[1U], configurationPath);
    }

    charpointervector argumentPointers;
    argumentPointers.reserve(values.size() + 1U);
    for (string& value : values)
    {
        argumentPointers.push_back(value.data());
    }
    argumentPointers.push_back(nullptr);
    return runSelection(static_cast<int32>(values.size()),
        argumentPointers.data());
}

} /* namespace xwalk::hal::example */
