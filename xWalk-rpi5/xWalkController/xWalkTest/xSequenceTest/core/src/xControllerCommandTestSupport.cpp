/******************************************************************************
 * @file        xControllerCommandTestSupport.cpp
 * @brief       Implements the shared in-memory CLI-to-HAL host composition.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"

#include "xAgent_Rpi5CarGptCar.h"
#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarGrayscaleModule.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarLanguageModel.h"
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarMotor.h"
#include "xHal_Rpi5CarMusic.h"
#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarServo.h"
#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSpi.h"
#include "xHal_Rpi5CarTextToSpeech.h"
#include "xHal_Rpi5CarUltrasonic.h"
#include "xHal_Rpi5CarVoiceAssistant.h"

#include "xHal_Rpi5CarTrace.h"
namespace {

using xwalk::agent::test::ControllerCommandTestState;

struct TestBus {
  ::ctrl::bytevector sample{0x03U, 0xE8U};
  xwalk::agent::test::ControllerCommandTestState *state{nullptr};
};

struct TestGpio {
  ::ctrl::boolean value{};
};

struct CallbackContext {
  xwalk::agent::test::ControllerCommandTestState *state{nullptr};
  xwalk::hal::XWalkMotors *motors{nullptr};
  xwalk::agent::XWalkPicarx *picarx{nullptr};
};

::ctrl::boolean probe(::ctrl::contextpointer context, ::ctrl::uint8 address) {
  static_cast<void>(context);
  static_cast<void>(address);
  return true;
}

void writeRegister(::ctrl::contextpointer context, ::ctrl::uint8 address,
                   ::ctrl::uint8 reg, const ::ctrl::bytevector &data) {
  static_cast<void>(address);
  static_cast<void>(reg);
  static_cast<void>(data);
  ControllerCommandTestState &state = *static_cast<TestBus *>(context)->state;
  ++state.i2cWriteCount;
  state.eventLog.emplace_back("hal.i2c.write");
}

::ctrl::boolean tryWriteRegister(::ctrl::contextpointer context,
                                 ::ctrl::uint8 address, ::ctrl::uint8 reg,
                                 const ::ctrl::bytevector &data) noexcept {
  writeRegister(context, address, reg, data);
  return true;
}

::ctrl::bytevector readI2c(::ctrl::contextpointer context,
                           ::ctrl::uint8 address, ::ctrl::size length) {
  static_cast<void>(address);
  static_cast<void>(length);
  TestBus &bus = *static_cast<TestBus *>(context);
  bus.state->eventLog.emplace_back("hal.i2c.read");
  return bus.sample;
}

void configureGpio(::ctrl::contextpointer context, ::ctrl::uint8 pin,
                   XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull,
                   ::ctrl::boolean initialValue) {
  static_cast<void>(pin);
  static_cast<void>(mode);
  static_cast<void>(pull);
  static_cast<TestGpio *>(context)->value = initialValue;
}

::ctrl::boolean readGpio(::ctrl::contextpointer context, ::ctrl::uint8 pin) {
  static_cast<void>(pin);
  return static_cast<TestGpio *>(context)->value;
}

void writeGpio(::ctrl::contextpointer context, ::ctrl::uint8 pin,
               ::ctrl::boolean value) {
  static_cast<void>(pin);
  static_cast<TestGpio *>(context)->value = value;
}

void interruptGpio(::ctrl::contextpointer context, ::ctrl::uint8 pin,
                   XWalkHal::XWalkGpioEdge edge, ::ctrl::uint32 debounceMs,
                   ::ctrl::contextpointer handlerContext,
                   XWalkHal::gpiointerrupthandler handler) {
  static_cast<void>(context);
  static_cast<void>(pin);
  static_cast<void>(edge);
  static_cast<void>(debounceMs);
  static_cast<void>(handlerContext);
  static_cast<void>(handler);
}

void cancelInterrupt(::ctrl::contextpointer context, ::ctrl::uint8 pin) {
  static_cast<void>(context);
  static_cast<void>(pin);
}

XWalkHal::XWalkGpioCallbacks gpioCallbacks() {
  return {&configureGpio, &readGpio, &writeGpio, &interruptGpio,
          &cancelInterrupt};
}

void output(::ctrl::contextpointer context, ::ctrl::stringview line) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("controller.output");
  state.outputLines.emplace_back(line);
}

::ctrl::string input(::ctrl::contextpointer context,
                     ::ctrl::stringview prompt) {
  static_cast<void>(prompt);
  xwalk::agent::test::ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("controller.input");
  const ::ctrl::boolean inputUnavailable =
      static_cast<::ctrl::boolean>(state.inputIndex >= state.inputLines.size());
  if (inputUnavailable) {
    return "skip";
  }
  const ::ctrl::string result = state.inputLines[state.inputIndex];
  ++state.inputIndex;
  return result;
}

void delay(::ctrl::contextpointer context, ::ctrl::uint32 durationMs) {
  CallbackContext &callbackContext = *static_cast<CallbackContext *>(context);
  callbackContext.state->eventLog.emplace_back("controller.delay");
  callbackContext.state->delays.push_back(durationMs);
  callbackContext.state->leftSpeeds.push_back(
      callbackContext.motors->left().speed());
  callbackContext.state->rightSpeeds.push_back(
      callbackContext.motors->right().speed());
  callbackContext.state->steeringAngles.push_back(
      callbackContext.picarx->directionAngleDegrees());
}

::ctrl::boolean startVision(::ctrl::contextpointer context) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("vision.start");
  state.visionStarted = true;
  return true;
}

void stopVision(::ctrl::contextpointer context) noexcept {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("vision.stop");
  state.visionStarted = false;
  state.visionColor = xwalk::agent::XWalkComputerVisionColor::Close;
  state.visionFaceEnabled = false;
  state.visionQrEnabled = false;
}

::ctrl::string captureVision(::ctrl::contextpointer context) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("vision.capture");
  ++state.visionCaptureCount;
  return "/tmp/photo_2026-08-04-12-00-00.jpg";
}

::ctrl::boolean startAppControl(::ctrl::contextpointer context,
                                ::ctrl::stringview name,
                                ::ctrl::stringview type, ::ctrl::uint16 port) {
  static_cast<void>(name);
  static_cast<void>(type);
  static_cast<void>(port);
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("app.start");
  state.appTransportStarted = true;
  return true;
}

void stopAppControl(::ctrl::contextpointer context) noexcept {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("app.stop");
  state.appTransportStarted = false;
}

xwalk::agent::XWalkAppControlInput
pollAppControl(::ctrl::contextpointer context) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("app.poll");
  const ::ctrl::boolean appInputUnavailable = static_cast<::ctrl::boolean>(
      state.appInputIndex >= state.appInputs.size());
  if (appInputUnavailable) {
    return {};
  }
  const xwalk::agent::XWalkAppControlInput result =
      state.appInputs[state.appInputIndex];
  ++state.appInputIndex;
  return result;
}

void publishAppControl(
    ::ctrl::contextpointer context,
    const xwalk::agent::XWalkAppControlTelemetry &telemetry) {
  static_cast<void>(telemetry);
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("app.publish");
  ++state.appPublishCount;
}

void setVisionColor(::ctrl::contextpointer context,
                    xwalk::agent::XWalkComputerVisionColor color) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("vision.color");
  state.visionColor = color;
}

void setVisionFace(::ctrl::contextpointer context, ::ctrl::boolean enabled) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("vision.face");
  state.visionFaceEnabled = enabled;
}

void setVisionQr(::ctrl::contextpointer context, ::ctrl::boolean enabled) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("vision.qr");
  state.visionQrEnabled = enabled;
}

xwalk::agent::XWalkComputerVisionObservation
observeVision(::ctrl::contextpointer context) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("vision.observe");
  ++state.visionObservationCount;
  xwalk::agent::XWalkComputerVisionObservation observation;
  if (state.visionColorVisible &&
      (state.visionColor != xwalk::agent::XWalkComputerVisionColor::Close)) {
    ::ctrl::uint32 widthPixels{40U};
    const ::ctrl::size widthIndex = state.visionObservationCount - 1U;
    const ::ctrl::boolean colorWidthAvailable = static_cast<::ctrl::boolean>(
        widthIndex < state.visionColorWidths.size());
    if (colorWidthAvailable) {
      widthPixels = state.visionColorWidths[widthIndex];
    }
    observation.color = {1U, 120, 80, widthPixels, 30U};
  }
  if (state.visionFaceEnabled) {
    observation.face = {1U, 300, 200, 100U, 120U};
  }
  if (state.visionQrEnabled) {
    observation.qrData = "xwalk-qr";
  }
  return observation;
}

xwalk::agent::XWalkComputerVisionColor
selectTreasureColor(::ctrl::contextpointer context) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  ::ctrl::string colorName{"red"};
  const ::ctrl::boolean treasureColorAvailable = static_cast<::ctrl::boolean>(
      state.treasureColorIndex < state.treasureColorNames.size());
  if (treasureColorAvailable) {
    colorName = state.treasureColorNames[state.treasureColorIndex];
    ++state.treasureColorIndex;
  }
  if (colorName == "orange") {
    return xwalk::agent::XWalkComputerVisionColor::Orange;
  }
  if (colorName == "yellow") {
    return xwalk::agent::XWalkComputerVisionColor::Yellow;
  }
  if (colorName == "green") {
    return xwalk::agent::XWalkComputerVisionColor::Green;
  }
  if (colorName == "blue") {
    return xwalk::agent::XWalkComputerVisionColor::Blue;
  }
  if (colorName == "purple") {
    return xwalk::agent::XWalkComputerVisionColor::Purple;
  }
  return xwalk::agent::XWalkComputerVisionColor::Red;
}

::ctrl::string beginVideo(::ctrl::contextpointer context,
                          ::ctrl::stringview name) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("video.begin");
  state.videoRecording = true;
  return "/tmp/xwalk-videos/" + ::ctrl::string(name) + ".avi";
}

void pauseVideo(::ctrl::contextpointer context) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("video.pause");
  state.videoPaused = true;
}

void continueVideo(::ctrl::contextpointer context) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("video.continue");
  state.videoPaused = false;
}

void stopVideo(::ctrl::contextpointer context) noexcept {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("video.stop");
  state.videoRecording = false;
  state.videoPaused = false;
}

::ctrl::string videoTimestamp(::ctrl::contextpointer context) {
  static_cast<void>(context);
  return "2026-08-05-12.30.45";
}

::ctrl::boolean selfDriveDelay(::ctrl::contextpointer context,
                               ::ctrl::uint32 durationMs) noexcept {
  delay(context, durationMs);
  return true;
}

/**
 * @brief Yields GPT action-worker polling without mutating shared test state.
 * @param[in,out] context Unused nullable test context.
 * @param[in] durationMs Unused requested delay in milliseconds.
 * @return Always `true` after a one-millisecond host delay.
 */
