/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsaHardwareTest.cpp
 * @brief       Provides an opt-in low-volume Raspberry Pi music test.
 *
 * @details
 * Creates the shared ALSA owner and music adapter, selects five-percent mixer
 * volume, and writes one short generated tone without decoding a file.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic ALSA Adapter Hardware Test
 *
 * @author      Joxy John
 * @date        2026-08-01
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

#include "xHal_Rpi5CarMusic.h"
#include "xHal_Rpi5CarMusicAlsa.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Writes one short, low-volume tone through configured ALSA devices.
 *
 * @param[in] argumentCount
 * Either one for defaults or four with PCM, mixer, and element names.
 *
 * @param[in] argumentValues
 * Program name followed optionally by three non-empty ALSA names.
 *
 * @return
 * Zero after playback completes.
 *
 * @warning
 * Run only with the correct Robot HAT and speaker connected at a safe volume.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, char* argumentValues[])
{
    if ((argumentCount != 1) && (argumentCount != 4))
    {
        XHAL_THROW_INVALID_ARGUMENT("Music hardware test accepts PCM, mixer, and element names");
    }
    const XWalkHal::stringview pcmDevice =
        (argumentCount == 4) ? XWalkHal::stringview(argumentValues[1]) : XWalkHal::stringview("default");
    const XWalkHal::stringview mixerDevice =
        (argumentCount == 4) ? XWalkHal::stringview(argumentValues[2]) : XWalkHal::stringview("default");
    const XWalkHal::stringview mixerElement =
        (argumentCount == 4) ? XWalkHal::stringview(argumentValues[3]) : XWalkHal::stringview("PCM");

    XWalkHal::XWalkAudioAlsa audio(pcmDevice, mixerDevice, mixerElement);
    XWalkHal::XWalkMusicAlsa adapter(audio);
    XWalkHal::XWalkMusic music(&adapter, adapter.callbacks());
    music.musicSetVolume(5.0);
    music.playToneFor(220.0, 0.05);
    return 0;
}
