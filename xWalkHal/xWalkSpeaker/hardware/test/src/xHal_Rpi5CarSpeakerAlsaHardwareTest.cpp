/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerAlsaHardwareTest.cpp
 * @brief       Provides an opt-in silent Raspberry Pi Speaker playback test.
 *
 * @details
 * Generates one short known PCM WAVE fixture, applies five-percent mixer
 * volume, plays its silent frames through shared ALSA, and removes the fixture.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker ALSA Adapter Hardware Test
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

#include "xHal_Rpi5CarSpeaker.h"
#include "xHal_Rpi5CarSpeakerAlsa.h"

#include "xHal_Rpi5CarFileFunctions.h"

#include <fstream>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains fixture generation private to this hardware test.
 */
namespace
{

/** @brief Stores one little-endian sixteen-bit fixture value. */
void writeUint16(XWalkHal::string& data, XWalkHal::size byteOffset,
    XWalkHal::uint16 value)
{
    data[byteOffset] = static_cast<char>(value & 0xFFU);
    data[byteOffset + 1U] = static_cast<char>((value >> 8U) & 0xFFU);
}

/** @brief Stores one little-endian thirty-two-bit fixture value. */
void writeUint32(XWalkHal::string& data, XWalkHal::size byteOffset,
    XWalkHal::uint32 value)
{
    for (XWalkHal::size byteIndex = 0U; byteIndex < 4U; ++byteIndex)
    {
        const XWalkHal::uint32 shiftBits = static_cast<XWalkHal::uint32>(byteIndex * 8U);
        data[byteOffset + byteIndex] = static_cast<char>((value >> shiftBits) & 0xFFU);
    }
}

/** @brief Writes a known 256-frame mono silent PCM WAVE fixture. */
void writeSilentWave(const XWalkHal::filesystempath& filePath)
{
    const XWalkHal::uint32 pcmByteCount = 512U;
    XWalkHal::string waveData(44U + pcmByteCount, '\0');
    waveData.replace(0U, 4U, "RIFF");
    writeUint32(waveData, 4U, 36U + pcmByteCount);
    waveData.replace(8U, 4U, "WAVE");
    waveData.replace(12U, 4U, "fmt ");
    writeUint32(waveData, 16U, 16U);
    writeUint16(waveData, 20U, 1U);
    writeUint16(waveData, 22U, 1U);
    writeUint32(waveData, 24U, 44'100U);
    writeUint32(waveData, 28U, 88'200U);
    writeUint16(waveData, 32U, 2U);
    writeUint16(waveData, 34U, 16U);
    waveData.replace(36U, 4U, "data");
    writeUint32(waveData, 40U, pcmByteCount);
    XWalkHal::outputfilestream file(filePath, XWalkHal::FILE_OPEN_WRITE_TRUNCATE);
    file << waveData;
    if (!file.good())
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker hardware test could not write its WAVE fixture");
    }
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Plays one short silent fixture through configured ALSA devices.
 *
 * @param[in] argumentCount
 * Either one for defaults or four with PCM, mixer, and element names.
 *
 * @param[in] argumentValues
 * Program name followed optionally by three non-empty ALSA names.
 *
 * @return
 * Zero after playback and fixture cleanup complete.
 *
 * @warning
 * Run only with the correct Raspberry Pi and Robot HAT audio setup connected.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer argumentValues[])
{
    if ((argumentCount != 1) && (argumentCount != 4))
    {
        XHAL_THROW_INVALID_ARGUMENT("Speaker hardware test accepts PCM, mixer, and element names");
    }
    const XWalkHal::stringview pcmDevice =
        (argumentCount == 4) ? XWalkHal::stringview(argumentValues[1]) : XWalkHal::stringview("default");
    const XWalkHal::stringview mixerDevice =
        (argumentCount == 4) ? XWalkHal::stringview(argumentValues[2]) : XWalkHal::stringview("default");
    const XWalkHal::stringview mixerElement =
        (argumentCount == 4) ? XWalkHal::stringview(argumentValues[3]) : XWalkHal::stringview("PCM");
    const XWalkHal::filesystempath fixturePath("/tmp/xwalk-speaker-alsa-hardware-test.wav");
    writeSilentWave(fixturePath);

    XWalkHal::XWalkAudioAlsa audio(pcmDevice, mixerDevice, mixerElement);
    XWalkHal::XWalkSpeakerAlsa adapter(audio, 5U);
    XWalkHal::XWalkSpeaker speaker(&adapter, adapter.callbacks());
    const XWalkHal::string taskId = speaker.play(fixturePath.string());
    XWalkHal::boolean playbackComplete = false;
    for (XWalkHal::uint32 attempt = 0U; attempt < 200U; ++attempt)
    {
        if (speaker.listTasks().empty())
        {
            playbackComplete = true;
            break;
        }
        XWalkHal::common::sleepMilliseconds(10U);
    }
    if (!playbackComplete)
    {
        static_cast<void>(speaker.stop(taskId));
        XHAL_THROW_RUNTIME_ERROR("Speaker hardware test playback did not complete in two seconds");
    }
    static_cast<void>(XWalkHal::removeFilesystemEntry(fixturePath));
    return 0;
}