::ctrl::boolean selfDriveQuietDelay(::ctrl::contextpointer context,
                                    ::ctrl::uint32 durationMs) noexcept {
  static_cast<void>(context);
  static_cast<void>(durationMs);
  XWalkHal::common::sleepMilliseconds(1U);
  return true;
}

::ctrl::boolean continueOperation(::ctrl::contextpointer context) {
  xwalk::agent::test::ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("controller.continue");
  const ::ctrl::boolean result =
      state.operationQueries < state.operationQueryLimit;
  ++state.operationQueries;
  return result;
}

void setServoZeroingAngle(::ctrl::contextpointer context, ::ctrl::uint8 servoId,
                          ::ctrl::float64 angleDegrees) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("servo-zeroing.angle");
  state.servoZeroingIds.push_back(static_cast<::ctrl::uint32>(servoId));
  state.servoZeroingAngles.push_back(angleDegrees);
}

::ctrl::boolean sound(::ctrl::contextpointer context,
                      const xwalk::ctrl::XWalkSoundRequest &request) {
  xwalk::agent::test::ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.sound");
  state.soundOperation = request.operation;
  state.soundFile = request.filePath;
  state.soundVolume = request.volumePercent;
  return true;
}

::ctrl::bytevector transferSpi(::ctrl::contextpointer context,
                               const ::ctrl::bytevector &transmitData) {
  static_cast<CallbackContext *>(context)->state->eventLog.emplace_back(
      "hal.spi.transfer");
  ::ctrl::bytevector response;
  response.reserve(transmitData.size());
  for (const ::ctrl::uint8 value : transmitData) {
    response.push_back(static_cast<::ctrl::uint8>(value ^ 0xFFU));
  }
  return response;
}

