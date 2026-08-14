/******************************************************************************
 * @file        xHal_Rpi5CarTtsOpenAiExampleLinux.cpp
 * @brief       Implements Linux composition for the OpenAI TTS example.
 *
 * @details
 * Sends bounded authenticated JSON requests through libcurl, stores only MP3
 * response bytes, and invokes a selected player without using a shell.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarTtsOpenAiExampleLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include "xHal_Rpi5CarTrace.h"
#include <curl/curl.h>
#include <iostream>

namespace xwalk::hal::example {

/** @brief Maximum accepted MP3 response size. */
constexpr size XWALK_TTS_OPEN_AI_MAXIMUM_RESPONSE_BYTES{16U * 1024U * 1024U};
/** @brief Synchronous speech request timeout in milliseconds. */
constexpr long XWALK_TTS_OPEN_AI_TIMEOUT_MS{120000L};

/** @brief Stores and validates OpenAI and playback configuration. */
XWalkTtsOpenAiExampleLinux::XWalkTtsOpenAiExampleLinux(stringview apiKey,
                                                       stringview executable,
                                                       stringview endpoint)
    : apiKeyValue(apiKey), endpointValue(endpoint), executableName(executable) {
  const hal::boolean apiKeyEndpointExecutableNameInvalid =
      static_cast<hal::boolean>(apiKeyValue.empty() || endpointValue.empty() ||
                                executableName.empty());
  if (apiKeyEndpointExecutableNameInvalid) {
    XWALK_HAL_ERROR(
        XWALK_INVAL,
        "OpenAI TTS key, endpoint, and playback executable are required");
  }
}

/** @brief Delivers all fixed requests through the live Linux adapter. */
void XWalkTtsOpenAiExampleLinux::run() {
  XWalkTtsOpenAiExample example(this, &speak, &report);
  example.run();
}

/** @brief Resolves one non-null Linux callback context. */
XWalkTtsOpenAiExampleLinux &
XWalkTtsOpenAiExampleLinux::adapter(contextpointer context) {
  if (context == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "OpenAI TTS Linux context must not be null");
  }
  return *static_cast<XWalkTtsOpenAiExampleLinux *>(context);
}

/** @brief Escapes quotes, slashes, and control bytes for JSON. */
string XWalkTtsOpenAiExampleLinux::escapeJson(stringview value) {
  constexpr char hexDigits[]{"0123456789abcdef"};
  string escaped;
  escaped.reserve(value.size());
  for (const char character : value) {
    const uint8 byteValue = static_cast<uint8>(character);
    if ((character == '\"') || (character == '\\')) {
      escaped.push_back('\\');
      escaped.push_back(character);
    } else if (byteValue < 0x20U) {
      escaped.append("\\u00");
      escaped.push_back(hexDigits[(byteValue >> 4U) & 0x0FU]);
      escaped.push_back(hexDigits[byteValue & 0x0FU]);
    } else {
      escaped.push_back(character);
    }
  }
  return escaped;
}

/** @brief Appends one response block without exceeding the MP3 bound. */
size XWalkTtsOpenAiExampleLinux::writeResponse(charpointer data, size itemSize,
                                               size itemCount,
                                               contextpointer context) {
  const hal::boolean contextItemCountItemSizeInvalid =
      static_cast<hal::boolean>(
          (data == nullptr) || (context == nullptr) ||
          ((itemCount != 0U) &&
           (itemSize > (std::numeric_limits<size>::max() / itemCount))));
  if (contextItemCountItemSizeInvalid) {
    return 0U;
  }
  XWalkTtsOpenAiExampleLinux &self = adapter(context);
  const size byteCount = itemSize * itemCount;
  const hal::boolean selfResponseDataByteCountInvalid =
      static_cast<hal::boolean>(
          (self.responseData.size() >
           XWALK_TTS_OPEN_AI_MAXIMUM_RESPONSE_BYTES) ||
          (byteCount > (XWALK_TTS_OPEN_AI_MAXIMUM_RESPONSE_BYTES -
                        self.responseData.size())));
  if (selfResponseDataByteCountInvalid) {
    return 0U;
  }
  self.responseData.append(data, byteCount);
  return byteCount;
}

/** @brief Synthesizes and plays one fixed request. */
void XWalkTtsOpenAiExampleLinux::speak(contextpointer context, stringview model,
                                       stringview voice, stringview text,
                                       stringview instructions) {
  XWalkTtsOpenAiExampleLinux &self = adapter(context);
  self.requestSpeech(model, voice, text, instructions);
  self.playResponse();
}

/** @brief Prints one exact source-compatible line. */
void XWalkTtsOpenAiExampleLinux::report(contextpointer context,
                                        stringview message) {
  static_cast<void>(adapter(context));
  std::cout << message << '\n';
}

/** @brief Posts one authenticated JSON request and captures bounded MP3 bytes.
 */
