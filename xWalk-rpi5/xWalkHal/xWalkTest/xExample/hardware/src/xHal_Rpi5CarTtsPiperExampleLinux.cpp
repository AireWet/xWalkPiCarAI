/******************************************************************************
 * @file        xHal_Rpi5CarTtsPiperExampleLinux.cpp
 * @brief       Implements Linux composition for the Piper example.
 *
 * @details
 * Creates a private temporary WAV, invokes Piper and a WAV player as
 * bounded shell-free child processes, and removes the file after use.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
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

#include "xHal_Rpi5CarTtsPiperExampleLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains Linux composition for ported Robot HAT examples.
 */
namespace xwalk::hal::example {

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Stores and validates deployment-selected process executables.
 * @param[in] synthesisExecutable Non-empty Piper executable name or path.
 * @param[in] playbackExecutable Non-empty WAV player executable name or path.
 * @throws std::invalid_argument If either executable is empty.
 */
XWalkTtsPiperExampleLinux::XWalkTtsPiperExampleLinux(
    stringview synthesisExecutable, stringview playbackExecutable)
    : synthesisExecutableName(synthesisExecutable),
      playbackExecutableName(playbackExecutable) {
  const hal::boolean ttsConfigurationInvalid = static_cast<hal::boolean>(
      synthesisExecutableName.empty() || playbackExecutableName.empty());
  if (ttsConfigurationInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Piper synthesis and playback executables are required");
  }
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Delivers the fixed configured request through the Linux adapter.
 * @warning Creates a temporary file and produces audible output.
 */
void XWalkTtsPiperExampleLinux::run() {
  XWalkTtsPiperExample example(this, &speak);
  example.run();
}

/** @brief Synthesizes and plays caller-supplied text through the live adapter.
 */
void XWalkTtsPiperExampleLinux::speakText(stringview model, stringview text) {
  const hal::boolean modelTextInvalid =
      static_cast<hal::boolean>(model.empty() || text.empty());
  if (modelTextInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Piper model and speech text must not be empty");
  }
  speak(this, model, text);
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Resolves one non-null Linux callback context.
 * @param[in,out] context Non-null pointer to a live adapter.
 * @return Referenced live adapter.
 * @throws std::invalid_argument If `context` is null.
 */
XWalkTtsPiperExampleLinux &
XWalkTtsPiperExampleLinux::adapter(contextpointer context) {
  if (context == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Piper example Linux context must not be null");
  }
  return *static_cast<XWalkTtsPiperExampleLinux *>(context);
}

/**
 * @brief Synthesizes and plays one shell-free temporary-WAV request.
 * @param[in,out] context Non-null pointer to a live adapter.
 * @param[in] model Non-empty Piper voice-model name.
 * @param[in] text Non-empty speech text.
 * @throws std::runtime_error If temporary-file or child-process work fails.
 */
void XWalkTtsPiperExampleLinux::speak(contextpointer context, stringview model,
                                      stringview text) {
  XWalkTtsPiperExampleLinux &self = adapter(context);
  const string ownedModel(model);
  const string ownedText(text);
  char temporaryPath[]{"/tmp/xwalk-piper-XXXXXX.wav"};
  const int32 descriptor = ::mkstemps(temporaryPath, 4);
  if (descriptor < 0) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Piper temporary file creation failed");
  }
  const hal::boolean descriptorDifferent =
      static_cast<hal::boolean>(::close(descriptor) != 0);
  if (descriptorDifferent) {
    static_cast<void>(::unlink(temporaryPath));
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Piper temporary file close failed");
  }

  const auto synthesisProcess = ::fork();
  if (synthesisProcess < 0) {
    static_cast<void>(::unlink(temporaryPath));
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Piper process creation failed");
  }
  if (synthesisProcess == 0) {
    ::execlp(self.synthesisExecutableName.c_str(),
             self.synthesisExecutableName.c_str(), "-m", ownedModel.c_str(),
             "-f", temporaryPath, "--", ownedText.c_str(),
             static_cast<charpointer>(nullptr));
    ::_exit(127);
  }

  int32 synthesisStatus{};
  auto synthesisWait = ::waitpid(synthesisProcess, &synthesisStatus, 0);
  while ((synthesisWait < 0) && (errno == EINTR)) {
    synthesisWait = ::waitpid(synthesisProcess, &synthesisStatus, 0);
  }
  const hal::boolean synthesisFailed = static_cast<hal::boolean>(
      (synthesisWait != synthesisProcess) || !WIFEXITED(synthesisStatus) ||
      (WEXITSTATUS(synthesisStatus) != 0));
  if (synthesisFailed) {
    static_cast<void>(::unlink(temporaryPath));
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Piper synthesis executable failed");
  }

  const auto playbackProcess = ::fork();
  if (playbackProcess < 0) {
    static_cast<void>(::unlink(temporaryPath));
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Piper playback process creation failed");
  }
  if (playbackProcess == 0) {
    ::execlp(self.playbackExecutableName.c_str(),
             self.playbackExecutableName.c_str(), temporaryPath,
             static_cast<charpointer>(nullptr));
    ::_exit(127);
  }

  int32 playbackStatus{};
  auto playbackWait = ::waitpid(playbackProcess, &playbackStatus, 0);
  while ((playbackWait < 0) && (errno == EINTR)) {
    playbackWait = ::waitpid(playbackProcess, &playbackStatus, 0);
  }
  const int32 removeResult = ::unlink(temporaryPath);
  const hal::boolean playbackFailed = static_cast<hal::boolean>(
      (removeResult != 0) || (playbackWait != playbackProcess) ||
      !WIFEXITED(playbackStatus) || (WEXITSTATUS(playbackStatus) != 0));
  if (playbackFailed) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Piper playback executable failed");
  }
}

} /* namespace xwalk::hal::example */
