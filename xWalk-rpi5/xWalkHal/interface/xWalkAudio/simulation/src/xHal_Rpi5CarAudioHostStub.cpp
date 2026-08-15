/******************************************************************************
 * @file        xHal_Rpi5CarAudioHostStub.cpp
 * @brief       Implements the device-free xWalkAudio host stub.
 *
 * @details
 * Supplies an in-memory implementation of every injected Audio operation and
 * records the representative frame count and mixer volume for host assertions.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
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

#include "xHal_Rpi5CarAudioHostStub.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim
{

    XWalkAudioHostStub::XWalkAudioHostStub() = default;
    XWalkAudioHostStub::~XWalkAudioHostStub() = default;

    audiopcmhandle XWalkAudioHostStub::openPcm(contextpointer context, stringview deviceName)
    {
        auto& stub = *static_cast<XWalkAudioHostStub*>(context);
        const boolean deviceValid = deviceName.empty() == false;
        const boolean tokenAvailable = stub.nextPcmToken < stub.pcmTokens.size();
        if ((deviceValid == false) || (tokenAvailable == false))
        {
            return nullptr;
        }
        audiopcmhandle handle = &stub.pcmTokens[stub.nextPcmToken];
        ++stub.nextPcmToken;
        const string ownedDeviceName(deviceName);
        XWALK_HAL_TRACE_UID1(RPI .090, "Host Audio opened mirrored PCM %s", ownedDeviceName.c_str());
        return handle;
    }

    boolean XWalkAudioHostStub::configurePcm(contextpointer context,
                                             audiopcmhandle pcmHandle,
                                             const XWalkAudioStreamConfiguration& configuration)
    {
        static_cast<void>(context);
        return (pcmHandle != nullptr) && (configuration.sampleRateHz > 0U);
    }

    int32 XWalkAudioHostStub::writePcm(
        contextpointer context, audiopcmhandle pcmHandle, const bytevector& pcmData, size byteOffset, size frameCount)
    {
        static_cast<void>(pcmData);
        static_cast<void>(byteOffset);
        auto& stub = *static_cast<XWalkAudioHostStub*>(context);
        if (pcmHandle == nullptr)
        {
            return -1;
        }
        stub.writtenFrameCountValue += frameCount;
        return static_cast<int32>(frameCount);
    }

    boolean XWalkAudioHostStub::recoverPcm(contextpointer context, audiopcmhandle pcmHandle, int32 errorValue)
    {
        static_cast<void>(context);
        return (pcmHandle != nullptr) && (errorValue < 0);
    }

    void XWalkAudioHostStub::closePcm(contextpointer context, audiopcmhandle pcmHandle)
    {
        static_cast<void>(context);
        static_cast<void>(pcmHandle);
    }

    audiomixerhandle XWalkAudioHostStub::openMixer(contextpointer context, stringview deviceName)
    {
        auto& stub = *static_cast<XWalkAudioHostStub*>(context);
        const boolean deviceValid = deviceName.empty() == false;
        if (deviceValid == false)
        {
            return nullptr;
        }
        const string ownedDeviceName(deviceName);
        XWALK_HAL_TRACE_UID1(RPI .091, "Host Audio opened mirrored mixer %s", ownedDeviceName.c_str());
        return &stub.mixerToken;
    }

    boolean XWalkAudioHostStub::setMixerVolume(contextpointer context,
                                               audiomixerhandle mixerHandle,
                                               stringview elementName,
                                               uint8 volumePercent)
    {
        auto& stub = *static_cast<XWalkAudioHostStub*>(context);
        const boolean requestValid =
            (mixerHandle != nullptr) && (elementName.empty() == false) && (volumePercent <= 100U);
        if (requestValid)
        {
            stub.volumePercentValue = volumePercent;
            XWALK_HAL_TRACE_UID1(RPI .092, "Host Audio mirrored volume %u percent", static_cast<uint32>(volumePercent));
        }
        return requestValid;
    }

    void XWalkAudioHostStub::closeMixer(contextpointer context, audiomixerhandle mixerHandle)
    {
        static_cast<void>(context);
        static_cast<void>(mixerHandle);
    }

    XWalkAudioAlsaOperations XWalkAudioHostStub::operations() noexcept
    {
        return {&openPcm, &configurePcm, &writePcm, &recoverPcm, &closePcm, &openMixer, &setMixerVolume, &closeMixer};
    }

    size XWalkAudioHostStub::writtenFrameCount() const noexcept
    {
        return writtenFrameCountValue;
    }

    uint8 XWalkAudioHostStub::volumePercent() const noexcept
    {
        return volumePercentValue;
    }

} /* namespace xwalk::hal::sim */
