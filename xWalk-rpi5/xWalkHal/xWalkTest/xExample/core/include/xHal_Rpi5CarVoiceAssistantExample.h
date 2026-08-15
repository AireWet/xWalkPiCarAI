/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantExample.h
 * @brief       Declares the bounded multimodal voice-assistant example.
 *
 * @details
 * Preserves the active upstream assistant configuration while injecting input,
 * image, model, speech, and reporting operations for host and Linux adapters.
 *
 * @project     xWalk Firmware
 * @module      xExample
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_VOICE_ASSISTANT_EXAMPLE_H
#define XHAL_RPI5CAR_VOICE_ASSISTANT_EXAMPLE_H

#include "xHal_Rpi5CarCommon.h"

#define XHAL_RPI5CAR_VOICE_ASSISTANT_EXAMPLE_MAXIMUM_ROUNDS 100U

namespace xwalk::hal::example
{

    /** @brief Selects one bounded source from the two enabled upstream inputs. */
    enum class XWalkVoiceAssistantExampleInputMode : uint8
    {
        /** @brief Reads prompts from the terminal. */
        Keyboard = 0U,
        /** @brief Waits for `hey buddy`, acknowledges it, and listens for a prompt. */
        WakeWord = 1U
    };

    /** @brief Owns the exact active settings from the upstream Python example. */
    struct XWalkVoiceAssistantExampleConfiguration
    {
            /** @brief Assistant name used by wake and welcome text. */
            string name{"Buddy"};
            /** @brief Piper voice model used for every spoken message. */
            string ttsModel{"en_US-ryan-low"};
            /** @brief OpenAI chat model used for every prompt. */
            string languageModel{"gpt-4o-mini"};
            /** @brief Speech-recognition language selected by the source. */
            string speechLanguage{"en-us"};
            /** @brief Exact enabled wake phrase. */
            string wakeWord{"hey buddy"};
            /** @brief Exact acknowledgement spoken after wake detection. */
            string answerOnWake{"Hi there"};
            /** @brief Exact startup welcome message. */
            string welcome{"Hi, I'm Buddy. Wake me up with: hey buddy"};
            /** @brief Exact source system instructions, including surrounding newlines. */
            string instructions{"\nYou are a helpful assistant, named Buddy.\n"};
            /** @brief Whether prompts include a newly captured image. */
            boolean withImage{true};
            /** @brief Whether terminal input is enabled by the source. */
            boolean keyboardEnabled{true};
            /** @brief Whether wake-word input is enabled by the source. */
            boolean wakeEnabled{true};
    };

    using voiceassistantexampleconfigurecallback =
        void (*)(contextpointer context, const XWalkVoiceAssistantExampleConfiguration& configuration);
    using voiceassistantexamplereadcallback = boolean (*)(contextpointer context, string& inputText);
    using voiceassistantexamplelistencallback = string (*)(contextpointer context,
                                                           uint32 timeoutMs,
                                                           stringview language);
    using voiceassistantexamplecapturecallback = boolean (*)(contextpointer context, stringview imagePath);
    using voiceassistantexamplepromptcallback = string (*)(contextpointer context,
                                                           stringview model,
                                                           stringview text,
                                                           stringview imagePath);
    using voiceassistantexamplespeakcallback = void (*)(contextpointer context, stringview model, stringview text);
    using voiceassistantexamplereportcallback = void (*)(contextpointer context, stringview text);

    /** @brief Complete injected operation table required by the assistant example. */
    struct XWalkVoiceAssistantExampleCallbacks
    {
            /** @brief Applies the complete fixed source configuration. */
            voiceassistantexampleconfigurecallback configure{nullptr};
            /** @brief Reads one keyboard prompt and reports end of input. */
            voiceassistantexamplereadcallback readKeyboard{nullptr};
            /** @brief Captures and recognizes one bounded microphone utterance. */
            voiceassistantexamplelistencallback listen{nullptr};
            /** @brief Captures one image at the requested path. */
            voiceassistantexamplecapturecallback capture{nullptr};
            /** @brief Sends one text and optional image prompt to the model. */
            voiceassistantexamplepromptcallback prompt{nullptr};
            /** @brief Synthesizes and plays one message through Piper. */
            voiceassistantexamplespeakcallback speak{nullptr};
            /** @brief Reports one welcome or response string. */
            voiceassistantexamplereportcallback report{nullptr};
    };

    /** @brief Runs bounded keyboard or wake-driven multimodal assistant rounds. */
    class XWalkVoiceAssistantExample final
    {
        private:
            /** @brief Non-owning context forwarded synchronously to every operation. */
            contextpointer callbackContext;
            /** @brief Complete validated operation table copied during construction. */
            XWalkVoiceAssistantExampleCallbacks callbacks;
            /** @brief Exact active source configuration owned by this example. */
            XWalkVoiceAssistantExampleConfiguration configurationValue;

        protected:
            /** @brief Acquires one prompt through the selected enabled input mode. */
            boolean acquirePrompt(XWalkVoiceAssistantExampleInputMode inputMode, uint32 timeoutMs, string& promptText);

        public:
            /**
             * @brief Binds all operations required by the active source configuration.
             * @param[in,out] context Non-owning context forwarded to every callback.
             * @param[in] exampleCallbacks Table containing seven non-null operations.
             * @throws std::invalid_argument If any operation is null.
             */
            XWalkVoiceAssistantExample(contextpointer context,
                                       const XWalkVoiceAssistantExampleCallbacks& exampleCallbacks);

            XWalkVoiceAssistantExample(const XWalkVoiceAssistantExample&) = delete;
            XWalkVoiceAssistantExample(XWalkVoiceAssistantExample&&) = delete;
            XWalkVoiceAssistantExample& operator=(const XWalkVoiceAssistantExample&) = delete;
            XWalkVoiceAssistantExample& operator=(XWalkVoiceAssistantExample&&) = delete;

            /**
             * @brief Runs a bounded number of keyboard or wake-driven assistant rounds.
             * @param[in] inputMode Keyboard or wake-word source selected for this run.
             * @param[in] maximumRounds Round limit from one through one hundred.
             * @param[in] timeoutMs Microphone capture timeout from 1 through 300,000 ms.
             * @param[in] imagePath Non-empty capture path required when images are enabled.
             * @throws std::out_of_range If a numeric bound is invalid.
             * @throws std::invalid_argument If the selected mode or path is invalid.
             * @throws std::runtime_error If an enabled image capture fails.
             */
            void run(XWalkVoiceAssistantExampleInputMode inputMode,
                     uint32 maximumRounds,
                     uint32 timeoutMs,
                     stringview imagePath);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_VOICE_ASSISTANT_EXAMPLE_H */
