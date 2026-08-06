/******************************************************************************
 * @file        xAgent_Rpi5CarTextVisionTalkLifecycle.cpp
 * @brief       Implements text-vision-talk construction and validation.
 * @project     xWalk Firmware
 * @module      xWalkTextVisionTalk
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarTextVisionTalk.h"

#include "xHal_Rpi5CarExceptions.h"

#include <algorithm>
#include <cctype>

/** @namespace xwalk::agent @brief Contains application coordinators for xWalk firmware. */
namespace xwalk::agent
{

/**
 * @brief Binds caller-owned model, camera, callbacks, and conversation settings.
 * @param[in,out] languageModel Model coordinator that must outlive this Agent.
 * @param[in,out] cameraCapture Capture Agent that must outlive this Agent.
 * @param[in,out] context Nullable context that must outlive callback use.
 * @param[in] backendCallbacks Complete synchronous callback table.
 * @param[in] talkConfiguration Owned source-compatible settings.
 */
XWalkTextVisionTalk::XWalkTextVisionTalk(hal::XWalkLanguageModel& languageModel,
    XWalkCameraCapture& cameraCapture, agent::contextpointer context,
    const XWalkTextVisionTalkCallbacks& backendCallbacks,
    const XWalkTextVisionTalkConfiguration& talkConfiguration):
    languageModelObject(&languageModel), cameraCaptureObject(&cameraCapture),
    callbackContext(context), callbacks(backendCallbacks),
    configuration(talkConfiguration)
{
    validate(callbacks, configuration);
}

/**
 * @brief Trims and lowercases interactive control text.
 * @param[in] text Text to normalize.
 * @return Owned normalized text.
 */
agent::string XWalkTextVisionTalk::normalize(agent::stringview text)
{
    agent::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), [](char value)
    {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
    });
    const auto isNotSpace = [](char value)
    {
        return std::isspace(static_cast<unsigned char>(value)) == 0;
    };
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), isNotSpace));
    result.erase(std::find_if(result.rbegin(), result.rend(), isNotSpace).base(), result.end());
    return result;
}

/**
 * @brief Validates callbacks and bounded source configuration.
 * @param[in] backendCallbacks Callback table requiring four non-null functions.
 * @param[in] talkConfiguration Settings requiring a prompt and non-zero bounds.
 * @throws std::invalid_argument If a callback or prompt is missing.
 * @throws std::out_of_range If a numeric setting is zero.
 */
void XWalkTextVisionTalk::validate(
    const XWalkTextVisionTalkCallbacks& backendCallbacks,
    const XWalkTextVisionTalkConfiguration& talkConfiguration)
{
    if ((backendCallbacks.output == nullptr) || (backendCallbacks.input == nullptr) ||
        (backendCallbacks.delay == nullptr) ||
        (backendCallbacks.shouldContinue == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Text-vision-talk callbacks must be complete");
    }
    const agent::boolean promptTextEmpty =
        static_cast<agent::boolean>(
            talkConfiguration.promptText.empty());
    if (promptTextEmpty)
    {
        XHAL_THROW_INVALID_ARGUMENT("Text-vision-talk prompt must not be empty");
    }
    if ((talkConfiguration.maximumMessages == 0U) ||
        (talkConfiguration.cameraWarmupMs == 0U))
    {
        XHAL_THROW_OUT_OF_RANGE("Text-vision-talk configuration is outside its range");
    }
}

} /* namespace xwalk::agent */
