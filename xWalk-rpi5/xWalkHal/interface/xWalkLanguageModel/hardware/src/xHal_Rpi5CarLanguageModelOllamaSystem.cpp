/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelOllamaSystem.cpp
 * @brief       Implements the real bounded Ollama libcurl transport.
 *
 * @details
 * Performs synchronous application/json POST requests with explicit timeout,
 * response-size, HTTP-status, and process-lifetime libcurl initialization.
 *
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Ollama Backend
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

#include "xHal_Rpi5CarLanguageModelOllama.h"

#include "xHal_Rpi5CarTrace.h"
#include <curl/curl.h>

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
 * @brief Returns the complete real libcurl transport operation table.
 * @return One non-null synchronous JSON POST operation.
 */
XWalkLanguageModelOllamaOperations
XWalkLanguageModelOllama::systemOperations() noexcept {
  return {&systemPostJson};
}

/**
 * @brief Sends one real bounded JSON POST request through libcurl.
 *
 * @param[in,out] context Unused nullable transport context.
 * @param[in] endpoint Non-empty Ollama chat endpoint.
 * @param[in] requestJson Bounded complete JSON request.
 * @param[in] authorizationHeader Empty text or one complete authorization
 * header.
 * @param[in] timeoutMs Request timeout in milliseconds.
 * @param[in] maximumResponseBytes Maximum accepted response bytes.
 * @return Owned successful two-hundred-range JSON response.
 * @throws std::runtime_error If libcurl setup, transfer, or HTTP status fails.
 */
string XWalkLanguageModelOllama::systemPostJson(contextpointer context,
                                                stringview endpoint,
                                                stringview requestJson,
                                                stringview authorizationHeader,
                                                uint32 timeoutMs,
                                                size maximumResponseBytes) {
  static_cast<void>(context);
  static const CURLcode initialization =
      ::curl_global_init(CURL_GLOBAL_DEFAULT);
  if (initialization != CURLE_OK) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Ollama libcurl initialization failed");
  }
  XWalkLanguageModelOllamaResponseState responseState{{}, maximumResponseBytes};
  responseState.response.reserve(maximumResponseBytes);
  const string endpointCopy{endpoint};
  CURL *request = ::curl_easy_init();
  if (request == nullptr) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Ollama libcurl request allocation failed");
  }
  curl_slist *headers =
      ::curl_slist_append(nullptr, "Content-Type: application/json");
  if (headers == nullptr) {
    ::curl_easy_cleanup(request);
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Ollama libcurl header allocation failed");
  }
  const string authorizationHeaderCopy{authorizationHeader};
  const hal::boolean authorizationHeaderCopyAvailable =
      static_cast<hal::boolean>(!authorizationHeaderCopy.empty());
  if (authorizationHeaderCopyAvailable) {
    curl_slist *authenticatedHeaders =
        ::curl_slist_append(headers, authorizationHeaderCopy.c_str());
    if (authenticatedHeaders == nullptr) {
      ::curl_slist_free_all(headers);
      ::curl_easy_cleanup(request);
      XWALK_HAL_ERROR(XWALK_RUNTIME,
                      "Language-model authorization header allocation failed");
    }
    headers = authenticatedHeaders;
  }
  const long timeoutValue = static_cast<long>(timeoutMs);
  const curl_off_t requestSize = static_cast<curl_off_t>(requestJson.size());
  CURLcode optionResult =
      ::curl_easy_setopt(request, CURLOPT_URL, endpointCopy.c_str());
  if (optionResult == CURLE_OK) {
    optionResult = ::curl_easy_setopt(request, CURLOPT_HTTPHEADER, headers);
  }
  if (optionResult == CURLE_OK) {
    optionResult =
        ::curl_easy_setopt(request, CURLOPT_POSTFIELDS, requestJson.data());
  }
  if (optionResult == CURLE_OK) {
    optionResult =
        ::curl_easy_setopt(request, CURLOPT_POSTFIELDSIZE_LARGE, requestSize);
  }
  if (optionResult == CURLE_OK) {
    optionResult =
        ::curl_easy_setopt(request, CURLOPT_TIMEOUT_MS, timeoutValue);
  }
  if (optionResult == CURLE_OK) {
    optionResult = ::curl_easy_setopt(request, CURLOPT_NOSIGNAL, 1L);
  }
  if (optionResult == CURLE_OK) {
    optionResult = ::curl_easy_setopt(request, CURLOPT_WRITEFUNCTION,
                                      &systemWriteResponse);
  }
  if (optionResult == CURLE_OK) {
    optionResult =
        ::curl_easy_setopt(request, CURLOPT_WRITEDATA, &responseState);
  }
  if (optionResult != CURLE_OK) {
    ::curl_slist_free_all(headers);
    ::curl_easy_cleanup(request);
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Ollama libcurl option configuration failed");
  }
  const CURLcode transferResult = ::curl_easy_perform(request);
  long statusCode{};
  const CURLcode statusResult =
      ::curl_easy_getinfo(request, CURLINFO_RESPONSE_CODE, &statusCode);
  ::curl_slist_free_all(headers);
  ::curl_easy_cleanup(request);
  if (transferResult != CURLE_OK) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Ollama HTTP request failed");
  }
  if ((statusResult != CURLE_OK) || (statusCode < 200L) ||
      (statusCode >= 300L)) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Ollama HTTP status is unsuccessful");
  }
  return responseState.response;
}

/**
 * @brief Appends one bounded libcurl response block.
 *
 * @param[in] data Received byte block.
 * @param[in] itemSize Byte width of one received item.
 * @param[in] itemCount Number of received items.
 * @param[in,out] userData Non-null owned response string for this request.
 * @return Accepted byte count, or zero when the response limit would be
 * exceeded.
 */
size XWalkLanguageModelOllama::systemWriteResponse(charpointer data,
                                                   size itemSize,
                                                   size itemCount,
                                                   contextpointer userData) {
  const hal::boolean userDataItemCountItemSizeInvalid =
      static_cast<hal::boolean>(
          (data == nullptr) || (userData == nullptr) ||
          ((itemCount != 0U) &&
           (itemSize > (std::numeric_limits<size>::max() / itemCount))));
  if (userDataItemCountItemSizeInvalid) {
    return 0U;
  }
  const size byteCount = itemSize * itemCount;
  XWalkLanguageModelOllamaResponseState &state =
      *static_cast<XWalkLanguageModelOllamaResponseState *>(userData);
  const hal::boolean stateResponseMaximumBytesInvalid =
      static_cast<hal::boolean>(
          (state.response.size() > state.maximumBytes) ||
          (byteCount > (state.maximumBytes - state.response.size())));
  if (stateResponseMaximumBytesInvalid) {
    return 0U;
  }
  state.response.append(data, byteCount);
  return byteCount;
}

} /* namespace xwalk::hal */
