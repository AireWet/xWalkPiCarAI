/******************************************************************************
 * @file        xHal_Rpi5CarAudioAlsa.h
 * @brief       Declares shared ALSA PCM and mixer ownership.
 *
 * @details
 * Owns one configured mixer and a bounded set of PCM playback handles while
 * exposing deterministic stream, write, recovery, volume, and cleanup behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio ALSA Backend
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

#ifndef XHAL_RPI5CAR_AUDIO_ALSA_H
#define XHAL_RPI5CAR_AUDIO_ALSA_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAudioTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkAudioAlsa
     * @brief Owns shared Linux ALSA playback and mixer resources.
     *
     * @details
     * Opens the configured mixer during construction and owns up to eight PCM
     * handles opened on demand. All public operations are serialized. Injected
     * operation callbacks permit deterministic software tests without a sound card.
     */
    class XWalkAudioAlsa final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Mutex serializing mixer state and every owned PCM handle. */
            mutable mutexhandle mutex;

            /**
             * @brief Nullable non-owning context supplied to every ALSA operation.
             *
             * @note
             * A non-null object must outlive this backend. Null is permitted only
             * when every callback supports stateless operation.
             */
            contextpointer operationContextPointer{nullptr};

            /** @brief Complete ALSA operation table copied during construction. */
            XWalkAudioAlsaOperations operations{};

            /** @brief Configured owned ALSA PCM device name. */
            string pcmDeviceNameValue{};

            /** @brief Configured owned ALSA mixer device name. */
            string mixerDeviceNameValue{};

            /** @brief Configured owned ALSA simple mixer element name. */
            string mixerElementNameValue{};

            /** @brief Non-null owned mixer handle closed during destruction. */
            audiomixerhandle mixerHandle{nullptr};

            /** @brief Bounded owned PCM handles; null entries are available slots. */
            fixedarray<audiopcmhandle, XHAL_RPI5CAR_AUDIO_MAXIMUM_STREAM_COUNT> pcmHandles{};

            /** @brief Negotiated configuration retained for each corresponding PCM slot. */
            fixedarray<XWalkAudioStreamConfiguration, XHAL_RPI5CAR_AUDIO_MAXIMUM_STREAM_COUNT> streamConfigurations{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Validates every callback and configured ALSA name.
             *
             * @param[in] backendOperations
             * Operation table inspected before acquiring the mixer.
             *
             * @param[in] pcmDeviceName
             * PCM device name that must not be empty.
             *
             * @param[in] mixerDeviceName
             * Mixer device name that must not be empty.
             *
             * @param[in] mixerElementName
             * Mixer element name that must not be empty.
             *
             * @throws std::invalid_argument
             * If a callback is null or a configured name is empty.
             */
            static void validateConstruction(const XWalkAudioAlsaOperations& backendOperations,
                                             stringview pcmDeviceName,
                                             stringview mixerDeviceName,
                                             stringview mixerElementName);

            /**
             * @brief Validates and returns one stream configuration.
             *
             * @param[in] configuration
             * Stream configuration to inspect.
             *
             * @return
             * Validated configuration copy.
             *
             * @throws std::invalid_argument
             * If a required count or duration is zero.
             *
             * @throws std::out_of_range
             * If a channel or period count exceeds its supported maximum.
             */
            static XWalkAudioStreamConfiguration
            validateConfiguration(const XWalkAudioStreamConfiguration& configuration);

            /**
             * @brief Returns the byte width of one sample representation.
             *
             * @param[in] format
             * Supported PCM sample format.
             *
             * @return
             * Two or four bytes per sample.
             *
             * @throws std::out_of_range
             * If the format enumerator is unsupported.
             */
            static size bytesPerSample(XWalkAudioSampleFormat format);

            /**
             * @brief Finds an available owned-stream slot.
             *
             * @return
             * First available slot index.
             *
             * @throws std::out_of_range
             * If every bounded stream slot is occupied.
             *
             * @pre
             * The caller holds `mutex`.
             */
            size availableStreamIndex() const;

            /**
             * @brief Finds the slot owning one PCM handle.
             *
             * @param[in] streamHandle
             * Non-null handle returned by `openStream()`.
             *
             * @return
             * Occupied slot index that owns the handle.
             *
             * @throws std::invalid_argument
             * If the handle is null or is not owned by this backend.
             *
             * @pre
             * The caller holds `mutex`.
             */
            size streamIndex(audiopcmhandle streamHandle) const;

            /**
             * @brief Returns the real Linux ALSA operation table.
             *
             * @return
             * Complete stateless callback table backed by libasound.
             */
            static XWalkAudioAlsaOperations systemOperations() noexcept;

            /** @brief Opens one real ALSA PCM playback handle. */
            static audiopcmhandle systemOpenPcm(contextpointer context, stringview deviceName);
            /** @brief Configures one real ALSA PCM playback handle. */
            static boolean systemConfigurePcm(contextpointer context,
                                              audiopcmhandle pcmHandle,
                                              const XWalkAudioStreamConfiguration& configuration);
            /** @brief Writes frames to one real ALSA PCM playback handle. */
            static int32 systemWritePcm(contextpointer context,
                                        audiopcmhandle pcmHandle,
                                        const bytevector& pcmData,
                                        size byteOffset,
                                        size frameCount);
            /** @brief Recovers one real ALSA PCM playback handle. */
            static boolean systemRecoverPcm(contextpointer context, audiopcmhandle pcmHandle, int32 errorValue);
            /** @brief Drains and closes one real ALSA PCM playback handle. */
            static void systemClosePcm(contextpointer context, audiopcmhandle pcmHandle);
            /** @brief Opens and initializes one real ALSA mixer handle. */
            static audiomixerhandle systemOpenMixer(contextpointer context, stringview deviceName);
            /** @brief Applies volume to one real ALSA mixer element. */
            static boolean systemSetMixerVolume(contextpointer context,
                                                audiomixerhandle mixerHandle,
                                                stringview elementName,
                                                uint8 volumePercent);
            /** @brief Closes one real ALSA mixer handle. */
            static void systemCloseMixer(contextpointer context, audiomixerhandle mixerHandle);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a backend using real libasound operations.
             *
             * @param[in] pcmDeviceName
             * Non-empty ALSA PCM name selected by deployment configuration.
             *
             * @param[in] mixerDeviceName
             * Non-empty ALSA mixer name selected by deployment configuration.
             *
             * @param[in] mixerElementName
             * Non-empty ALSA playback element name, commonly `PCM`.
             *
             * @throws std::invalid_argument
             * If a configured name is empty.
             *
             * @throws std::runtime_error
             * If the configured mixer cannot be opened and initialized.
             */
            explicit XWalkAudioAlsa(stringview pcmDeviceName = "default",
                                    stringview mixerDeviceName = "default",
                                    stringview mixerElementName = "PCM");

            /**
             * @brief Constructs a backend using injected ALSA operations.
             *
             * @param[in,out] operationContext
             * Nullable non-owning callback context that must outlive this backend.
             *
             * @param[in] backendOperations
             * Complete callback table copied before acquiring the mixer.
             *
             * @param[in] pcmDeviceName
             * Non-empty ALSA PCM name forwarded to stream-open operations.
             *
             * @param[in] mixerDeviceName
             * Non-empty ALSA mixer name forwarded during construction.
             *
             * @param[in] mixerElementName
             * Non-empty mixer element name forwarded by volume operations.
             *
             * @throws std::invalid_argument
             * If a callback is null or a configured name is empty.
             *
             * @throws std::runtime_error
             * If the configured mixer cannot be opened and initialized.
             */
            XWalkAudioAlsa(contextpointer operationContext,
                           const XWalkAudioAlsaOperations& backendOperations,
                           stringview pcmDeviceName,
                           stringview mixerDeviceName,
                           stringview mixerElementName);

            /**
             * @brief Closes every retained PCM stream and the owned mixer.
             *
             * @warning
             * Injected close callbacks must not throw.
             */
            ~XWalkAudioAlsa();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables copying of owned ALSA handles. */
            XWalkAudioAlsa(const XWalkAudioAlsa&) = delete;
            /** @brief Disables copy assignment of owned ALSA handles. */
            XWalkAudioAlsa& operator=(const XWalkAudioAlsa&) = delete;
            /** @brief Disables moving because callback contexts and handles retain identity. */
            XWalkAudioAlsa(XWalkAudioAlsa&&) = delete;
            /** @brief Disables move assignment because callback contexts and handles retain identity. */
            XWalkAudioAlsa& operator=(XWalkAudioAlsa&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Opens, negotiates, and retains one PCM playback stream.
             *
             * @param[in] configuration
             * Valid sample rate, channel count, format, period size, and latency.
             *
             * @return
             * Non-null opaque PCM handle owned until `closeStream()` or destruction.
             *
             * @throws std::invalid_argument
             * If the configuration contains a zero required value.
             *
             * @throws std::out_of_range
             * If a limit is exceeded or all stream slots are occupied.
             *
             * @throws std::runtime_error
             * If ALSA cannot open or configure the selected PCM device.
             */
            audiopcmhandle openStream(const XWalkAudioStreamConfiguration& configuration);

            /**
             * @brief Writes one complete interleaved PCM payload with bounded recovery.
             *
             * @param[in,out] streamHandle
             * Non-null handle currently owned by this backend.
             *
             * @param[in] pcmData
             * Payload containing exactly `frameCount` complete frames.
             *
             * @param[in] frameCount
             * Positive number of interleaved frames represented by `pcmData`, not
             * exceeding the configured period size.
             *
             * @throws std::invalid_argument
             * If the handle, frame count, or payload is invalid.
             *
             * @throws std::runtime_error
             * If writes stop progressing or recovery exceeds its bounded attempts.
             */
            void writeFrames(audiopcmhandle streamHandle, const bytevector& pcmData, size frameCount);

            /**
             * @brief Drains, closes, and releases one owned PCM stream.
             *
             * @param[in,out] streamHandle
             * Non-null handle currently owned by this backend.
             *
             * @throws std::invalid_argument
             * If the handle is null or is not owned by this backend.
             *
             * @warning
             * The injected close callback must not throw.
             */
            void closeStream(audiopcmhandle streamHandle);

            /**
             * @brief Applies one bounded percentage to the configured mixer element.
             *
             * @param[in] volumePercent
             * Volume in the inclusive range zero through one hundred percent.
             *
             * @throws std::out_of_range
             * If the volume exceeds one hundred percent.
             *
             * @throws std::runtime_error
             * If the mixer element cannot accept the volume.
             */
            void setVolume(uint8 volumePercent);

            /**
             * @brief Reports how many PCM handles are currently owned.
             *
             * @return
             * Stream count from zero through eight.
             */
            size openStreamCount() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_AUDIO_ALSA_H */
