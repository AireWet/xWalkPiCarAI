/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantExampleLinux.h
 * @brief       Declares Linux composition for the voice-assistant example.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_VOICE_ASSISTANT_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_VOICE_ASSISTANT_EXAMPLE_LINUX_H

#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarLanguageModel.h"
#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarTtsPiperExampleLinux.h"
#include "xHal_Rpi5CarVoiceAssistantExample.h"

namespace xwalk::hal::example
{

    /** @brief Composes Vosk, camera, OpenAI, Piper, and terminal adapters. */
    class XWalkVoiceAssistantExampleLinux final
    {
        private:
            /** @brief Temporarily bound recognizer valid only during `run()`. */
            XWalkSpeechToText* speechToTextObject{nullptr};
            /** @brief Temporarily bound model valid only during `run()`. */
            XWalkLanguageModel* languageModelObject{nullptr};
            /** @brief Temporarily bound camera valid only during `run()`. */
            XWalkCamera* cameraObject{nullptr};
            /** @brief Temporarily bound Piper adapter valid only during `run()`. */
            XWalkTtsPiperExampleLinux* piperObject{nullptr};

        protected:
            /** @brief Resolves a callback context with every live provider binding. */
            static XWalkVoiceAssistantExampleLinux& adapter(contextpointer context);
            /** @brief Applies and validates the exact upstream configuration. */
            static void configure(contextpointer context, const XWalkVoiceAssistantExampleConfiguration& configuration);
            /** @brief Prints `>>> ` and reads one terminal prompt. */
            static boolean readKeyboard(contextpointer context, string& inputText);
            /** @brief Captures and recognizes one bounded microphone utterance. */
            static string listen(contextpointer context, uint32 timeoutMs, stringview language);
            /** @brief Captures one configured camera image. */
            static boolean capture(contextpointer context, stringview imagePath);
            /** @brief Sends one multimodal OpenAI prompt. */
            static string prompt(contextpointer context, stringview model, stringview text, stringview imagePath);
            /** @brief Synthesizes and plays one response through Piper. */
            static void speak(contextpointer context, stringview model, stringview text);
            /** @brief Prints one welcome or response line. */
            static void report(contextpointer context, stringview text);

        public:
            /**
             * @brief Runs bounded live keyboard or wake-word assistant rounds.
             * @param[in] apiKey Non-empty OpenAI credential supplied outside arguments.
             * @param[in] inputMode Keyboard or wake-word source for this run.
             * @param[in] maximumRounds Round limit from one through one hundred.
             * @param[in] timeoutMs Microphone capture timeout in milliseconds.
             * @param[in] microphoneDevice Non-empty ALSA capture device.
             * @param[in] voskLibrary Non-empty Vosk shared-library name or path.
             * @param[in] voskModelPath Non-empty English Vosk model directory.
             * @param[in] piperExecutable Non-empty Piper executable name or path.
             * @param[in] playbackExecutable Non-empty WAV player name or path.
             * @param[in] cameraConnection Exact lowercase `csi` or `usb`.
             * @param[in] captureExecutable Non-empty camera capture executable.
             * @param[in] cameraDevice V4L2 device used only for USB capture.
             * @warning Captures microphone audio and images, contacts OpenAI, and plays audio.
             */
            void run(stringview apiKey,
                     XWalkVoiceAssistantExampleInputMode inputMode,
                     uint32 maximumRounds,
                     uint32 timeoutMs,
                     stringview microphoneDevice,
                     stringview voskLibrary,
                     stringview voskModelPath,
                     stringview piperExecutable,
                     stringview playbackExecutable,
                     stringview cameraConnection,
                     stringview captureExecutable,
                     stringview cameraDevice);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_VOICE_ASSISTANT_EXAMPLE_LINUX_H */
