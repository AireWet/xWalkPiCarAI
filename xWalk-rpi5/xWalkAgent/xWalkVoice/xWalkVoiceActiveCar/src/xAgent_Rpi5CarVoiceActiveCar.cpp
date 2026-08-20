/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCar.cpp
 * @brief       Implements the sensor-aware voice-active PiCar-X loop.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCar
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarVoiceActiveCar.h"

#include "xAgent_Rpi5CarPicarxSafetyGuard.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarWebSearch.h"

#include "xHal_Rpi5CarTrace.h"
#include <cctype>
#include <exception>

namespace xwalk::agent
{

    XWalkVoiceActiveCar::XWalkVoiceActiveCar(XWalkPicarx& picarx,
                                             XWalkSelfDrive& selfDrive,
                                             hal::XWalkVoiceAssistant& assistant,
                                             hal::XWalkLed& led,
                                             agent::contextpointer context,
                                             const XWalkVoiceActiveCarCallbacks& backendCallbacks,
                                             const XWalkVoiceActiveCarConfiguration& carConfiguration)
        : picarxObject(&picarx), selfDriveObject(&selfDrive), assistantObject(&assistant), ledObject(&led),
          callbackContext(context), callbacks(backendCallbacks), configuration(carConfiguration),
          wakeDetectedValue(!carConfiguration.wakeEnabled)
    {
        validate(callbacks, configuration);
    }

