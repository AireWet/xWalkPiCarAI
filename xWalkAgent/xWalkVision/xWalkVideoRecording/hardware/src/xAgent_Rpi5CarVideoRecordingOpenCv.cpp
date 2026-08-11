/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingOpenCv.cpp
 * @brief       Implements continuous OpenCV AVI recording.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording OpenCV Backend
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoRecordingOpenCv.h"

#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

#include "xHal_Rpi5CarTrace.h"
#include <cmath>
#include <memory>

namespace xwalk::agent {

XWalkVideoRecordingOpenCv::XWalkVideoRecordingOpenCv(
    const XWalkVideoRecordingOpenCvConfiguration &configuration)
    : configurationValue(configuration) {
  const agent::boolean configurationInvalid =
      static_cast<agent::boolean>(configurationValue.cameraDevice.empty() ||
                                  configurationValue.videoDirectory.empty());
  if (configurationInvalid) {
    XWALK_RPIAGENT_ERROR(XWALK_INVAL,
                         "Video-recording paths must not be empty");
  }
  const agent::boolean pathSource =
      configurationValue.cameraBackend ==
          XWalkVideoRecordingOpenCvBackend::V4l2 ||
      configurationValue.cameraBackend ==
          XWalkVideoRecordingOpenCvBackend::VideoFile ||
      configurationValue.cameraBackend ==
          XWalkVideoRecordingOpenCvBackend::ImageSequence;
  const agent::boolean videoPathConfigurationInvalid = static_cast<
      agent::boolean>(
      (pathSource &&
       !agent::filesystempath(configurationValue.cameraDevice).is_absolute()) ||
      (configurationValue.cameraDevice.find('\n') != agent::string::npos) ||
      (configurationValue.cameraDevice.find('\r') != agent::string::npos) ||
      !agent::filesystempath(configurationValue.videoDirectory).is_absolute());
  if (videoPathConfigurationInvalid) {
    XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Video-recording paths must be absolute");
  }
  const agent::boolean videoFormatInvalid = static_cast<agent::boolean>(
      (configurationValue.widthPixels < 16U) ||
      (configurationValue.widthPixels > 7'680U) ||
      (configurationValue.heightPixels < 16U) ||
      (configurationValue.heightPixels > 4'320U) ||
      !std::isfinite(configurationValue.framesPerSecond) ||
      (configurationValue.framesPerSecond < 1.0) ||
      (configurationValue.framesPerSecond > 120.0) ||
      (configurationValue.readTimeoutMilliseconds == 0U) ||
      (configurationValue.readTimeoutMilliseconds > 60'000U));
  if (videoFormatInvalid) {
    XWALK_RPIAGENT_ERROR(XWALK_RANGE,
                         "Video-recording configuration is out of range");
  }
}

XWalkVideoRecordingOpenCvBackend
XWalkVideoRecordingOpenCv::backendFromString(agent::stringview backend) {
  if (backend == "v4l2") {
    return XWalkVideoRecordingOpenCvBackend::V4l2;
  }
  if (backend == "gstreamer") {
    return XWalkVideoRecordingOpenCvBackend::Gstreamer;
  }
  if (backend == "video_file") {
    return XWalkVideoRecordingOpenCvBackend::VideoFile;
  }
  if (backend == "image_sequence") {
    return XWalkVideoRecordingOpenCvBackend::ImageSequence;
  }
  if (backend == "automatic") {
    return XWalkVideoRecordingOpenCvBackend::Automatic;
  }
  XWALK_RPIAGENT_ERROR(XWALK_INVAL,
                       "video_recording_camera_backend is unsupported");
}

XWalkVideoRecordingOpenCv::~XWalkVideoRecordingOpenCv() noexcept {
  stopProvider();
}

XWalkVideoRecordingOpenCv &
XWalkVideoRecordingOpenCv::provider(agent::contextpointer context) {
  if (context == nullptr) {
    XWALK_RPIAGENT_ERROR(XWALK_INVAL,
                         "Video-recording provider context is null");
  }
  return *static_cast<XWalkVideoRecordingOpenCv *>(context);
}

agent::boolean
XWalkVideoRecordingOpenCv::startCamera(agent::contextpointer context) {
  XWalkVideoRecordingOpenCv &backend = provider(context);
  const agent::boolean cameraOpen =
      static_cast<agent::boolean>(backend.camera.isOpened());
  if (cameraOpen) {
    return true;
  }
  int apiPreference = cv::CAP_ANY;
  if (backend.configurationValue.cameraBackend ==
      XWalkVideoRecordingOpenCvBackend::V4l2) {
    apiPreference = cv::CAP_V4L2;
  } else if (backend.configurationValue.cameraBackend ==
             XWalkVideoRecordingOpenCvBackend::Gstreamer) {
    apiPreference = cv::CAP_GSTREAMER;
  }
  const agent::boolean cameraOpened = backend.camera.open(
      backend.configurationValue.cameraDevice, apiPreference);
  if (cameraOpened == false) {
    return false;
  }
  static_cast<void>(backend.camera.set(
      cv::CAP_PROP_FRAME_WIDTH,
      static_cast<agent::float64>(backend.configurationValue.widthPixels)));
  static_cast<void>(backend.camera.set(
      cv::CAP_PROP_FRAME_HEIGHT,
      static_cast<agent::float64>(backend.configurationValue.heightPixels)));
  static_cast<void>(backend.camera.set(
      cv::CAP_PROP_READ_TIMEOUT_MSEC,
      static_cast<agent::float64>(
          backend.configurationValue.readTimeoutMilliseconds)));
  backend.frameWidthPixelsValue = static_cast<agent::uint32>(
      std::lround(backend.camera.get(cv::CAP_PROP_FRAME_WIDTH)));
  backend.frameHeightPixelsValue = static_cast<agent::uint32>(
      std::lround(backend.camera.get(cv::CAP_PROP_FRAME_HEIGHT)));
  if ((backend.frameWidthPixelsValue < 16U) ||
      (backend.frameWidthPixelsValue > 7'680U) ||
      (backend.frameHeightPixelsValue < 16U) ||
      (backend.frameHeightPixelsValue > 4'320U)) {
    backend.camera.release();
    return false;
  }
  backend.stopRequested.store(false);
  backend.workerFailed.store(false);
  backend.sourceEnded.store(false);
  backend.recordingValue = false;
  backend.pausedValue = false;
  const auto rollbackStart = [&backend](void *) noexcept {
    backend.camera.release();
  };
  std::unique_ptr<void, decltype(rollbackStart)> rollbackGuard(&backend,
                                                               rollbackStart);
  backend.captureThread =
      std::thread(&XWalkVideoRecordingOpenCv::captureLoop, &backend);
  const agent::contextpointer releasedContext = rollbackGuard.release();
  static_cast<void>(releasedContext);
  return true;
}

void XWalkVideoRecordingOpenCv::stopCamera(
    agent::contextpointer context) noexcept {
  if (context != nullptr) {
    static_cast<XWalkVideoRecordingOpenCv *>(context)->stopProvider();
  }
}

agent::string
XWalkVideoRecordingOpenCv::beginRecording(agent::contextpointer context,
                                          agent::stringview recordingName) {
  XWalkVideoRecordingOpenCv &backend = provider(context);
  const agent::boolean recordingNameBackendWorkerFailedInvalid =
      static_cast<agent::boolean>(recordingName.empty() ||
                                  backend.workerFailed.load() ||
                                  backend.sourceEnded.load());
  if (recordingNameBackendWorkerFailedInvalid) {
    XWALK_RPIAGENT_ERROR(XWALK_RUNTIME,
                         "Video-recording provider is unavailable");
  }
  const agent::filesystempath directory{
      backend.configurationValue.videoDirectory};
  const agent::boolean directoryExists = hal::filesystemEntryExists(directory);
  if (directoryExists == false) {
    const agent::boolean directoryCreated = hal::createDirectories(directory);
    if (directoryCreated == false) {
      XWALK_RPIAGENT_ERROR(XWALK_RUNTIME,
                           "Video directory could not be created");
    }
  }
  const agent::filesystempath path =
      directory / (agent::string(recordingName) + ".avi");
  std::lock_guard<std::mutex> lock(backend.stateMutex);
  const agent::boolean backendCaptureThreadJoinableInvalid =
      static_cast<agent::boolean>(!backend.captureThread.joinable() ||
                                  backend.writer.isOpened());
  if (backendCaptureThreadJoinableInvalid) {
    XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "Video recorder is not ready to start");
  }
  const int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
  const agent::boolean writerOpened = backend.writer.open(
      path.string(), codec, backend.configurationValue.framesPerSecond,
      cv::Size(static_cast<int>(backend.frameWidthPixelsValue),
               static_cast<int>(backend.frameHeightPixelsValue)));
  if (writerOpened == false) {
    XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, "AVI output could not be opened");
  }
  backend.outputPathValue = path.string();
  backend.pausedValue = false;
  backend.recordingValue = true;
  return backend.outputPathValue;
}

void XWalkVideoRecordingOpenCv::pauseRecording(agent::contextpointer context) {
  XWalkVideoRecordingOpenCv &backend = provider(context);
  std::lock_guard<std::mutex> lock(backend.stateMutex);
  if (!backend.recordingValue) {
    XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "Video recording is not active");
  }
  backend.pausedValue = true;
}

void XWalkVideoRecordingOpenCv::continueRecording(
    agent::contextpointer context) {
  XWalkVideoRecordingOpenCv &backend = provider(context);
  std::lock_guard<std::mutex> lock(backend.stateMutex);
  if (!backend.recordingValue) {
    XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "Video recording is not active");
  }
  backend.pausedValue = false;
}

void XWalkVideoRecordingOpenCv::stopRecording(
    agent::contextpointer context) noexcept {
  XWalkVideoRecordingOpenCv &backend = provider(context);
  std::lock_guard<std::mutex> lock(backend.stateMutex);
  const agent::boolean writerOpen =
      static_cast<agent::boolean>(backend.writer.isOpened());
  if (writerOpen) {
    backend.writer.release();
  }
  backend.recordingValue = false;
  backend.pausedValue = false;
}

agent::string
XWalkVideoRecordingOpenCv::timestamp(agent::contextpointer context) {
  static_cast<void>(provider(context));
  const time_t currentTime = ::time(nullptr);
  struct tm localTime {};
  const agent::boolean currentTimeLocalTimeMatched =
      static_cast<agent::boolean>(::localtime_r(&currentTime, &localTime) ==
                                  nullptr);
  if (currentTimeLocalTimeMatched) {
    XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, "Video timestamp failed");
  }
  agent::fixedarray<char, 32U> value{};
  const agent::boolean valueMDMatched = static_cast<agent::boolean>(
      ::strftime(value.data(), value.size(), "%Y-%m-%d-%H.%M.%S", &localTime) ==
      0U);
  if (valueMDMatched) {
    XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, "Video timestamp formatting failed");
  }
  return value.data();
}

void XWalkVideoRecordingOpenCv::captureLoop() noexcept {
  try {
    const agent::boolean processingLoopRequested{true};
    while (processingLoopRequested) {
      const agent::boolean operationMayContinue =
          static_cast<agent::boolean>(!stopRequested.load());
      if (operationMayContinue == false) {
        break;
      }
      cv::Mat frame;
      const agent::boolean frameRead = camera.read(frame);
      const agent::boolean frameUnavailable =
          static_cast<agent::boolean>((frameRead == false) || frame.empty());
      if (frameUnavailable) {
        const agent::boolean finiteSource =
            configurationValue.cameraBackend ==
                XWalkVideoRecordingOpenCvBackend::VideoFile ||
            configurationValue.cameraBackend ==
                XWalkVideoRecordingOpenCvBackend::ImageSequence;
        if (finiteSource) {
          sourceEnded.store(true);
        } else {
          workerFailed.store(true);
        }
        break;
      }
      std::lock_guard<std::mutex> lock(stateMutex);
      const agent::boolean frameReadyToWrite = static_cast<agent::boolean>(
          recordingValue && !pausedValue && writer.isOpened());
      if (frameReadyToWrite) {
        writer.write(frame);
      }
    }
  } catch (...) {
    workerFailed.store(true);
  }
}

void XWalkVideoRecordingOpenCv::stopProvider() noexcept {
  stopRequested.store(true);
  const agent::boolean captureThreadJoinable =
      static_cast<agent::boolean>(captureThread.joinable());
  if (captureThreadJoinable) {
    captureThread.join();
  }
  try {
    std::lock_guard<std::mutex> lock(stateMutex);
    const agent::boolean writerOpen =
        static_cast<agent::boolean>(writer.isOpened());
    if (writerOpen) {
      writer.release();
    }
    const agent::boolean cameraOpen =
        static_cast<agent::boolean>(camera.isOpened());
    if (cameraOpen) {
      camera.release();
    }
    recordingValue = false;
    pausedValue = false;
  } catch (...) {
    workerFailed.store(true);
  }
}

XWalkVideoRecordingCallbacks
XWalkVideoRecordingOpenCv::callbacks() const noexcept {
  return {&startCamera,       &stopCamera,    &beginRecording, &pauseRecording,
          &continueRecording, &stopRecording, nullptr,         nullptr,
          &timestamp};
}

} /* namespace xwalk::agent */
