/******************************************************************************
 * @file        xHal_Rpi5CarTtsEspeakExampleLinux.cpp
 * @brief       Implements Linux process composition for the Espeak example.
 *
 * @details
 * Converts numeric settings into independent child-process arguments, invokes
 * the selected executable without a shell, and validates its exit status.
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

#include "xHal_Rpi5CarTtsEspeakExampleLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::example {

/** @brief Stores and validates the deployment-selected executable. */
XWalkTtsEspeakExampleLinux::XWalkTtsEspeakExampleLinux(stringview executable)
    : executableName(executable) {
  const hal::boolean executableNameEmpty =
      static_cast<hal::boolean>(executableName.empty());
  if (executableNameEmpty) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Espeak playback executable is required");
  }
}

/** @brief Delivers the fixed configured request through the process adapter. */
void XWalkTtsEspeakExampleLinux::run() {
  XWalkTtsEspeakExample example(this, &speak);
  example.run();
}

/** @brief Resolves one non-null Linux callback context. */
XWalkTtsEspeakExampleLinux &
XWalkTtsEspeakExampleLinux::adapter(contextpointer context) {
  if (context == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Espeak example Linux context must not be null");
  }
  return *static_cast<XWalkTtsEspeakExampleLinux *>(context);
}

/** @brief Executes and waits for one shell-free configured Espeak process. */
void XWalkTtsEspeakExampleLinux::speak(contextpointer context, uint8 amplitude,
                                       uint16 speed, uint16 gap, uint8 pitch,
                                       stringview text) {
  XWalkTtsEspeakExampleLinux &self = adapter(context);
  const string amplitudeText = std::to_string(amplitude);
  const string speedText = std::to_string(speed);
  const string gapText = std::to_string(gap);
  const string pitchText = std::to_string(pitch);
  const string ownedText(text);
  const auto childProcess = ::fork();
  if (childProcess < 0) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Espeak example process creation failed");
  }
  if (childProcess == 0) {
    ::execlp(self.executableName.c_str(), self.executableName.c_str(), "-a",
             amplitudeText.c_str(), "-s", speedText.c_str(), "-g",
             gapText.c_str(), "-p", pitchText.c_str(), ownedText.c_str(),
             static_cast<charpointer>(nullptr));
    ::_exit(127);
  }

  int32 processStatus{};
  auto waitResult = ::waitpid(childProcess, &processStatus, 0);
  while ((waitResult < 0) && (errno == EINTR)) {
    waitResult = ::waitpid(childProcess, &processStatus, 0);
  }
  const hal::boolean waitResultChildProcessProcessStatusInvalid =
      static_cast<hal::boolean>((waitResult != childProcess) ||
                                !WIFEXITED(processStatus) ||
                                (WEXITSTATUS(processStatus) != 0));
  if (waitResultChildProcessProcessStatusInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Espeak playback executable failed");
  }
}

} /* namespace xwalk::hal::example */
