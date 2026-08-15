/******************************************************************************
 * @file        xHal_Rpi5CarAudioAlsaHardwareTest.cpp
 * @brief       Verifies silent PCM and mixer operations on Raspberry Pi ALSA.
 *
 * @details
 * Opens configured ALSA PCM and mixer devices, writes only zero-valued samples,
 * drains the stream, and applies a conservative playback volume.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio ALSA Hardware Test
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

#include "xHal_Rpi5CarAudioAlsa.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Writes silence through the deployment-selected ALSA device.
 *
 * @param[in] argumentCount
 * One for defaults or four when PCM, mixer, and element names are supplied.
 *
 * @param[in] argumentValues
 * Executable name followed optionally by PCM device, mixer device, and element.
 *
 * @return
 * Zero after silence, volume, and cleanup operations succeed; otherwise one.
 *
 * @warning
 * Run only on an approved Raspberry Pi audio setup. This changes mixer volume
 * to fifty percent but writes no audible sample.
 */
int main(int argumentCount, char* argumentValues[])
{
    if ((argumentCount != 1) && (argumentCount != 4))
    {
        return 1;
    }
    const xwalk::hal::stringview pcmDevice = (argumentCount == 4) ? argumentValues[1] : "default";
    const xwalk::hal::stringview mixerDevice = (argumentCount == 4) ? argumentValues[2] : "default";
    const xwalk::hal::stringview mixerElement = (argumentCount == 4) ? argumentValues[3] : "PCM";

    xwalk::hal::XWalkAudioAlsa audio(pcmDevice, mixerDevice, mixerElement);
    const xwalk::hal::XWalkAudioStreamConfiguration configuration{
        44'100U,
        1U,
        xwalk::hal::XWalkAudioSampleFormat::Signed16LittleEndian,
        256U,
        XHAL_RPI5CAR_AUDIO_DEFAULT_LATENCY_US};
    xwalk::hal::audiopcmhandle stream = audio.openStream(configuration);
    const xwalk::hal::size frameCount = 256U;
    const xwalk::hal::bytevector silence(frameCount * 2U, 0U);
    audio.writeFrames(stream, silence, frameCount);
    audio.closeStream(stream);
    audio.setVolume(50U);
    return 0;
}
