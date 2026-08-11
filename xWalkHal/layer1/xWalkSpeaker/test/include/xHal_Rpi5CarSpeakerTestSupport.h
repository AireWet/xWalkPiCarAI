/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerTestSupport.h
 * @brief       Declares reusable xWalkSpeaker host-test support.
 * @project     xWalk Firmware
 * @module      xWalkSpeaker Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_SPEAKER_TEST_SUPPORT_H
#define XHAL_RPI5CAR_SPEAKER_TEST_SUPPORT_H
#include "xHal_Rpi5CarSpeaker.h"
namespace xwalk::hal::test::speaker
{
/** @brief Records simulated decoding, stream, and output operations. */
struct TestBackend
{
    mutexhandle callbackMutex{};
    conditionvariable callbackCondition{};
    uint32 enableCount{};
    uint32 disableCount{};
    uint32 decodeCount{};
    uint32 openCount{};
    uint32 writeCount{};
    uint32 closeCount{};
    uint32 taskIdCount{};
    size decodedFrameCount{65'536U};
    XWalkSpeakerAudioHandler handler{XWalkSpeakerAudioHandler::SoundFile};
    boolean invalidAudio{};
    boolean failWrite{};
    boolean writeDelayEnabled{true};
};
void enableOutput(contextpointer context);
void disableOutput(contextpointer context);
XWalkSpeakerAudioData decodeAudio(contextpointer context, stringview filePath,
    XWalkSpeakerAudioHandler handler);
speakerstreamhandle openStream(contextpointer context, uint32 sampleRateHz,
    uint8 channelCount);
void writeStream(contextpointer context, speakerstreamhandle stream,
    const XWalkSpeakerAudioData& audioData, size firstFrame, size frameCount);
void closeStream(contextpointer context, speakerstreamhandle stream);
string createTaskId(contextpointer context);
XWalkSpeakerCallbacks speakerCallbacks();
void createTestFile(const filesystempath& path);
boolean waitForWriteCount(TestBackend& backend, uint32 expectedCount);
} /* namespace xwalk::hal::test::speaker */
#endif /* XHAL_RPI5CAR_SPEAKER_TEST_SUPPORT_H */
