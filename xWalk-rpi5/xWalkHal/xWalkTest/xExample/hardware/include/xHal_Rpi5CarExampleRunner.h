/******************************************************************************
 * @file        xHal_Rpi5CarExampleRunner.h
 * @brief       Declares centralized ported-example CLI dispatch.
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

#ifndef XHAL_RPI5CAR_EXAMPLE_RUNNER_H
#define XHAL_RPI5CAR_EXAMPLE_RUNNER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::example
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Validates and dispatches every xExample command-line selector. */
    class XWalkExampleRunner final
    {
        protected:
            /** @brief Prints every supported example selector and its arguments. */
            static void printUsage();
            /** @brief Validates and runs the physical LED example. */
            static int32 runLed(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the remote DeepSeek example. */
            static int32 runDeepseek(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the text-only remote Doubao example. */
            static int32 runDoubao(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the camera-backed remote Doubao example. */
            static int32 runDoubaoImage(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the remote Gemini example. */
            static int32 runGemini(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the remote Grok example. */
            static int32 runGrok(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the text-only local Ollama example. */
            static int32 runOllama(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the camera-backed local Ollama example. */
            static int32 runOllamaImage(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the camera-backed OpenAI example. */
            static int32 runOpenAiImage(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the text-only OpenAI example. */
            static int32 runOpenAi(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the generic compatible-provider example. */
            static int32 runOthers(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the Qwen example. */
            static int32 runQwen(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs bounded D3 pin-input sampling. */
            static int32 runPinInput(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs bounded D2/D3 ultrasonic ranging. */
            static int32 runUltrasonic(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs bounded multimodal voice-assistant rounds. */
            static int32 runVoiceAssistant(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs bounded channel-one servo sweeps. */
            static int32 runServo(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs bounded offline Vosk recognition sessions. */
            static int32 runSttVoskStream(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs bounded threaded Vosk wake-word detection. */
            static int32 runSttVoskWakeWordThread(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs bounded synchronous Vosk wake detection. */
            static int32 runSttVoskWakeWord(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs bounded non-streaming Vosk recognition. */
            static int32 runSttVoskWithoutStream(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs one live Microsoft Edge TTS request. */
            static int32 runTtsEdge(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs one configured Espeak request. */
            static int32 runTtsEspeak(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs the three fixed OpenAI TTS requests. */
            static int32 runTtsOpenAi(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs one configured Pico2Wave request. */
            static int32 runTtsPico2Wave(int32 argumentCount, char* argumentValues[]);
            /** @brief Validates and runs one configured Piper request. */
            static int32 runTtsPiper(int32 argumentCount, char* argumentValues[]);
            /** @brief Dispatches one selector after its formal arguments are resolved. */
            static int32 runSelection(int32 argumentCount, char* argumentValues[]);
            /** @brief Loads selector arguments from one validated YAML file. */
            static int32 runConfigured(stringview executable, stringview selection, stringview configurationPath);

        public:
            /**
             * @brief Runs one explicitly selected ported example.
             *
             * @param[in] argumentCount Executable, optional YAML path, selector, and optional arguments.
             * @param[in] argumentValues Process arguments in selector-specific order.
             * @return Zero after completion, or two for invalid input.
             * @warning Examples may access hardware or remote network services.
             */
            static int32 run(int32 argumentCount, char* argumentValues[]);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_EXAMPLE_RUNNER_H */
