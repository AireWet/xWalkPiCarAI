/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechPiper.h
 * @brief       Declares the shell-free Piper speech provider.
 * @project     xWalk Firmware
 * @module      xWalkGPT Piper Provider
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_TEXT_TO_SPEECH_PIPER_H
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_PIPER_H

#include "xHal_Rpi5CarTextToSpeechTypes.h"

namespace xwalk::hal
{

/** @brief Synthesizes with Piper and plays one private temporary WAV. */
class XWalkTextToSpeechPiper final
{
private:
    string executableName{};
    string playbackExecutableName{};
    string modelName{};

protected:
    static XWalkTextToSpeechPiper& provider(contextpointer context);
    static void speak(contextpointer context, stringview text);
    void execute(stringview text) const;

public:
    XWalkTextToSpeechPiper(stringview executable,
        stringview playbackExecutable, stringview model);
    ~XWalkTextToSpeechPiper() = default;

    XWalkTextToSpeechPiper(const XWalkTextToSpeechPiper&) = delete;
    XWalkTextToSpeechPiper(XWalkTextToSpeechPiper&&) = delete;
    XWalkTextToSpeechPiper& operator=(const XWalkTextToSpeechPiper&) = delete;
    XWalkTextToSpeechPiper& operator=(XWalkTextToSpeechPiper&&) = delete;

    texttospeechspeakcallback callback() const noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_TEXT_TO_SPEECH_PIPER_H */
