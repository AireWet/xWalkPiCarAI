/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechPiper.cpp
 * @brief       Implements shell-free Piper synthesis and WAV playback.
 * @project     xWalk Firmware
 * @module      xWalkGPT Piper Provider
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarTextToSpeechPiper.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal {

XWalkTextToSpeechPiper::XWalkTextToSpeechPiper(stringview executable,
                                               stringview playbackExecutable,
                                               stringview model)
    : executableName(executable), playbackExecutableName(playbackExecutable),
      modelName(model) {
  const hal::boolean ttsConfigurationInvalid = static_cast<hal::boolean>(
      executableName.empty() || playbackExecutableName.empty() ||
      modelName.empty());
  if (ttsConfigurationInvalid) {
    XWALK_HAL_ERROR(
        XWALK_INVAL,
        "Piper executable, playback executable, and model are required");
  }
}

texttospeechspeakcallback XWalkTextToSpeechPiper::callback() const noexcept {
  return &speak;
}

XWalkTextToSpeechPiper &
XWalkTextToSpeechPiper::provider(contextpointer context) {
  if (context == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Piper provider context must not be null");
  }
  return *static_cast<XWalkTextToSpeechPiper *>(context);
}

void XWalkTextToSpeechPiper::speak(contextpointer context, stringview text) {
  const hal::boolean textEmpty = static_cast<hal::boolean>(text.empty());
  if (textEmpty) {
    return;
  }
  const hal::boolean textTooLarge =
      static_cast<hal::boolean>(text.size() > 4'096U);
  if (textTooLarge) {
    XWALK_HAL_ERROR(XWALK_RANGE, "Piper text exceeds the bounded input length");
  }
  provider(context).execute(text);
}

void XWalkTextToSpeechPiper::execute(stringview text) const {
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
    const int32 nullDescriptor = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
    if (nullDescriptor >= 0) {
      static_cast<void>(::dup2(nullDescriptor, STDERR_FILENO));
      static_cast<void>(::close(nullDescriptor));
    }
    ::execlp(executableName.c_str(), executableName.c_str(), "-m",
             modelName.c_str(), "-f", temporaryPath, "--", ownedText.c_str(),
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
    ::execlp(playbackExecutableName.c_str(), playbackExecutableName.c_str(),
             temporaryPath, static_cast<charpointer>(nullptr));
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

} /* namespace xwalk::hal */