void enableMusic(::ctrl::contextpointer context) { static_cast<void>(context); }

void playSound(::ctrl::contextpointer context, ::ctrl::stringview filename,
               ::ctrl::optionalfloat64 volume) {
  static_cast<void>(volume);
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.music.sound");
  state.musicSoundFile = filename;
}

void playMusic(::ctrl::contextpointer context, ::ctrl::stringview filename,
               ::ctrl::int32 loops, ::ctrl::float64 startSeconds) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.music.play");
  state.backgroundMusicFile = filename;
  static_cast<void>(loops);
  static_cast<void>(startSeconds);
}

void setVolume(::ctrl::contextpointer context, ::ctrl::float64 volume) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.music.volume");
  state.soundVolume = volume;
}

void musicControl(::ctrl::contextpointer context) {
  static_cast<CallbackContext *>(context)->state->eventLog.emplace_back(
      "hal.music.control");
}

::ctrl::float64 soundLength(::ctrl::contextpointer context,
                            ::ctrl::stringview filename) {
  static_cast<void>(context);
  static_cast<void>(filename);
  return 1.0;
}

void playTone(::ctrl::contextpointer context, const ::ctrl::bytevector &pcmData,
              ::ctrl::uint32 sampleRateHz, ::ctrl::uint8 channelCount) {
  static_cast<void>(context);
  static_cast<void>(pcmData);
  static_cast<void>(sampleRateHz);
  static_cast<void>(channelCount);
}

