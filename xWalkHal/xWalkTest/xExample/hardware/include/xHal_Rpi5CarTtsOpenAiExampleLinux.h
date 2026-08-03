/******************************************************************************
 * @file        xHal_Rpi5CarTtsOpenAiExampleLinux.h
 * @brief       Declares Linux composition for the OpenAI TTS example.
 *
 * @details
 * Owns one credential, endpoint, playback executable, and bounded response
 * buffer used by synchronous OpenAI speech requests and shell-free playback.
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

#ifndef XHAL_RPI5CAR_TTS_OPEN_AI_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_TTS_OPEN_AI_EXAMPLE_LINUX_H

#include "xHal_Rpi5CarTtsOpenAiExample.h"

namespace xwalk::hal::example
{

/** @brief Executes bounded OpenAI synthesis and local MP3 playback. */
class XWalkTtsOpenAiExampleLinux final
{
private:

    /** @brief Owned non-empty OpenAI credential. */
    string apiKeyValue;
    /** @brief Owned non-empty speech endpoint. */
    string endpointValue;
    /** @brief Owned non-empty playback executable name or path. */
    string executableName;
    /** @brief Bounded binary response used by the current request. */
    string responseData;

protected:

    /** @brief Resolves a callback context into its required Linux adapter. */
    static XWalkTtsOpenAiExampleLinux& adapter(contextpointer context);
    /** @brief Escapes one bounded value for a JSON string literal. */
    static string escapeJson(stringview value);
    /** @brief Appends one bounded libcurl audio response block. */
    static size writeResponse(charpointer data, size itemSize,
        size itemCount, contextpointer context);
    /** @brief Synthesizes and plays one OpenAI speech request. */
    static void speak(contextpointer context, stringview model,
        stringview voice, stringview text, stringview instructions);
    /** @brief Prints one source-compatible report line. */
    static void report(contextpointer context, stringview message);
    /** @brief Posts one request and replaces the bounded audio buffer. */
    void requestSpeech(stringview model, stringview voice,
        stringview text, stringview instructions);
    /** @brief Plays the current MP3 response through a temporary file. */
    void playResponse();

public:

    /**
     * @brief Stores deployment-selected OpenAI and playback configuration.
     * @param[in] apiKey Non-empty OpenAI API credential.
     * @param[in] executable Non-empty MP3 playback executable name or path.
     * @param[in] endpoint Non-empty OpenAI-compatible speech endpoint.
     * @throws std::invalid_argument If any required value is empty.
     */
    XWalkTtsOpenAiExampleLinux(stringview apiKey, stringview executable,
        stringview endpoint = "https://api.openai.com/v1/audio/speech");

    XWalkTtsOpenAiExampleLinux(const XWalkTtsOpenAiExampleLinux&) = delete;
    XWalkTtsOpenAiExampleLinux(XWalkTtsOpenAiExampleLinux&&) = delete;
    XWalkTtsOpenAiExampleLinux& operator=(
        const XWalkTtsOpenAiExampleLinux&) = delete;
    XWalkTtsOpenAiExampleLinux& operator=(XWalkTtsOpenAiExampleLinux&&) = delete;

    /**
     * @brief Runs the three exact source speech requests in order.
     * @warning Uses a billable remote service and produces audible output.
     */
    void run();
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_TTS_OPEN_AI_EXAMPLE_LINUX_H */
