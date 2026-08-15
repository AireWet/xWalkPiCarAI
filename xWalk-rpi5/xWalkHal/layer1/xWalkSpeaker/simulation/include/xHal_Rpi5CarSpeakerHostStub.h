/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerHostStub.h
 * @brief       Declares the silent in-memory Speaker host stub.
 * @project     xWalk Firmware
 * @module      xWalkSpeaker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_SPEAKER_HOST_STUB_H
#define XHAL_RPI5CAR_SPEAKER_HOST_STUB_H
#include "xHal_Rpi5CarSpeaker.h"
namespace xwalk::hal::sim
{
    /** @brief Records Speaker operations without decoding or physical output. */
    class XWalkSpeakerHostStub final
    {
        private:
            mutable mutexhandle mutexValue{};
            uint32 enableCountValue{};
            uint32 disableCountValue{};
            uint32 decodeCountValue{};
            uint32 openCountValue{};
            uint32 writeCountValue{};
            uint32 closeCountValue{};
            uint32 taskCountValue{};

        public:
            static void enableOutput(contextpointer context);
            static void disableOutput(contextpointer context);
            static XWalkSpeakerAudioData
            decodeAudio(contextpointer context, stringview filePath, XWalkSpeakerAudioHandler handler);
            static speakerstreamhandle openStream(contextpointer context, uint32 sampleRateHz, uint8 channelCount);
            static void writeStream(contextpointer context,
                                    speakerstreamhandle stream,
                                    const XWalkSpeakerAudioData& audioData,
                                    size firstFrame,
                                    size frameCount);
            static void closeStream(contextpointer context, speakerstreamhandle stream);
            static string createTaskId(contextpointer context);
            static XWalkSpeakerCallbacks callbacks();
            uint32 enableCount() const;
            uint32 disableCount() const;
            uint32 decodeCount() const;
            uint32 openCount() const;
            uint32 writeCount() const;
            uint32 closeCount() const;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_SPEAKER_HOST_STUB_H */
