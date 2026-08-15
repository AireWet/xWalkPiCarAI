/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerHostStub.cpp
 * @brief       Implements the silent in-memory Speaker host stub.
 * @project     xWalk Firmware
 * @module      xWalkSpeaker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarSpeakerHostStub.h"
namespace xwalk::hal::sim
{
    void XWalkSpeakerHostStub::enableOutput(contextpointer context)
    {
        XWalkSpeakerHostStub& backend = *static_cast<XWalkSpeakerHostStub*>(context);
        const mutexlock lock(backend.mutexValue);
        ++backend.enableCountValue;
    }
    void XWalkSpeakerHostStub::disableOutput(contextpointer context)
    {
        XWalkSpeakerHostStub& backend = *static_cast<XWalkSpeakerHostStub*>(context);
        const mutexlock lock(backend.mutexValue);
        ++backend.disableCountValue;
    }
    XWalkSpeakerAudioData
    XWalkSpeakerHostStub::decodeAudio(contextpointer context, stringview filePath, XWalkSpeakerAudioHandler handler)
    {
        static_cast<void>(filePath);
        static_cast<void>(handler);
        XWalkSpeakerHostStub& backend = *static_cast<XWalkSpeakerHostStub*>(context);
        {
            const mutexlock lock(backend.mutexValue);
            ++backend.decodeCountValue;
        }
        return {float64vector(2'048U, 0.0), 44'100U, 1U};
    }
    speakerstreamhandle
    XWalkSpeakerHostStub::openStream(contextpointer context, uint32 sampleRateHz, uint8 channelCount)
    {
        static_cast<void>(sampleRateHz);
        static_cast<void>(channelCount);
        XWalkSpeakerHostStub& backend = *static_cast<XWalkSpeakerHostStub*>(context);
        const mutexlock lock(backend.mutexValue);
        ++backend.openCountValue;
        return context;
    }
    void XWalkSpeakerHostStub::writeStream(contextpointer context,
                                           speakerstreamhandle stream,
                                           const XWalkSpeakerAudioData& audioData,
                                           size firstFrame,
                                           size frameCount)
    {
        static_cast<void>(stream);
        static_cast<void>(audioData);
        static_cast<void>(firstFrame);
        static_cast<void>(frameCount);
        XWalkSpeakerHostStub& backend = *static_cast<XWalkSpeakerHostStub*>(context);
        const mutexlock lock(backend.mutexValue);
        ++backend.writeCountValue;
    }
    void XWalkSpeakerHostStub::closeStream(contextpointer context, speakerstreamhandle stream)
    {
        static_cast<void>(stream);
        XWalkSpeakerHostStub& backend = *static_cast<XWalkSpeakerHostStub*>(context);
        const mutexlock lock(backend.mutexValue);
        ++backend.closeCountValue;
    }
    string XWalkSpeakerHostStub::createTaskId(contextpointer context)
    {
        XWalkSpeakerHostStub& backend = *static_cast<XWalkSpeakerHostStub*>(context);
        const mutexlock lock(backend.mutexValue);
        ++backend.taskCountValue;
        return string("speaker-simulation-") + std::to_string(backend.taskCountValue);
    }
    XWalkSpeakerCallbacks XWalkSpeakerHostStub::callbacks()
    {
        return {&enableOutput, &disableOutput, &decodeAudio, &openStream, &writeStream, &closeStream, &createTaskId};
    }
    uint32 XWalkSpeakerHostStub::enableCount() const
    {
        const mutexlock lock(mutexValue);
        return enableCountValue;
    }
    uint32 XWalkSpeakerHostStub::disableCount() const
    {
        const mutexlock lock(mutexValue);
        return disableCountValue;
    }
    uint32 XWalkSpeakerHostStub::decodeCount() const
    {
        const mutexlock lock(mutexValue);
        return decodeCountValue;
    }
    uint32 XWalkSpeakerHostStub::openCount() const
    {
        const mutexlock lock(mutexValue);
        return openCountValue;
    }
    uint32 XWalkSpeakerHostStub::writeCount() const
    {
        const mutexlock lock(mutexValue);
        return writeCountValue;
    }
    uint32 XWalkSpeakerHostStub::closeCount() const
    {
        const mutexlock lock(mutexValue);
        return closeCountValue;
    }
} /* namespace xwalk::hal::sim */
