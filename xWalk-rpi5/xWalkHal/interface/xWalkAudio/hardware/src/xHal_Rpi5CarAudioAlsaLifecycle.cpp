/******************************************************************************
 * @file        xHal_Rpi5CarAudioAlsaLifecycle.cpp
 * @brief       Defines shared ALSA backend validation and lifecycle behavior.
 *
 * @details
 * Validates injected operations and device names, acquires the configured
 * mixer, and deterministically releases every retained ALSA resource.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAudioAlsa.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

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
void XWalkAudioAlsa::validateConstruction(
    const XWalkAudioAlsaOperations &backendOperations, stringview pcmDeviceName,
    stringview mixerDeviceName, stringview mixerElementName) {
  const boolean pcmOperationMissing =
      (backendOperations.openPcm == nullptr) ||
      (backendOperations.configurePcm == nullptr) ||
      (backendOperations.writePcm == nullptr) ||
      (backendOperations.recoverPcm == nullptr) ||
      (backendOperations.closePcm == nullptr);
  const boolean mixerOperationMissing =
      (backendOperations.openMixer == nullptr) ||
      (backendOperations.setMixerVolume == nullptr) ||
      (backendOperations.closeMixer == nullptr);
  if (pcmOperationMissing || mixerOperationMissing) {
    XWALK_HAL_ERROR(XWALK_INVAL, "ALSA audio operations must not be null");
  }
  const hal::boolean audioConfigurationInvalid = static_cast<hal::boolean>(
      pcmDeviceName.empty() || mixerDeviceName.empty() ||
      mixerElementName.empty());
  if (audioConfigurationInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "ALSA audio device and mixer names must not be empty");
  }
}

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
XWalkAudioStreamConfiguration XWalkAudioAlsa::validateConfiguration(
    const XWalkAudioStreamConfiguration &configuration) {
  if ((configuration.sampleRateHz == 0U) ||
      (configuration.channelCount == 0U) ||
      (configuration.periodFrames == 0U) || (configuration.latencyUs == 0U)) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "ALSA audio stream values must be greater than zero");
  }
  if (configuration.channelCount > XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "ALSA audio channel count exceeds its supported maximum");
  }
  if (configuration.periodFrames > XHAL_RPI5CAR_AUDIO_MAXIMUM_PERIOD_FRAMES) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "ALSA audio period exceeds its supported maximum");
  }
  static_cast<void>(bytesPerSample(configuration.format));
  return configuration;
}

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
size XWalkAudioAlsa::bytesPerSample(XWalkAudioSampleFormat format) {
  switch (format) {
  case XWalkAudioSampleFormat::Signed16LittleEndian:
    return 2U;
  case XWalkAudioSampleFormat::Float32LittleEndian:
    return 4U;
  default:
    XWALK_HAL_ERROR(XWALK_RANGE, "ALSA audio sample format is not supported");
  }
}

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
size XWalkAudioAlsa::availableStreamIndex() const {
  for (size index = 0U; index < pcmHandles.size(); ++index) {
    if (pcmHandles[index] == nullptr) {
      return index;
    }
  }
  XWALK_HAL_ERROR(XWALK_RANGE, "ALSA audio stream limit has been reached");
}

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
size XWalkAudioAlsa::streamIndex(audiopcmhandle streamHandle) const {
  if (streamHandle == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "ALSA audio stream handle must not be null");
  }
  for (size index = 0U; index < pcmHandles.size(); ++index) {
    if (pcmHandles[index] == streamHandle) {
      return index;
    }
  }
  XWALK_HAL_ERROR(XWALK_INVAL,
                  "ALSA audio stream is not owned by this backend");
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

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
XWalkAudioAlsa::XWalkAudioAlsa(stringview pcmDeviceName,
                               stringview mixerDeviceName,
                               stringview mixerElementName)
    : XWalkAudioAlsa(nullptr, systemOperations(), pcmDeviceName,
                     mixerDeviceName, mixerElementName) {}

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
XWalkAudioAlsa::XWalkAudioAlsa(
    contextpointer operationContext,
    const XWalkAudioAlsaOperations &backendOperations, stringview pcmDeviceName,
    stringview mixerDeviceName, stringview mixerElementName)
    : operationContextPointer(operationContext), operations(backendOperations),
      pcmDeviceNameValue(pcmDeviceName), mixerDeviceNameValue(mixerDeviceName),
      mixerElementNameValue(mixerElementName) {
  validateConstruction(backendOperations, pcmDeviceName, mixerDeviceName,
                       mixerElementName);
  mixerHandle =
      operations.openMixer(operationContextPointer, mixerDeviceNameValue);
  if (mixerHandle == nullptr) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "ALSA audio mixer could not be opened");
  }
  XWALK_HAL_TRACE_UID2(
      RPI .085, "ALSA audio backend constructed for PCM %s and mixer %s",
      pcmDeviceNameValue.c_str(), mixerDeviceNameValue.c_str());
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Closes every retained PCM stream and the owned mixer.
 *
 * @warning
 * Injected close callbacks must not throw.
 */
XWalkAudioAlsa::~XWalkAudioAlsa() {
  for (audiopcmhandle &pcmHandle : pcmHandles) {
    if (pcmHandle != nullptr) {
      operations.closePcm(operationContextPointer, pcmHandle);
      pcmHandle = nullptr;
    }
  }
  if (mixerHandle != nullptr) {
    operations.closeMixer(operationContextPointer, mixerHandle);
    mixerHandle = nullptr;
  }
}

} /* namespace xwalk::hal */