void primeSpeaker(::ctrl::contextpointer context, ::ctrl::uint32 durationMs) {
  static_cast<void>(durationMs);
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.speaker.prime");
  ++state.speakerPrimeCount;
}

::ctrl::boolean recognitionReady(::ctrl::contextpointer context) {
  static_cast<void>(context);
  return true;
}

::ctrl::string recognizeSpeech(::ctrl::contextpointer context,
                               ::ctrl::uint32 timeoutMs) {
  static_cast<void>(timeoutMs);
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.speech.listen");
  const ::ctrl::boolean recognitionTranscriptUnavailable =
      static_cast<::ctrl::boolean>(state.recognitionTranscriptIndex >=
                                   state.recognitionTranscripts.size());
  if (recognitionTranscriptUnavailable) {
    return {};
  }
  const ::ctrl::string transcript =
      state.recognitionTranscripts[state.recognitionTranscriptIndex];
  ++state.recognitionTranscriptIndex;
  return transcript;
}

::ctrl::string transcribeFile(::ctrl::contextpointer context,
                              ::ctrl::stringview filePath) {
  static_cast<void>(context);
  static_cast<void>(filePath);
  return {};
}

void stopRecognition(::ctrl::contextpointer context) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.speech.stop");
  ++state.recognitionStopCount;
}

void setModelText(::ctrl::contextpointer context, ::ctrl::stringview text) {
  static_cast<void>(context);
  static_cast<void>(text);
  static_cast<CallbackContext *>(context)->state->eventLog.emplace_back(
      "hal.model.configure");
}

void setMaximumMessages(::ctrl::contextpointer context,
                        ::ctrl::uint32 maximumMessages) {
  static_cast<void>(context);
  static_cast<void>(maximumMessages);
}

void addMessage(::ctrl::contextpointer context,
                XWalkHal::XWalkLanguageModelRole role,
                ::ctrl::stringview content, ::ctrl::stringview imagePath) {
  static_cast<void>(context);
  static_cast<void>(role);
  static_cast<void>(content);
  static_cast<void>(imagePath);
  static_cast<CallbackContext *>(context)->state->eventLog.emplace_back(
      "hal.model.prompt");
}

::ctrl::string promptModel(::ctrl::contextpointer context,
                           ::ctrl::stringview prompt,
                           ::ctrl::stringview imagePath) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.model.prompt");
  state.modelPrompts.emplace_back(prompt);
  state.modelImagePaths.emplace_back(imagePath);
  const ::ctrl::boolean modelResponseAvailable = static_cast<::ctrl::boolean>(
      state.modelResponseIndex < state.modelResponses.size());
  if (modelResponseAvailable) {
    const ::ctrl::string response =
        state.modelResponses[state.modelResponseIndex];
    ++state.modelResponseIndex;
    return response;
  }
  return "Ready.\nACTIONS: stop";
}

::ctrl::boolean captureTextVisionImage(
    ::ctrl::contextpointer context, ::ctrl::stringview outputPath,
    const XWalkHal::XWalkCameraConfiguration &configuration) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.camera.capture");
  state.cameraCapturePaths.emplace_back(outputPath);
  state.cameraWidthPixels = configuration.widthPixels;
  state.cameraHeightPixels = configuration.heightPixels;
  return true;
}

::ctrl::string captureVoiceActiveCarImage(::ctrl::contextpointer context) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  const ::ctrl::filesystempath configuredImage =
      ::ctrl::filesystempath(XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY) /
      "voice-active-car.jpg";
  const ::ctrl::string imageContents =
      xwalk::hal::readFileContents(configuredImage);
  const ::ctrl::boolean imageContentsUnsignedCharInvalid =
      static_cast<::ctrl::boolean>(
          (imageContents.size() < 3U) ||
          (static_cast<unsigned char>(imageContents[0U]) != 0xFFU) ||
          (static_cast<unsigned char>(imageContents[1U]) != 0xD8U) ||
          (static_cast<unsigned char>(imageContents[2U]) != 0xFFU));
  if (imageContentsUnsignedCharInvalid) {
    XWALK_CTRL_ERROR(XWALK_RUNTIME,
                     "Configured voice-active-car image is not a JPEG file");
  }
  state.eventLog.emplace_back("hal.camera.capture");
  state.cameraCapturePaths.emplace_back(configuredImage.string());
  return state.cameraCapturePaths.back();
}

