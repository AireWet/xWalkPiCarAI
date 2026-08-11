/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelHardwareTest.cpp
 * @brief       Provides an explicit one-request Ollama provider smoke test.
 *
 * @details
 * Contacts only the endpoint and model supplied by deployment, performs one
 * bounded text-only request, and does not print prompt or response content.
 *
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Ollama Hardware Test
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

#include "xHal_Rpi5CarLanguageModel.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Performs one explicitly configured Ollama chat request.
 *
 * @param[in] argumentCount Exactly four arguments are required.
 * @param[in] argumentValues Program name, endpoint, model, and bounded prompt.
 * @return Zero after one successful final response, including an empty
 * response.
 * @warning Run only after approving the endpoint, model, prompt, and network
 * policy.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount,
                     XWalkHal::charpointer argumentValues[]) {
  if (argumentCount != 4) {
    XWALK_HAL_ERROR(
        XWALK_INVAL,
        "Ollama hardware test requires endpoint, model, and prompt");
  }
  XWalkHal::XWalkLanguageModelOllama backend(argumentValues[1],
                                             argumentValues[2]);
  XWalkHal::XWalkLanguageModel model(&backend, backend.callbacks());
  model.setMaximumMessages(4U);
  static_cast<void>(model.prompt(argumentValues[3]));
  return 0;
}