void XWalkTtsOpenAiExampleLinux::requestSpeech(stringview model,
                                               stringview voice,
                                               stringview text,
                                               stringview instructions) {
  string requestJson{"{\"model\":\""};
  requestJson.append(escapeJson(model));
  requestJson.append("\",\"voice\":\"");
  requestJson.append(escapeJson(voice));
  requestJson.append("\",\"input\":\"");
  requestJson.append(escapeJson(text));
  const hal::boolean instructionsAvailable =
      static_cast<hal::boolean>(!instructions.empty());
  if (instructionsAvailable) {
    requestJson.append("\",\"instructions\":\"");
    requestJson.append(escapeJson(instructions));
  }
  requestJson.append("\",\"response_format\":\"mp3\"}");

  static const CURLcode initialization =
      ::curl_global_init(CURL_GLOBAL_DEFAULT);
  if (initialization != CURLE_OK) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS libcurl initialization failed");
  }
  CURL *request = ::curl_easy_init();
  if (request == nullptr) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS request allocation failed");
  }
  curl_slist *headers =
      ::curl_slist_append(nullptr, "Content-Type: application/json");
  if (headers == nullptr) {
    ::curl_easy_cleanup(request);
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS header allocation failed");
  }
  string authorizationHeader{"Authorization: Bearer "};
  authorizationHeader.append(apiKeyValue);
  curl_slist *authenticatedHeaders =
      ::curl_slist_append(headers, authorizationHeader.c_str());
  if (authenticatedHeaders == nullptr) {
    ::curl_slist_free_all(headers);
    ::curl_easy_cleanup(request);
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS header allocation failed");
  }
  headers = authenticatedHeaders;
  responseData.clear();
  const curl_off_t requestSize = static_cast<curl_off_t>(requestJson.size());
  CURLcode result =
      ::curl_easy_setopt(request, CURLOPT_URL, endpointValue.c_str());
  if (result == CURLE_OK) {
    result = ::curl_easy_setopt(request, CURLOPT_HTTPHEADER, headers);
  }
  if (result == CURLE_OK) {
    result =
        ::curl_easy_setopt(request, CURLOPT_POSTFIELDS, requestJson.data());
  }
  if (result == CURLE_OK) {
    result =
        ::curl_easy_setopt(request, CURLOPT_POSTFIELDSIZE_LARGE, requestSize);
  }
  if (result == CURLE_OK) {
    result = ::curl_easy_setopt(request, CURLOPT_TIMEOUT_MS,
                                XWALK_TTS_OPEN_AI_TIMEOUT_MS);
  }
  if (result == CURLE_OK) {
    result = ::curl_easy_setopt(request, CURLOPT_NOSIGNAL, 1L);
  }
  if (result == CURLE_OK) {
    result = ::curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, &writeResponse);
  }
  if (result == CURLE_OK) {
    result = ::curl_easy_setopt(request, CURLOPT_WRITEDATA, this);
  }
  if (result != CURLE_OK) {
    ::curl_slist_free_all(headers);
    ::curl_easy_cleanup(request);
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS request configuration failed");
  }
  const CURLcode transferResult = ::curl_easy_perform(request);
  long statusCode{};
  const CURLcode statusResult =
      ::curl_easy_getinfo(request, CURLINFO_RESPONSE_CODE, &statusCode);
  ::curl_slist_free_all(headers);
  ::curl_easy_cleanup(request);
  if (transferResult != CURLE_OK) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS request failed");
  }
  const hal::boolean statusResultStatusCodeResponseDataInvalid =
      static_cast<hal::boolean>((statusResult != CURLE_OK) ||
                                (statusCode < 200L) || (statusCode >= 300L) ||
                                responseData.empty());
  if (statusResultStatusCodeResponseDataInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS response is unsuccessful");
  }
}

/** @brief Writes, plays, and removes one bounded temporary MP3 file. */
void XWalkTtsOpenAiExampleLinux::playResponse() {
  char temporaryPath[]{"/tmp/xwalk-openai-tts-XXXXXX"};
  const int32 descriptor = ::mkstemp(temporaryPath);
  if (descriptor < 0) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS temporary file creation failed");
  }
  size writtenBytes{};
  const hal::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const hal::boolean responseBytesRemaining =
        static_cast<hal::boolean>(writtenBytes < responseData.size());
    if (responseBytesRemaining == false) {
      break;
    }
    const ssize_t result =
        ::write(descriptor, responseData.data() + writtenBytes,
                responseData.size() - writtenBytes);
    if ((result < 0) && (errno == EINTR)) {
      continue;
    }
    if (result <= 0) {
      static_cast<void>(::close(descriptor));
      static_cast<void>(::unlink(temporaryPath));
      XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS temporary file write failed");
    }
    writtenBytes += static_cast<size>(result);
  }
  const hal::boolean descriptorDifferent =
      static_cast<hal::boolean>(::close(descriptor) != 0);
  if (descriptorDifferent) {
    static_cast<void>(::unlink(temporaryPath));
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS temporary file close failed");
  }

  const auto childProcess = ::fork();
  if (childProcess < 0) {
    static_cast<void>(::unlink(temporaryPath));
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "OpenAI TTS playback process creation failed");
  }
  if (childProcess == 0) {
    ::execlp(executableName.c_str(), executableName.c_str(), "--no-video",
             "--really-quiet", temporaryPath,
             static_cast<charpointer>(nullptr));
    ::_exit(127);
  }

  int32 processStatus{};
  auto waitResult = ::waitpid(childProcess, &processStatus, 0);
  while ((waitResult < 0) && (errno == EINTR)) {
    waitResult = ::waitpid(childProcess, &processStatus, 0);
  }
  const int32 removeResult = ::unlink(temporaryPath);
  const hal::boolean removeResultWaitResultChildProcessInvalid =
      static_cast<hal::boolean>(
          (removeResult != 0) || (waitResult != childProcess) ||
          !WIFEXITED(processStatus) || (WEXITSTATUS(processStatus) != 0));
  if (removeResultWaitResultChildProcessInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "OpenAI TTS playback executable failed");
  }
}

} /* namespace xwalk::hal::example */
