/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechHardwareTest.cpp
 * @brief       Provides opt-in synthesized-fixture playback through ALSA.
 *
 * @details
 * Maps one fixed short phrase to deployment-supplied 16 kHz mono signed-16 PCM
 * and plays it at a conservative volume through explicitly selected devices.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Text-to-Speech ALSA Hardware Test
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

#include "xHal_Rpi5CarTextToSpeechAlsa.h"

#include "xHal_Rpi5CarFileFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains the explicitly selected prerecorded synthesis fixture.
 */
namespace
{

/******************************************************************************
 * Constants
 ******************************************************************************/

/** @brief Fixed bounded phrase represented by the supplied raw PCM fixture. */
constexpr XWalkHal::stringview TEST_PHRASE{"xWalk text to speech test"};

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Retains the deployment-owned raw PCM fixture path. */
struct FixtureProvider
{
    /** @brief Existing raw signed sixteen-bit little-endian PCM file path. */
    XWalkHal::filesystempath filePath{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Loads the deployment-supplied PCM fixture for the fixed test phrase.
 *
 * @param[in,out] context Non-null fixture-provider context.
 * @param[in] text Exact fixed phrase represented by the fixture.
 * @return Owned 16 kHz mono signed sixteen-bit PCM.
 * @throws std::invalid_argument If `text` does not match the fixed phrase.
 * @throws std::runtime_error If the fixture cannot be read.
 */
XWalkHal::XWalkTextToSpeechPcmData synthesizeFixture(
    XWalkHal::contextpointer context, XWalkHal::stringview text)
{
    if (text != TEST_PHRASE)
    {
        XHAL_THROW_INVALID_ARGUMENT("Text-to-speech hardware phrase is fixed");
    }
    const FixtureProvider& provider = *static_cast<FixtureProvider*>(context);
    const XWalkHal::string contents = XWalkHal::readFileContents(provider.filePath);
    XWalkHal::bytevector pcmData{};
    pcmData.reserve(contents.size());
    for (const char character : contents)
    {
        pcmData.push_back(static_cast<XWalkHal::uint8>(static_cast<unsigned char>(character)));
    }
    return {pcmData, 16'000U, 1U};
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Plays one explicitly supplied prerecorded speech fixture.
 *
 * @param[in] argumentCount Exactly five arguments are required.
 * @param[in] argumentValues Program, PCM device, mixer device, element, and raw PCM fixture.
 * @return Zero after synthesis adaptation and bounded playback complete.
 * @warning Run only after confirming the intended speaker, mixer, fixture, and privacy context.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer argumentValues[])
{
    if (argumentCount != 5)
    {
        XHAL_THROW_INVALID_ARGUMENT("Text-to-speech hardware test requires explicit devices and fixture");
    }
    XWalkHal::XWalkAudioAlsa audio(argumentValues[1], argumentValues[2], argumentValues[3]);
    FixtureProvider provider{argumentValues[4]};
    const XWalkHal::XWalkTextToSpeechAlsaOperations operations{&synthesizeFixture};
    XWalkHal::XWalkTextToSpeechAlsa adapter(audio, &provider, operations, 15U);
    adapter.callback()(&adapter, TEST_PHRASE);
    return 0;
}