void speak(::ctrl::contextpointer context, ::ctrl::stringview text) {
  ControllerCommandTestState &state =
      *static_cast<CallbackContext *>(context)->state;
  state.eventLog.emplace_back("hal.speech.speak");
  state.spokenText.emplace_back(text);
}

} /* namespace */

namespace xwalk::agent::test {

::ctrl::int32
runControllerCommandHostTest(::ctrl::int32 argumentCount,
                             ::ctrl::charpointer argumentValues[],
                             controllercommandtestcallback callback) {
  if ((argumentCount != 2) || (argumentValues == nullptr) ||
      (argumentValues[1U] == nullptr) || (callback == nullptr)) {
    return 1;
  }

  const ::ctrl::filesystempath configPath(argumentValues[1U]);
  ::ctrl::filesystempath replacementPath = configPath;
  replacementPath += ".tmp";
  static_cast<void>(hal::removeFilesystemEntry(configPath));
  static_cast<void>(hal::removeFilesystemEntry(replacementPath));

  ControllerCommandTestState state;
  TestBus bus{{0x03U, 0xE8U}, &state};
  hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &readI2c, nullptr,
                    &tryWriteRegister);
  hal::XWalkPwmTimerState timerState;
  hal::XWalkPwm leftPwm(i2c, "P13", 0x14U, timerState);
  hal::XWalkPwm rightPwm(i2c, "P12", 0x14U, timerState);
  hal::XWalkPwm directionPwm(i2c, "P2", 0x14U, timerState);
  hal::XWalkPwm panPwm(i2c, "P0", 0x14U, timerState);
  hal::XWalkPwm tiltPwm(i2c, "P1", 0x14U, timerState);
  TestGpio leftBackend;
  TestGpio rightBackend;
  TestGpio triggerBackend;
  TestGpio echoBackend;
  TestGpio resetBackend;
  TestGpio speakerBackend;
  TestGpio ledBackend;
  const hal::XWalkGpioCallbacks gpioBackend = gpioCallbacks();
  hal::XWalkGpio leftDirection(&leftBackend, gpioBackend, "D4");
  hal::XWalkGpio rightDirection(&rightBackend, gpioBackend, "D5");
  hal::XWalkGpio trigger(&triggerBackend, gpioBackend, "D2");
  hal::XWalkGpio echo(&echoBackend, gpioBackend, "D3");
  hal::XWalkGpio reset(&resetBackend, gpioBackend, "MCURST");
  hal::XWalkGpio speaker(&speakerBackend, gpioBackend,
                         XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
  hal::XWalkGpio ledGpio(&ledBackend, gpioBackend, "D0");
  hal::XWalkMotor leftMotor(leftPwm, leftDirection);
  hal::XWalkMotor rightMotor(rightPwm, rightDirection);
  hal::XWalkMotors motors(leftMotor, rightMotor);
  hal::XWalkServo directionServo(directionPwm);
  hal::XWalkServo panServo(panPwm);
  hal::XWalkServo tiltServo(tiltPwm);
  hal::XWalkAdc adc0(i2c, "A0", 0x14U);
  hal::XWalkAdc adc1(i2c, "A1", 0x14U);
  hal::XWalkAdc adc2(i2c, "A2", 0x14U);
  hal::XWalkAdc adc4(i2c, "A4", 0x14U);
  hal::XWalkGrayscaleModule grayscale(adc0, adc1, adc2);
  hal::XWalkUltrasonic ultrasonic(trigger, echo, 0U);
  hal::XWalkConfigStore configuration(configPath.string());
  configuration.set("picarx_max_motor_output_percent", "100");
  configuration.set("picarx_calibration_verified", "true");
  XWalkPicarx picarx(motors, directionServo, panServo, tiltServo, grayscale,
                     ultrasonic, configuration);
  static_cast<void>(picarx.initialize());

  CallbackContext callbackContext{&state, &motors, &picarx};
  const xwalk::ctrl::XWalkControllerCallbacks controllerCallbacks{
      &output, &input, &delay, &continueOperation, &sound};
  XWalkLineTracking lineTracking(picarx, &callbackContext, &delay);
  const hal::XWalkMusicCallbacks musicCallbacks{
      &enableMusic,  &playSound,    &playSound,    &playMusic,   &setVolume,
      &musicControl, &musicControl, &musicControl, &soundLength, &playTone};
  hal::XWalkMusic music(&callbackContext, musicCallbacks);
  XWalkSelfDrive selfDrive(picarx, music, &callbackContext, &selfDriveDelay,
                           nullptr, XWALK_TEST_SOUND_DIRECTORY);
  XWalkSelfDrive selfDriveGpt(picarx, music, &callbackContext,
                              &selfDriveQuietDelay, nullptr,
                              XWALK_TEST_SOUND_DIRECTORY);
  XWalkSelfDrive selfDriveVoice(picarx, music, &callbackContext,
                                &selfDriveQuietDelay, nullptr,
                                XWALK_TEST_SOUND_DIRECTORY);
  XWalkSelfDrive selfDriveGptCar(picarx, music, &callbackContext,
                                 &selfDriveQuietDelay, nullptr,
                                 XWALK_TEST_SOUND_DIRECTORY);
  XWalkGrayscaleCalibration grayscaleCalibration(picarx, &callbackContext,
                                                 &delay, &continueOperation);
  XWalkServoMotorCalibration servoMotorCalibration(picarx, &callbackContext,
                                                   &delay, &continueOperation);
  XWalkMoveExample moveExample(picarx, &callbackContext, &delay,
                               &continueOperation);
  XWalkKeyboardControl keyboardControl(picarx, &callbackContext, &delay,
                                       &continueOperation);
  XWalkObstacleAvoidance obstacleAvoidance(picarx, &callbackContext, &delay,
                                           &continueOperation);
  XWalkCliffDetection cliffDetection(picarx, &callbackContext, &delay,
                                     &continueOperation);
  xwalk::ctrl::XWalkController controller(
      picarx, grayscaleCalibration, servoMotorCalibration, moveExample,
      keyboardControl, obstacleAvoidance, cliffDetection, &callbackContext,
      controllerCallbacks);
  xwalk::ctrl::XWalkController lineController(
      picarx, lineTracking, &callbackContext, controllerCallbacks);
  xwalk::ctrl::XWalkController selfDriveController(
      picarx, selfDrive, &callbackContext, controllerCallbacks);
  hal::XWalkSpi spi(&callbackContext, &transferSpi);
  XWalkSpiTransfer spiTransfer(spi);
  xwalk::ctrl::XWalkController spiController(spiTransfer, &callbackContext,
                                             controllerCallbacks);
  const ::ctrl::stringvector passingReport{"[PASS] Configuration: ready"};
  const ::ctrl::stringvector failingReport{"[FAIL] I2C: unavailable"};
  xwalk::ctrl::XWalkController doctorController(passingReport, &callbackContext,
                                                controllerCallbacks);
  xwalk::ctrl::XWalkController failingDoctorController(
      failingReport, &callbackContext, controllerCallbacks);
  const XWalkServoZeroingCallbacks servoZeroingCallbacks{
      &setServoZeroingAngle, &delay, &continueOperation};
  XWalkServoZeroing servoZeroing(&callbackContext, servoZeroingCallbacks);
  xwalk::ctrl::XWalkController servoZeroingController(
      servoZeroing, &callbackContext, controllerCallbacks);

  hal::XWalkBoardControl boardControl(reset, speaker, adc4, &callbackContext,
                                      &primeSpeaker);
  const hal::XWalkSpeechToTextCallbacks speechCallbacks{
      &recognitionReady, &recognizeSpeech, &transcribeFile, &stopRecognition};
  hal::XWalkSpeechToText speechToText(&callbackContext, speechCallbacks);
  const hal::XWalkLanguageModelCallbacks modelCallbacks{
      &setModelText, &setModelText, &setMaximumMessages, &addMessage,
      &promptModel};
  hal::XWalkLanguageModel languageModel(&callbackContext, modelCallbacks);
  languageModel.setMaximumMessages(20U);
  hal::XWalkTextToSpeech textToSpeech(boardControl, &callbackContext, &speak);
  const hal::XWalkVoiceAssistantConfiguration chatbotAssistantConfiguration{
      XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_INSTRUCTIONS,
      XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME};
  hal::XWalkVoiceAssistant chatbotAssistant(
      speechToText, languageModel, textToSpeech, chatbotAssistantConfiguration);
  hal::XWalkVoiceAssistant assistant(
      speechToText, languageModel, textToSpeech,
      XWalkVoiceActiveCar::assistantConfiguration());
  const hal::XWalkVoiceAssistantConfiguration gptAssistantConfiguration =
      XWalkVoiceActiveCarGpt::assistantConfiguration();
  hal::XWalkVoiceAssistant gptAssistant(
      speechToText, languageModel, textToSpeech, gptAssistantConfiguration);
  hal::XWalkVoiceAssistant gptCarAssistant(
      speechToText, languageModel, textToSpeech,
      XWalkGptCar::assistantConfiguration());
  hal::XWalkLed led(ledGpio);
  const XWalkLocalVoiceChatbotCallbacks chatbotCallbacks{
      &output, &continueOperation, &delay};
  XWalkLocalVoiceChatbot voiceChat(chatbotAssistant, &callbackContext,
                                   chatbotCallbacks);
  const XWalkVoiceActiveCarCallbacks voiceCallbacks{
      &output, &continueOperation, &delay, &captureVoiceActiveCarImage, &input};
  XWalkVoiceActiveCar voiceActive(picarx, selfDriveVoice, assistant, led,
                                  &callbackContext, voiceCallbacks,
                                  XWalkVoiceActiveCar::carConfiguration());
  XWalkVoiceActiveCar voiceActiveGpt(
      picarx, selfDriveGpt, gptAssistant, led, &callbackContext, voiceCallbacks,
      XWalkVoiceActiveCarGpt::carConfiguration());
  XWalkVoiceActiveCar gptCarCoordinator(
      picarx, selfDriveGptCar, gptCarAssistant, led, &callbackContext,
      voiceCallbacks, XWalkGptCar::carConfiguration());
  XWalkGptCar gptCar(gptCarCoordinator);
  const XWalkVoiceControlledCarCallbacks controlledCallbacks{
      &output, &continueOperation, &delay};
  XWalkVoiceControlledCar voiceControlled(
      picarx, speechToText, &callbackContext, controlledCallbacks);
  const XWalkVoicePromptCarCallbacks promptCallbacks{
      &output, &continueOperation, &delay};
  XWalkVoicePromptCar voicePrompt(picarx, textToSpeech, &callbackContext,
                                  promptCallbacks);
  xwalk::ctrl::XWalkController voiceChatController(
      picarx, voiceChat, &callbackContext, controllerCallbacks);
  xwalk::ctrl::XWalkController voiceActiveController(
      picarx, voiceActive, &callbackContext, controllerCallbacks);
  xwalk::ctrl::XWalkController voiceActiveGptController(
      picarx, voiceActiveGpt, &callbackContext, controllerCallbacks);
  xwalk::ctrl::XWalkController gptCarController(
      picarx, gptCar, &callbackContext, controllerCallbacks);
  xwalk::ctrl::XWalkController voiceControlledController(
      picarx, voiceControlled, &callbackContext, controllerCallbacks);
  xwalk::ctrl::XWalkController voicePromptController(
      picarx, voicePrompt, &callbackContext, controllerCallbacks);
  const XWalkStorytellingRobotCallbacks storyCallbacks{&delay,
                                                       &continueOperation};
  XWalkStorytellingRobot storytellingRobot(picarx, textToSpeech,
                                           &callbackContext, storyCallbacks);
  xwalk::ctrl::XWalkController storytellingController(
      picarx, storytellingRobot, &callbackContext, controllerCallbacks);
  hal::XWalkCameraConfiguration textVisionCameraConfiguration;
  textVisionCameraConfiguration.widthPixels = 1'280U;
  textVisionCameraConfiguration.heightPixels = 720U;
  hal::XWalkCamera textVisionCamera(&callbackContext, &captureTextVisionImage,
                                    textVisionCameraConfiguration);
  XWalkCameraCapture textVisionCapture(textVisionCamera, "/tmp/llm-img.jpg");
  const XWalkTextVisionTalkCallbacks textVisionCallbacks{
      &output, &input, &delay, &continueOperation};
  XWalkTextVisionTalk textVisionTalk(languageModel, textVisionCapture,
                                     &callbackContext, textVisionCallbacks);
  xwalk::ctrl::XWalkController textVisionTalkController(
      textVisionTalk, &callbackContext, controllerCallbacks);
  const XWalkOnlineLlmTestCallbacks onlineCallbacks{&output, &input,
                                                    &continueOperation};
  XWalkOnlineLlmTest onlineLlmTest(languageModel, &callbackContext,
                                   onlineCallbacks);
  xwalk::ctrl::XWalkController onlineLlmTestController(
      onlineLlmTest, &callbackContext, controllerCallbacks);
  const XWalkComputerVisionCallbacks visionCallbacks{
      &startVision,    &stopVision,    &captureVision,
      &setVisionColor, &setVisionFace, &setVisionQr,
      &observeVision,  &delay,         &continueOperation};
  XWalkComputerVision computerVision(&callbackContext, visionCallbacks);
  xwalk::ctrl::XWalkController computerVisionController(
      computerVision, &callbackContext, controllerCallbacks);
  XWalkFaceTracking faceTracking(picarx, &callbackContext, visionCallbacks);
  xwalk::ctrl::XWalkController faceTrackingController(
      picarx, faceTracking, &callbackContext, controllerCallbacks);
  XWalkBullFight bullFight(picarx, &callbackContext, visionCallbacks);
  xwalk::ctrl::XWalkController bullFightController(
      picarx, bullFight, &callbackContext, controllerCallbacks);
  const XWalkTreasureHuntCallbacks treasureCallbacks{visionCallbacks,
                                                     &selectTreasureColor};
  XWalkTreasureHunt treasureHunt(picarx, textToSpeech, &callbackContext,
                                 treasureCallbacks);
  xwalk::ctrl::XWalkController treasureHuntController(
      picarx, treasureHunt, &callbackContext, controllerCallbacks);
  const XWalkVideoRecordingCallbacks videoCallbacks{
      &startVision, &stopVision,        &beginVideo,
      &pauseVideo,  &continueVideo,     &stopVideo,
      &delay,       &continueOperation, &videoTimestamp};
  XWalkVideoRecording videoRecording(&callbackContext, videoCallbacks);
  xwalk::ctrl::XWalkController videoRecordingController(
      videoRecording, &callbackContext, controllerCallbacks);
  XWalkVideoCar videoCar(picarx, &callbackContext, visionCallbacks);
  xwalk::ctrl::XWalkController videoCarController(
      picarx, videoCar, &callbackContext, controllerCallbacks);
  XWalkAppControlCallbacks appCallbacks{
      &callbackContext,   &startAppControl, &stopAppControl, &pollAppControl,
      &publishAppControl, &callbackContext, visionCallbacks};
  XWalkAppControlConfiguration appConfiguration;
  appConfiguration.maximumLineRecoverySamples = 1U;
  appConfiguration.sampleDelayMs = 1U;
  XWalkAppControl appControl(picarx, appCallbacks, appConfiguration);
  xwalk::ctrl::XWalkController appControlController(
      picarx, appControl, &callbackContext, controllerCallbacks);
  XWalkSoundBackgroundMusic soundBackgroundMusic(
      music, &callbackContext, &delay, &continueOperation,
      XWALK_TEST_SOUND_DIRECTORY, XWALK_TEST_MUSIC_DIRECTORY);
  xwalk::ctrl::XWalkController soundBackgroundMusicController(
      soundBackgroundMusic, &callbackContext, controllerCallbacks);

  ControllerCommandTestContext testContext{&state,
                                           &controller,
                                           &lineController,
                                           &selfDriveController,
                                           &spiController,
                                           &doctorController,
                                           &servoZeroingController,
                                           &failingDoctorController,
                                           &voiceChatController,
                                           &voiceActiveController,
                                           &voiceActiveGptController,
                                           &gptCarController,
                                           &voiceControlledController,
                                           &voicePromptController,
                                           &storytellingController,
                                           &textVisionTalkController,
                                           &onlineLlmTestController,
                                           &computerVisionController,
                                           &faceTrackingController,
                                           &bullFightController,
                                           &treasureHuntController,
                                           &videoRecordingController,
                                           &videoCarController,
                                           &appControlController,
                                           &soundBackgroundMusicController,
                                           &motors,
                                           &picarx,
                                           &configuration};
  state.eventLog.clear();
  callback(testContext);

  static_cast<void>(hal::removeFilesystemEntry(configPath));
  static_cast<void>(hal::removeFilesystemEntry(replacementPath));
  return 0;
}

::ctrl::boolean containsOrderedEvents(const ::ctrl::stringvector &events,
                                      const ::ctrl::stringvector &required) {
  ::ctrl::size requiredIndex{};
  for (const ::ctrl::string &event : events) {
    const ::ctrl::boolean requiredEventMatched =
        static_cast<::ctrl::boolean>((requiredIndex < required.size()) &&
                                     (event == required[requiredIndex]));
    if (requiredEventMatched) {
      ++requiredIndex;
    }
  }
  return requiredIndex == required.size();
}

} /* namespace xwalk::agent::test */
