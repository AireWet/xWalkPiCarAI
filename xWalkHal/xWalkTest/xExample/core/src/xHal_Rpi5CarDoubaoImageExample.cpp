/******************************************************************************
 * @file        xHal_Rpi5CarDoubaoImageExample.cpp
 * @brief       Implements the bounded Doubao camera-chat example flow.
 *
 * @project     xWalk Firmware
 * @module      xExample
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarDoubaoImageExample.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains contracts and adapters for ported example programs.
 */
namespace xwalk::hal::example {

/**
 * @brief Binds injected dependencies and validates the console table.
 * @param[in,out] camera Caller-owned camera.
 * @param[in,out] languageModel Caller-owned model.
 * @param[in,out] context Non-owning console context.
 * @param[in] consoleCallbacks Complete console table.
 * @throws std::invalid_argument If either callback is null.
 */
XWalkDoubaoImageExample::XWalkDoubaoImageExample(
    XWalkCamera &camera, XWalkLanguageModel &languageModel,
    contextpointer context,
    const XWalkDoubaoImageExampleCallbacks &consoleCallbacks)
    : cameraObject(&camera), languageModelObject(&languageModel),
      consoleContext(context), callbacks(consoleCallbacks) {
  if ((callbacks.readPrompt == nullptr) || (callbacks.write == nullptr)) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Doubao image example requires complete console callbacks");
  }
}

/**
 * @brief Runs the welcome, capture, and image-prompt sequence.
 * @param[in] maximumPrompts Prompt limit from one through 100.
 * @param[in] imagePath Non-empty destination reused for captured JPEG images.
 * @throws std::out_of_range If `maximumPrompts` is outside its range.
 * @warning Each completed capture may be uploaded by the language-model
 * provider.
 */
void XWalkDoubaoImageExample::run(uint32 maximumPrompts, stringview imagePath) {
  if ((maximumPrompts == 0U) ||
      (maximumPrompts > XHAL_RPI5CAR_DOUBAO_IMAGE_EXAMPLE_MAXIMUM_PROMPTS)) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "Doubao image example prompt count is outside its range");
  }

  constexpr stringview instructions{"You are a helpful assistant."};
  constexpr stringview welcome{
      "Hello, I am a helpful assistant. How can I help you?"};
  languageModelObject->setMaximumMessages(20U);
  languageModelObject->setInstructions(instructions);
  languageModelObject->setWelcome(welcome);
  callbacks.write(consoleContext, welcome, true, false);

  for (uint32 promptIndex = 0U; promptIndex < maximumPrompts; ++promptIndex) {
    string inputText;
    const hal::boolean promptRead =
        callbacks.readPrompt(consoleContext, inputText);
    if (promptRead == false) {
      break;
    }
    const string capturedImagePath = cameraObject->capture(imagePath);
    const string response =
        languageModelObject->prompt(inputText, capturedImagePath);
    const hal::boolean responseAvailable =
        static_cast<hal::boolean>(!response.empty());
    if (responseAvailable) {
      callbacks.write(consoleContext, response, false, true);
    }
    callbacks.write(consoleContext, {}, true, false);
  }
}

} /* namespace xwalk::hal::example */