    agent::int32 XWalkVoiceActiveCar::run()
    {
        XWalkPicarxSafetyGuard safetyGuard(*picarxObject);
        selfDriveObject->start();
        ledObject->off();
        assistantObject->start();
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .010, "Voice-active-car processing loop started");
        const agent::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const agent::boolean operationMayContinue =
                static_cast<agent::boolean>(callbacks.shouldContinue(callbackContext));
            if (operationMayContinue == false)
            {
                break;
            }
            if (conversationActiveValue)
            {
                const agent::uint32 activeListenTimeout = conversationListenTimeout();
                if (activeListenTimeout == 0U)
                {
                    endConversation(false);
                    continue;
                }
            }
            agent::string prompt;
            agent::boolean ordinaryPrompt{true};
            if (configuration.sensorEnabled)
            {
                prompt = sensorPrompt();
                ordinaryPrompt = static_cast<agent::boolean>(prompt.empty());
            }
            agent::string imagePath;
            const agent::boolean promptEmpty = static_cast<agent::boolean>(prompt.empty());
            if (promptEmpty)
            {
                blink(2U, 100U, 800U);
                if (configuration.inputMode == XWalkVoiceActiveCarInputMode::Keyboard)
                {
                    prompt = callbacks.input(callbackContext, "input: ");
                }
                else
                {
                    const agent::uint32 listenTimeout = conversationListenTimeout();
                    if (listenTimeout == 0U)
                    {
                        endConversation(false);
                        continue;
                    }
                    prompt = assistantObject->listen(listenTimeout);
                }
                const agent::boolean receivedPromptEmpty = static_cast<agent::boolean>(prompt.empty());
                if (receivedPromptEmpty)
                {
                    if (conversationActiveValue)
                    {
                        ++conversationMissCount;
                        const agent::boolean retryLimitReached = static_cast<agent::boolean>(
                            conversationMissCount >= configuration.conversationMaximumMisses);
                        const agent::boolean idleExpired =
                            static_cast<agent::boolean>(conversationListenTimeout() == 0U);
                        if (retryLimitReached || idleExpired)
                        {
                            endConversation(false);
                        }
                    }
                    continue;
                }
                if (configuration.wakeEnabled && !wakeDetectedValue)
                {
                    const agent::boolean wakePhraseMatched = static_cast<agent::boolean>(isWakePhrase(prompt));
                    if (wakePhraseMatched)
                    {
                        wakeDetectedValue = true;
                        if (configuration.continuousConversationEnabled)
                        {
                            startConversation();
                        }
                        ledObject->on();
                        assistantObject->say(configuration.answerOnWake);
                        ledObject->off();
                        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .011,
                                                  "Voice-active-car wake phrase accepted and acknowledged");
                    }
                    continue;
                }
                conversationMissCount = 0U;
                const agent::boolean sleepRequested =
                    static_cast<agent::boolean>(conversationActiveValue && isSleepPhrase(prompt));
                if (sleepRequested)
                {
                    endConversation(true);
                    continue;
                }
                if (configuration.withImage && (callbacks.captureImage != nullptr))
                {
                    imagePath = callbacks.captureImage(callbackContext);
                }
            }
            blink(1U, 100U, 0U);
            selfDriveObject->setStatus(XWalkSelfDriveStatus::Think);
            agent::boolean retrievalUsed{false};
            if (ordinaryPrompt && configuration.webSearchEnabled && hal::XWalkWebSearch::shouldSearch(prompt))
            {
                try
                {
                    const hal::XWalkWebSearchResponse search = callbacks.webSearch(callbacks.webSearchContext, prompt);
                    retrievalUsed = static_cast<agent::boolean>(!search.referenceText.empty());
                    if (retrievalUsed)
                    {
                        prompt += search.referenceText;
                        for (agent::size index = 0U; index < search.sourceUrls.size(); ++index)
                        {
                            callbacks.output(callbackContext,
                                             agent::string("Web source: ") + search.sourceNames[index] + " " +
                                                 search.sourceUrls[index]);
                        }
                    }
                }
                catch (const std::exception&)
                {
                    callbacks.output(callbackContext, "Current information could not be retrieved.");
                    prompt += "\n\nCurrent web information was requested but retrieval failed. Clearly identify any "
                              "answer as local model knowledge and do not invent current facts or sources.";
                    XWALK_RPIAGENT_WARNING(XWALK_RUNTIME,
                                           "Voice-active-car web retrieval failed; continuing with local knowledge");
                }
            }
            XWalkVoiceActiveCarResponse response{};
            try
            {
                response = parseConfiguredResponse(assistantObject->think(prompt, imagePath));
            }
            catch (const std::exception&)
            {
                picarxObject->stop();
                conversationActiveValue = false;
                conversationRoundCount = 0U;
                conversationMissCount = 0U;
                conversationDeadlineMs = 0U;
                wakeDetectedValue = !configuration.wakeEnabled;
                ledObject->off();
                constexpr agent::stringview recoveryText =
                    "The language model is unavailable. Please check the local service and try again.";
                callbacks.output(callbackContext, recoveryText);
                XWALK_RPIAGENT_WARNING(XWALK_RUNTIME,
                                       "Voice-active-car language-model request failed; returning to wake mode");
                try
                {
                    assistantObject->say(recoveryText);
                }
                catch (...)
                {
                    XWALK_RPIAGENT_WARNING(XWALK_RUNTIME,
                                           "Voice-active-car could not speak the model failure acknowledgement");
                }
                continue;
            }
            if (retrievalUsed)
            {
                response.actions = {"stop"};
            }
            XWALK_RPIAGENT_TRACE_UID2(RPIAGENT .012,
                                      "Voice-active-car parsed %llu action request(s) and %llu response character(s)",
                                      static_cast<unsigned long long>(response.actions.size()),
                                      static_cast<unsigned long long>(response.text.size()));
            selfDriveObject->setStatus(XWalkSelfDriveStatus::Actions);
            dispatchActions(response.actions);
            ledObject->on();
            assistantObject->say(response.text);
            const agent::boolean actionsCompleted = selfDriveObject->waitActionsDone();
            if (actionsCompleted == false)
            {
                stop();
                return 1;
            }
            XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .013, "Voice-active-car assistant round completed");
            ledObject->off();
            if (conversationActiveValue && ordinaryPrompt)
            {
                ++conversationRoundCount;
                const agent::boolean roundLimitReached =
                    static_cast<agent::boolean>(conversationRoundCount >= configuration.conversationMaximumRounds);
                if (roundLimitReached)
                {
                    endConversation(false);
                }
                else
                {
                    const agent::uint64 currentMilliseconds = callbacks.monotonicMilliseconds(callbackContext);
                    conversationDeadlineMs = currentMilliseconds + configuration.conversationIdleTimeoutMs;
                }
            }
            else if (ordinaryPrompt)
            {
                wakeDetectedValue = !configuration.wakeEnabled;
            }
        }
        stop();
        return 0;
    }

    void XWalkVoiceActiveCar::stop()
    {
        assistantObject->stop();
        selfDriveObject->stop();
        picarxObject->close();
        ledObject->off();
        wakeDetectedValue = !configuration.wakeEnabled;
        conversationActiveValue = false;
        conversationRoundCount = 0U;
        conversationMissCount = 0U;
        conversationDeadlineMs = 0U;
    }

    XWalkVoiceActiveCarResponse XWalkVoiceActiveCar::parseResponse(agent::stringview response)
    {
        constexpr agent::stringview delimiter{"ACTIONS: "};
        const agent::size actionStart = response.find(delimiter);
        XWalkVoiceActiveCarResponse result;
        result.text = trimText(response.substr(0U, actionStart));
        if (actionStart != agent::string::npos)
        {
            agent::string actions = trimText(response.substr(actionStart + delimiter.size()));
            const agent::boolean actionParsingRequested{true};
            while (actionParsingRequested)
            {
                const agent::boolean actionsAvailable = static_cast<agent::boolean>(!actions.empty());
                if (actionsAvailable == false)
                {
                    break;
                }
                const agent::size separator = actions.find(", ");
                result.actions.push_back(trimText(actions.substr(0U, separator)));
                if (separator == agent::string::npos)
                {
                    actions.clear();
                }
                else
                {
                    actions.erase(0U, separator + 2U);
                }
            }
        }
        const agent::boolean actionsEmpty = static_cast<agent::boolean>(result.actions.empty());
        if (actionsEmpty)
        {
            result.actions.emplace_back("stop");
        }
        return result;
    }

    /**
     * @brief Parses the JSON object returned by the upstream GPT-car assistant.
     * @param[in] response Model response containing `actions` and `answer` values.
     * @return Owned answer and action names; unrecognized text is retained as the
     * answer.
     */
    XWalkVoiceActiveCarResponse XWalkVoiceActiveCar::parseJsonResponse(agent::stringview response)
    {
        XWalkVoiceActiveCarResponse result;
        const auto parseString = [&response](agent::size& position) -> agent::string
        {
            agent::string value;
            const agent::boolean positionResponseInvalid =
                static_cast<agent::boolean>((position >= response.size()) || (response[position] != '"'));
            if (positionResponseInvalid)
            {
                return value;
            }
            ++position;
            const agent::boolean stringDecodingRequested{true};
            while (stringDecodingRequested)
            {
                const agent::boolean responseCharacterAvailable =
                    static_cast<agent::boolean>(position < response.size());
                if (responseCharacterAvailable == false)
                {
                    break;
                }
                const char character = response[position++];
                if (character == '"')
                {
                    return value;
                }
                const agent::boolean escapeCharacterFound =
                    static_cast<agent::boolean>((character == '\\') && (position < response.size()));
                if (escapeCharacterFound)
                {
                    const char escaped = response[position++];
                    if (escaped == 'n')
                    {
                        value.push_back('\n');
                    }
                    else if (escaped == 't')
                    {
                        value.push_back('\t');
                    }
                    else
                    {
                        value.push_back(escaped);
                    }
                }
                else
                {
                    value.push_back(character);
                }
            }
            return {};
        };
        const auto valuePosition = [&response](agent::stringview key) -> agent::size
        {
            const agent::string quotedKey = agent::string("\"") + agent::string(key) + "\"";
            const agent::size keyPosition = response.find(quotedKey);
            if (keyPosition == agent::string::npos)
            {
                return keyPosition;
            }
            agent::size position = response.find(':', keyPosition + quotedKey.size());
            if (position == agent::string::npos)
            {
                return position;
            }
            ++position;
            const agent::boolean whitespaceParsingRequested{true};
            while (whitespaceParsingRequested)
            {
                const agent::boolean whitespaceAvailable = static_cast<agent::boolean>(
                    (position < response.size()) && ((response[position] == ' ') || (response[position] == '\t') ||
                                                     (response[position] == '\r') || (response[position] == '\n')));
                if (whitespaceAvailable == false)
                {
                    break;
                }
                ++position;
            }
            return position;
        };

        agent::size answerPosition = valuePosition("answer");
        if (answerPosition != agent::string::npos)
        {
            result.text = parseString(answerPosition);
        }
        agent::size actionsPosition = valuePosition("actions");
        const agent::boolean actionArrayFound =
            static_cast<agent::boolean>((actionsPosition != agent::string::npos) &&
                                        (actionsPosition < response.size()) && (response[actionsPosition] == '['));
        if (actionArrayFound)
        {
            ++actionsPosition;
            const agent::boolean actionArrayParsingRequested{true};
            while (actionArrayParsingRequested)
            {
                const agent::boolean actionCharacterAvailable =
                    static_cast<agent::boolean>(actionsPosition < response.size());
                if (actionCharacterAvailable == false)
                {
                    break;
                }
                const agent::boolean actionWhitespaceParsingRequested{true};
                while (actionWhitespaceParsingRequested)
                {
                    const agent::boolean actionsPositionResponseTRNInvalid = static_cast<agent::boolean>(
                        (actionsPosition < response.size()) &&
                        ((response[actionsPosition] == ' ') || (response[actionsPosition] == '\t') ||
                         (response[actionsPosition] == '\r') || (response[actionsPosition] == '\n') ||
                         (response[actionsPosition] == ',')));
                    if (actionsPositionResponseTRNInvalid == false)
                    {
                        break;
                    }
                    ++actionsPosition;
                }
                const agent::boolean actionsPositionResponseInvalid = static_cast<agent::boolean>(
                    (actionsPosition >= response.size()) || (response[actionsPosition] == ']'));
                if (actionsPositionResponseInvalid)
                {
                    break;
                }
                const agent::string action = parseString(actionsPosition);
                const agent::boolean actionEmpty = static_cast<agent::boolean>(action.empty());
                if (actionEmpty)
                {
                    break;
                }
                result.actions.push_back(action);
            }
        }
        const agent::boolean responseEmpty = static_cast<agent::boolean>(result.text.empty() && result.actions.empty());
        if (responseEmpty)
        {
            result.text = trimText(response);
        }
        return result;
    }

    /**
     * @brief Parses one response according to the selected profile syntax.
     * @param[in] response Final model response retained only for this call.
     * @return Owned answer and action names.
     */
    XWalkVoiceActiveCarResponse XWalkVoiceActiveCar::parseConfiguredResponse(agent::stringview response) const
    {
        if (configuration.responseFormat == XWalkVoiceActiveCarResponseFormat::Json)
        {
            return parseJsonResponse(response);
        }
        return parseResponse(response);
    }

    /**
     * @brief Selects voice or keyboard input before starting the foreground loop.
     * @param[in] inputMode Input source used by later rounds.
     */
    void XWalkVoiceActiveCar::setInputMode(XWalkVoiceActiveCarInputMode inputMode) noexcept
    {
        configuration.inputMode = inputMode;
    }

    /**
     * @brief Enables or disables image attachment before starting the foreground
     * loop.
     * @param[in] enabled `true` to request a captured still image for ordinary
     * prompts.
     */
    void XWalkVoiceActiveCar::setImageEnabled(agent::boolean enabled) noexcept
    {
        configuration.withImage = enabled;
    }

    void XWalkVoiceActiveCar::blink(agent::uint32 count, agent::uint32 toggleDelayMs, agent::uint32 pauseMs)
    {
        for (agent::uint32 index = 0U; index < count; ++index)
        {
            ledObject->on();
            callbacks.delay(callbackContext, toggleDelayMs);
            ledObject->off();
            callbacks.delay(callbackContext, toggleDelayMs);
        }
        if (pauseMs > 0U)
        {
            callbacks.delay(callbackContext, pauseMs);
        }
    }

    /**
     * @brief Queues parsed model actions for serialized worker execution.
     *
     * @details
     * The `stop` action uses the same worker queue as movement gestures so it
     * cannot access PiCar-X actuators concurrently with an active thinking pose.
     *
     * @param[in] actions
     * Exact lowercase action names retained for this call.
     */
    void XWalkVoiceActiveCar::dispatchActions(const agent::stringvector& actions)
    {
        agent::size acceptedActionCount{};
        for (const agent::string& action : actions)
        {
            const agent::boolean actionAdded = selfDriveObject->addAction(action);
            if (actionAdded == false)
            {
                XWALK_RPIAGENT_WARNING(XWALK_INVAL, "Voice-active-car rejected an unsupported model action");
                callbacks.output(callbackContext, agent::string("Unsupported voice action: ") + action);
            }
            else
            {
                ++acceptedActionCount;
            }
        }
        XWALK_RPIAGENT_TRACE_UID2(RPIAGENT .015,
                                  "Voice-active-car queued %llu of %llu parsed action request(s)",
                                  static_cast<unsigned long long>(acceptedActionCount),
                                  static_cast<unsigned long long>(actions.size()));
    }

    agent::string XWalkVoiceActiveCar::sensorPrompt()
    {
        const agent::float64 distanceCm = picarxObject->distance();
        if ((distanceCm > 1.0) && (distanceCm < configuration.tooCloseCm))
        {
            static_cast<void>(selfDriveObject->addAction("backward"));
            const agent::string distance = hal::common::float64ToString(distanceCm);
            callbacks.output(callbackContext, agent::string("Ultrasonic sense too close: ") + distance + "cm");
            XWALK_RPIAGENT_TRACE_UID1(
                RPIAGENT .014, "Voice-active-car proximity response triggered at %.2f cm", distanceCm);
            return agent::string("<<<Ultrasonic sense too close: ") + distance + "cm>>>";
        }
        return {};
    }

    /**
     * @brief Checks one recognition result for the configured wake phrase.
     * @param[in] text Recognized text retained only for this call.
     * @return `true` when `text` contains the wake phrase without case sensitivity.
     */
    agent::boolean XWalkVoiceActiveCar::isWakePhrase(agent::stringview text) const
    {
        return matchesWakePhrase(text, configuration.wakeWord);
    }

    /**
     * @brief Checks a transcript for a wake phrase without case sensitivity.
     * @param[in] text Recognized transcript retained only for this call.
     * @param[in] wakePhrase Non-empty configured wake phrase.
     * @return `true` when the complete wake phrase occurs within the transcript.
     */
    agent::boolean XWalkVoiceActiveCar::matchesWakePhrase(agent::stringview text, agent::stringview wakePhrase)
    {
        agent::string normalizedText(text);
        agent::string normalizedWakeWord(wakePhrase);
        for (char& value : normalizedText)
        {
            value = static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
        }
        for (char& value : normalizedWakeWord)
        {
            value = static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
        }
        return normalizedText.find(normalizedWakeWord) != agent::string::npos;
    }

    /**
     * @brief Trims surrounding ASCII whitespace from one text view.
     * @param[in] text Text retained only for this synchronous call.
     * @return Owned trimmed text, or an empty string when no non-whitespace character exists.
     */
    agent::string XWalkVoiceActiveCar::trimText(agent::stringview text)
    {
        const agent::size first = text.find_first_not_of(" \t\r\n");
        if (first == agent::string::npos)
        {
            return {};
        }
        const agent::size last = text.find_last_not_of(" \t\r\n");
        return agent::string(text.substr(first, (last - first) + 1U));
    }

    /**
     * @brief Normalizes one policy phrase for exact case-insensitive comparison.
     * @param[in] text Phrase retained only for this synchronous call.
     * @return Owned trimmed lowercase phrase.
     */
    agent::string XWalkVoiceActiveCar::normalizePhrase(agent::stringview text)
    {
        agent::string normalized = trimText(text);
        for (char& value : normalized)
        {
            value = static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
        }
        return normalized;
    }

    /**
     * @brief Checks one prompt against every configured sleep phrase.
     * @param[in] text Recognized prompt retained only for this synchronous call.
     * @return `true` for an exact trimmed case-insensitive sleep-phrase match.
     */
    agent::boolean XWalkVoiceActiveCar::isSleepPhrase(agent::stringview text) const
    {
        const agent::string normalizedText = normalizePhrase(text);
        for (const agent::string& phrase : configuration.sleepPhrases)
        {
            const agent::boolean phraseMatched = static_cast<agent::boolean>(normalizedText == normalizePhrase(phrase));
            if (phraseMatched)
            {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Starts one bounded continuous conversation after wake recognition.
     * @post Follow-up prompts are accepted until an idle, round, miss, sleep, or shutdown boundary.
     */
    void XWalkVoiceActiveCar::startConversation()
    {
        conversationActiveValue = true;
        conversationRoundCount = 0U;
        conversationMissCount = 0U;
        const agent::uint64 currentMilliseconds = callbacks.monotonicMilliseconds(callbackContext);
        conversationDeadlineMs = currentMilliseconds + configuration.conversationIdleTimeoutMs;
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .088, "Voice-active-car continuous conversation started");
    }

    /**
     * @brief Returns to wake mode and stops vehicle movement.
     * @param[in] acknowledge `true` to speak the configured explicit-sleep acknowledgement.
     * @post Continuous-session counters are cleared and later ordinary input requires the wake phrase.
     */
    void XWalkVoiceActiveCar::endConversation(agent::boolean acknowledge)
    {
        picarxObject->stop();
        conversationActiveValue = false;
        conversationRoundCount = 0U;
        conversationMissCount = 0U;
        conversationDeadlineMs = 0U;
        wakeDetectedValue = !configuration.wakeEnabled;
        ledObject->off();
        const agent::boolean acknowledgementAvailable =
            static_cast<agent::boolean>(acknowledge && !configuration.sleepAcknowledgement.empty());
        if (acknowledgementAvailable)
        {
            assistantObject->say(configuration.sleepAcknowledgement);
        }
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .089, "Voice-active-car continuous conversation ended safely");
    }

    /**
     * @brief Bounds one recognition request by the active conversation deadline.
     * @return Non-zero listen timeout in milliseconds, or zero after idle expiry.
     */
    agent::uint32 XWalkVoiceActiveCar::conversationListenTimeout()
    {
        if (conversationActiveValue == false)
        {
            return configuration.listenTimeoutMs;
        }
        const agent::uint64 currentMilliseconds = callbacks.monotonicMilliseconds(callbackContext);
        if (currentMilliseconds >= conversationDeadlineMs)
        {
            XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .090, "Voice-active-car conversation idle timeout reached");
            return 0U;
        }
        const agent::uint64 remainingMilliseconds = conversationDeadlineMs - currentMilliseconds;
        const agent::uint64 configuredTimeout = configuration.listenTimeoutMs;
        const agent::uint64 boundedTimeout = std::min(remainingMilliseconds, configuredTimeout);
        return static_cast<agent::uint32>(boundedTimeout);
    }

    void XWalkVoiceActiveCar::validate(const XWalkVoiceActiveCarCallbacks& backendCallbacks,
                                       const XWalkVoiceActiveCarConfiguration& carConfiguration)
    {
        if ((backendCallbacks.output == nullptr) || (backendCallbacks.shouldContinue == nullptr) ||
            (backendCallbacks.delay == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Voice-active-car callbacks must be complete");
        }
        if ((carConfiguration.inputMode == XWalkVoiceActiveCarInputMode::Keyboard) &&
            (backendCallbacks.input == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Voice-active-car keyboard input callback is required");
        }
        if (carConfiguration.webSearchEnabled &&
            ((backendCallbacks.webSearch == nullptr) || (backendCallbacks.webSearchContext == nullptr)))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Voice-active-car enabled web search requires a live callback");
        }
        const agent::boolean continuousConfigurationInvalid = static_cast<agent::boolean>(
            carConfiguration.continuousConversationEnabled &&
            ((!carConfiguration.wakeEnabled) || (backendCallbacks.monotonicMilliseconds == nullptr) ||
             (carConfiguration.conversationIdleTimeoutMs == 0U) ||
             (carConfiguration.conversationIdleTimeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS) ||
             (carConfiguration.conversationMaximumRounds == 0U) ||
             (carConfiguration.conversationMaximumRounds > 100U) ||
             (carConfiguration.conversationMaximumMisses == 0U) || (carConfiguration.conversationMaximumMisses > 10U) ||
             carConfiguration.sleepPhrases.empty()));
        if (continuousConfigurationInvalid)
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "Voice-active-car conversation configuration is outside its range");
        }
        for (agent::size index{}; index < carConfiguration.sleepPhrases.size(); ++index)
        {
            const agent::string normalizedPhrase = normalizePhrase(carConfiguration.sleepPhrases[index]);
            if (normalizedPhrase.empty())
            {
                XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Voice-active-car sleep phrases must not be empty");
            }
            for (agent::size prior{}; prior < index; ++prior)
            {
                const agent::boolean duplicatePhrase = static_cast<agent::boolean>(
                    normalizedPhrase == normalizePhrase(carConfiguration.sleepPhrases[prior]));
                if (duplicatePhrase)
                {
                    XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Voice-active-car sleep phrases must be unique");
                }
            }
        }
        if ((carConfiguration.tooCloseCm <= 1.0) || (carConfiguration.listenTimeoutMs == 0U) ||
            (carConfiguration.listenTimeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "Voice-active-car configuration is outside its range");
        }
        const agent::boolean wakeWordMissing =
            static_cast<agent::boolean>(carConfiguration.wakeEnabled && carConfiguration.wakeWord.empty());
        if (wakeWordMissing)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL,
                                 "Voice-active-car wake word is required "
                                 "when wake detection is enabled");
        }
    }

} /* namespace xwalk::agent */
