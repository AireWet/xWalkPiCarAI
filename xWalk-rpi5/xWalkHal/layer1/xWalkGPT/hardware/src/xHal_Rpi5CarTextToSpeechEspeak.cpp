/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechEspeak.cpp
 * @brief       Implements bounded Espeak WAV synthesis.
 *
 * @details
 * Executes Espeak without a shell, supplies text through standard input,
 * captures bounded WAV bytes, and extracts signed sixteen-bit PCM metadata.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Espeak Provider
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xHal_Rpi5CarTextToSpeechEspeak.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include "xHal_Rpi5CarTrace.h"
#include <algorithm>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Stores one Espeak executable and voice.
 * @param[in] executable Non-empty executable name or path.
 * @param[in] voice Non-empty Espeak voice identifier.
 * @throws std::invalid_argument If either value is empty.
 */
XWalkTextToSpeechEspeak::XWalkTextToSpeechEspeak(stringview executable,
                                                 stringview voice)
    : executableName(executable), voiceName(voice) {
  const hal::boolean executableNameVoiceNameInvalid =
      static_cast<hal::boolean>(executableName.empty() || voiceName.empty());
  if (executableNameVoiceNameInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Espeak executable and voice are required");
  }
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Returns the callback consumed by the ALSA speech-playback adapter.
 * @return Synthesis operation requiring this provider as context.
 */
XWalkTextToSpeechAlsaOperations
XWalkTextToSpeechEspeak::operations() const noexcept {
  return XWalkTextToSpeechAlsaOperations{&synthesize};
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Converts a callback context into its required provider.
 * @param[in,out] context Non-null pointer to a live provider.
 * @return Referenced provider.
 * @throws std::invalid_argument If `context` is null.
 */
XWalkTextToSpeechEspeak &
XWalkTextToSpeechEspeak::provider(contextpointer context) {
  if (context == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Espeak provider context must not be null");
  }
  return *static_cast<XWalkTextToSpeechEspeak *>(context);
}

/**
 * @brief Synthesizes one text value through a bounded child process.
 * @param[in,out] context Non-null provider context.
 * @param[in] text Text retained only for this synchronous call.
 * @return Owned signed sixteen-bit PCM and playback metadata.
 */
XWalkTextToSpeechPcmData
XWalkTextToSpeechEspeak::synthesize(contextpointer context, stringview text) {
  const hal::boolean textEmpty = static_cast<hal::boolean>(text.empty());
  if (textEmpty) {
    return {};
  }
  const hal::boolean textTooLarge =
      static_cast<hal::boolean>(text.size() > 4'096U);
  if (textTooLarge) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "Espeak text exceeds the bounded input length");
  }
  return parseWave(provider(context).execute(text));
}

/**
 * @brief Runs Espeak without a shell and captures WAV standard output.
 * @param[in] text Non-empty bounded text supplied through standard input.
 * @return Owned WAV bytes.
 * @throws std::runtime_error If process or descriptor operations fail.
 */
bytevector XWalkTextToSpeechEspeak::execute(stringview text) const {
  fixedarray<int32, 2U> inputPipe{};
  fixedarray<int32, 2U> outputPipe{};
  const hal::boolean inputPipeDifferent =
      static_cast<hal::boolean>(::pipe2(inputPipe.data(), O_CLOEXEC) != 0);
  if (inputPipeDifferent) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Espeak pipe creation failed");
  }
  const hal::boolean outputPipeDifferent =
      static_cast<hal::boolean>(::pipe2(outputPipe.data(), O_CLOEXEC) != 0);
  if (outputPipeDifferent) {
    static_cast<void>(::close(inputPipe[0U]));
    static_cast<void>(::close(inputPipe[1U]));
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Espeak output pipe creation failed");
  }
  size writtenByteCount{};
  const hal::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const hal::boolean textBytesRemaining =
        static_cast<hal::boolean>(writtenByteCount < text.size());
    if (textBytesRemaining == false) {
      break;
    }
    const auto result = ::write(inputPipe[1U], text.data() + writtenByteCount,
                                text.size() - writtenByteCount);
    if (result > 0) {
      writtenByteCount += static_cast<size>(result);
    } else if ((result < 0) && (errno == EINTR)) {
      /* Retry an interrupted descriptor write. */
    } else {
      static_cast<void>(::close(inputPipe[0U]));
      static_cast<void>(::close(inputPipe[1U]));
      static_cast<void>(::close(outputPipe[0U]));
      static_cast<void>(::close(outputPipe[1U]));
      XWALK_HAL_ERROR(XWALK_RUNTIME, "Espeak input staging failed");
    }
  }
  static_cast<void>(::close(inputPipe[1U]));
  const auto childProcess = ::fork();
  if (childProcess < 0) {
    static_cast<void>(::close(inputPipe[0U]));
    static_cast<void>(::close(outputPipe[0U]));
    static_cast<void>(::close(outputPipe[1U]));
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Espeak process creation failed");
  }
  if (childProcess == 0) {
    static_cast<void>(::close(outputPipe[0U]));
    const hal::boolean inputPipeOutputPipeInvalid =
        static_cast<hal::boolean>((::dup2(inputPipe[0U], STDIN_FILENO) < 0) ||
                                  (::dup2(outputPipe[1U], STDOUT_FILENO) < 0));
    if (inputPipeOutputPipeInvalid) {
      ::_exit(127);
    }
    static_cast<void>(::close(inputPipe[0U]));
    static_cast<void>(::close(outputPipe[1U]));
    const int32 nullDescriptor = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
    if (nullDescriptor >= 0) {
      static_cast<void>(::dup2(nullDescriptor, STDERR_FILENO));
      static_cast<void>(::close(nullDescriptor));
    }
    ::execlp(executableName.c_str(), executableName.c_str(), "--stdout", "-v",
             voiceName.c_str(), static_cast<charpointer>(nullptr));
    ::_exit(127);
  }

  static_cast<void>(::close(inputPipe[0U]));
  static_cast<void>(::close(outputPipe[1U]));

  bytevector waveData;
  fixedarray<uint8, 4'096U> buffer{};
  while (true) {
    const auto result = ::read(outputPipe[0U], buffer.data(), buffer.size());
    if (result > 0) {
      waveData.insert(waveData.end(), buffer.data(),
                      buffer.data() + static_cast<size>(result));
      const hal::boolean waveDataTooLarge = static_cast<hal::boolean>(
          waveData.size() >
          (XHAL_RPI5CAR_TEXT_TO_SPEECH_MAXIMUM_PCM_BYTES + 65'536U));
      if (waveDataTooLarge) {
        static_cast<void>(::close(outputPipe[0U]));
        static_cast<void>(::kill(childProcess, SIGTERM));
        static_cast<void>(::waitpid(childProcess, nullptr, 0));
        XWALK_HAL_ERROR(XWALK_RANGE,
                        "Espeak WAV output exceeds its bounded size");
      }
    } else if (result == 0) {
      break;
    } else if (errno != EINTR) {
      static_cast<void>(::close(outputPipe[0U]));
      static_cast<void>(::kill(childProcess, SIGTERM));
      static_cast<void>(::waitpid(childProcess, nullptr, 0));
      XWALK_HAL_ERROR(XWALK_RUNTIME, "Espeak output read failed");
    }
  }
  static_cast<void>(::close(outputPipe[0U]));
  int processStatus{};
  auto waitResult = ::waitpid(childProcess, &processStatus, 0);
  while ((waitResult < 0) && (errno == EINTR)) {
    waitResult = ::waitpid(childProcess, &processStatus, 0);
  }
  const hal::boolean waitResultProcessStatusInvalid =
      static_cast<hal::boolean>((waitResult < 0) || !WIFEXITED(processStatus) ||
                                (WEXITSTATUS(processStatus) != 0));
  if (waitResultProcessStatusInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Espeak executable failed or is unavailable");
  }
  return waveData;
}

/**
 * @brief Parses one PCM WAV result returned by Espeak.
 * @param[in] waveData Complete bounded WAV bytes.
 * @return Owned PCM with sample rate and channel count.
 * @throws std::runtime_error If required PCM WAV chunks are malformed or
 * absent.
 */
XWalkTextToSpeechPcmData
XWalkTextToSpeechEspeak::parseWave(const bytevector &waveData) {
  const hal::boolean waveDataReinterpretCastCstringInvalid =
      static_cast<hal::boolean>(
          (waveData.size() < 12U) ||
          (string(reinterpret_cast<cstring>(waveData.data()), 4U) != "RIFF") ||
          (string(reinterpret_cast<cstring>(waveData.data() + 8U), 4U) !=
           "WAVE"));
  if (waveDataReinterpretCastCstringInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Espeak output is not a RIFF WAVE stream");
  }
  uint16 audioFormat{};
  uint16 channelCount{};
  uint16 bitsPerSample{};
  uint32 sampleRateHz{};
  size dataOffset{};
  size dataByteCount{};
  size offset = 12U;
  const hal::boolean waveChunkParsingRequested{true};
  while (waveChunkParsingRequested) {
    const hal::boolean waveChunkHeaderAvailable =
        static_cast<hal::boolean>((offset + 8U) <= waveData.size());
    if (waveChunkHeaderAvailable == false) {
      break;
    }
    const string chunkName(reinterpret_cast<cstring>(waveData.data() + offset),
                           4U);
    const uint32 declaredSize = readUint32(waveData, offset + 4U);
    const size chunkOffset = offset + 8U;
    const size availableSize = waveData.size() - chunkOffset;
    const size chunkSize =
        std::min(static_cast<size>(declaredSize), availableSize);
    if ((chunkName == "fmt ") && (chunkSize >= 16U)) {
      audioFormat = readUint16(waveData, chunkOffset);
      channelCount = readUint16(waveData, chunkOffset + 2U);
      sampleRateHz = readUint32(waveData, chunkOffset + 4U);
      bitsPerSample = readUint16(waveData, chunkOffset + 14U);
    } else if (chunkName == "data") {
      dataOffset = chunkOffset;
      dataByteCount = chunkSize;
      break;
    }
    const size paddedSize = chunkSize + (chunkSize % 2U);
    if (paddedSize > availableSize) {
      break;
    }
    offset = chunkOffset + paddedSize;
  }
  if ((audioFormat != 1U) || (bitsPerSample != 16U) || (channelCount == 0U) ||
      (channelCount > 8U) || (sampleRateHz == 0U) || (dataByteCount == 0U)) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Espeak WAV does not contain supported PCM data");
  }
  XWalkTextToSpeechPcmData result;
  result.sampleRateHz = sampleRateHz;
  result.channelCount = static_cast<uint8>(channelCount);
  result.pcmData.assign(waveData.data() + dataOffset,
                        waveData.data() + dataOffset + dataByteCount);
  return result;
}

/**
 * @brief Reads one little-endian unsigned sixteen-bit WAV field.
 * @param[in] bytes Complete WAV bytes.
 * @param[in] offset Valid first byte offset.
 * @return Decoded value.
 */
uint16 XWalkTextToSpeechEspeak::readUint16(const bytevector &bytes,
                                           size offset) {
  const uint16 low = bytes[offset];
  const uint16 high = static_cast<uint16>(bytes[offset + 1U]) << 8U;
  return static_cast<uint16>(low | high);
}

/**
 * @brief Reads one little-endian unsigned thirty-two-bit WAV field.
 * @param[in] bytes Complete WAV bytes.
 * @param[in] offset Valid first byte offset.
 * @return Decoded value.
 */
uint32 XWalkTextToSpeechEspeak::readUint32(const bytevector &bytes,
                                           size offset) {
  const uint32 byte0 = bytes[offset];
  const uint32 byte1 = static_cast<uint32>(bytes[offset + 1U]) << 8U;
  const uint32 byte2 = static_cast<uint32>(bytes[offset + 2U]) << 16U;
  const uint32 byte3 = static_cast<uint32>(bytes[offset + 3U]) << 24U;
  return byte0 | byte1 | byte2 | byte3;
}

} /* namespace xwalk::hal */
