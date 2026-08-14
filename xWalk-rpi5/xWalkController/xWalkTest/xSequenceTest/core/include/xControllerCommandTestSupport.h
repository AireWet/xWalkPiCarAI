/******************************************************************************
 * @file        xControllerCommandTestSupport.h
 * @brief       Declares the shared host composition for CLI command sequences.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XCONTROLLER_COMMAND_TEST_SUPPORT_H
#define XCONTROLLER_COMMAND_TEST_SUPPORT_H

#include "xController.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarMotors.h"

#include <vector>

namespace xwalk::agent::test
{

/** @brief Records observable controller, Agent, and HAL activity. */
struct ControllerCommandTestState
{
    ::ctrl::stringvector outputLines{}; /**< Controller output in call order. */
    ::ctrl::stringvector inputLines{}; /**< Calibration input in read order. */
    ::ctrl::size inputIndex{}; /**< Next unread calibration input. */
    ::ctrl::uint32vector delays{}; /**< Requested delays in call order. */
    ::ctrl::float64vector leftSpeeds{}; /**< Left motor speed at every delay. */
    ::ctrl::float64vector rightSpeeds{}; /**< Right motor speed at every delay. */
    ::ctrl::float64vector steeringAngles{}; /**< Steering angle at every delay. */
    ::ctrl::uint32 i2cWriteCount{}; /**< Writes reaching the simulated I2C HAL. */
    ::ctrl::uint32 operationQueries{}; /**< Cancellation callback query count. */
    ::ctrl::uint32 operationQueryLimit{1'000'000U}; /**< Allowed foreground queries. */
    ::ctrl::uint32vector servoZeroingIds{}; /**< Servo-zeroing channels in call order. */
    ::ctrl::float64vector servoZeroingAngles{}; /**< Servo-zeroing angles in call order. */
    /** @brief Last sound action. */
    xwalk::ctrl::XWalkSoundOperation soundOperation{
        xwalk::ctrl::XWalkSoundOperation::Stop};
    ::ctrl::string soundFile{}; /**< Last sound path. */
    ::ctrl::optionalfloat64 soundVolume{}; /**< Last sound volume. */
    ::ctrl::string musicSoundFile{}; /**< Last self-drive sound path. */
    ::ctrl::string backgroundMusicFile{}; /**< Last streamed-music path. */
    ::ctrl::stringvector spokenText{}; /**< Text reaching the simulated speech HAL. */
    ::ctrl::stringvector recognitionTranscripts{}; /**< Scripted recognition results. */
    ::ctrl::size recognitionTranscriptIndex{}; /**< Next scripted recognition result. */
    ::ctrl::uint32 recognitionStopCount{}; /**< Speech-recognition stop calls. */
    ::ctrl::uint32 speakerPrimeCount{}; /**< Board speaker-prime calls. */
    ::ctrl::stringvector modelPrompts{}; /**< Text reaching the simulated model HAL. */
    ::ctrl::stringvector modelImagePaths{}; /**< Image paths reaching the model HAL. */
    ::ctrl::stringvector modelResponses{}; /**< Scripted model responses. */
    ::ctrl::size modelResponseIndex{}; /**< Next scripted model response. */
    ::ctrl::stringvector cameraCapturePaths{}; /**< Simulated still-image destinations. */
    ::ctrl::uint32 cameraWidthPixels{}; /**< Last simulated still-image width. */
    ::ctrl::uint32 cameraHeightPixels{}; /**< Last simulated still-image height. */
    XWalkComputerVisionColor visionColor{XWalkComputerVisionColor::Close};
    ::ctrl::boolean visionColorVisible{true}; /**< Whether color observation returns a target. */
    ::ctrl::boolean visionStarted{}; /**< True while the simulated camera is active. */
    ::ctrl::boolean visionFaceEnabled{}; /**< Simulated face-detector state. */
    ::ctrl::boolean visionQrEnabled{}; /**< Simulated QR-detector state. */
    ::ctrl::uint32 visionCaptureCount{}; /**< Simulated photograph count. */
    ::ctrl::uint32 visionObservationCount{}; /**< Simulated frame-observation count. */
    ::ctrl::uint32vector visionColorWidths{}; /**< Scripted detected-color widths. */
    ::ctrl::stringvector treasureColorNames{}; /**< Scripted treasure target names. */
    ::ctrl::size treasureColorIndex{}; /**< Next scripted treasure target. */
    ::ctrl::boolean videoRecording{}; /**< Simulated AVI writer state. */
    ::ctrl::boolean videoPaused{}; /**< Simulated recording pause state. */
    std::vector<XWalkAppControlInput> appInputs{}; /**< Simulated A-Q snapshots. */
    ::ctrl::size appInputIndex{}; /**< Next simulated app snapshot. */
    ::ctrl::boolean appTransportStarted{}; /**< Simulated WebSocket lifecycle. */
    ::ctrl::uint32 appPublishCount{}; /**< Published telemetry count. */
    ::ctrl::stringvector eventLog{}; /**< Ordered Controller, Agent, and HAL events. */
};

/** @brief Exposes caller-owned objects while the shared composition is alive. */
struct ControllerCommandTestContext
{
    ControllerCommandTestState* state; /**< Non-owning state pointer. */
    xwalk::ctrl::XWalkController* controller; /**< Non-owning ordinary-command controller. */
    xwalk::ctrl::XWalkController* lineController; /**< Non-owning line-command controller. */
    xwalk::ctrl::XWalkController* selfDriveController; /**< Non-owning action controller. */
    xwalk::ctrl::XWalkController* spiController; /**< Non-owning SPI-command controller. */
    xwalk::ctrl::XWalkController* doctorController; /**< Non-owning passing doctor controller. */
    xwalk::ctrl::XWalkController* servoZeroingController; /**< Non-owning servo-zeroing controller. */
    xwalk::ctrl::XWalkController* failingDoctorController; /**< Non-owning failing doctor controller. */
    xwalk::ctrl::XWalkController* voiceChatController; /**< Non-owning voice-chat controller. */
    xwalk::ctrl::XWalkController* voiceActiveController; /**< Non-owning voice-active controller. */
    xwalk::ctrl::XWalkController* voiceActiveGptController; /**< Non-owning example-21 controller. */
    xwalk::ctrl::XWalkController* gptCarController; /**< Non-owning upstream GPT-car controller. */
    xwalk::ctrl::XWalkController* voiceControlledController; /**< Non-owning voice-control controller. */
    xwalk::ctrl::XWalkController* voicePromptController; /**< Non-owning voice-prompt controller. */
    xwalk::ctrl::XWalkController* storytellingController; /**< Non-owning storytelling controller. */
    xwalk::ctrl::XWalkController* textVisionTalkController; /**< Non-owning text-vision controller. */
    xwalk::ctrl::XWalkController* onlineLlmTestController; /**< Non-owning online-LLM controller. */
    xwalk::ctrl::XWalkController* computerVisionController; /**< Non-owning computer-vision controller. */
    xwalk::ctrl::XWalkController* faceTrackingController; /**< Non-owning face-tracking controller. */
    xwalk::ctrl::XWalkController* bullFightController; /**< Non-owning red-target controller. */
    xwalk::ctrl::XWalkController* treasureHuntController; /**< Non-owning treasure-hunt controller. */
    xwalk::ctrl::XWalkController* videoRecordingController; /**< Non-owning video controller. */
    xwalk::ctrl::XWalkController* videoCarController; /**< Non-owning interactive video-car controller. */
    xwalk::ctrl::XWalkController* appControlController; /**< Non-owning mobile-app controller. */
    /** @brief Non-owning audio-example controller. */
    xwalk::ctrl::XWalkController* soundBackgroundMusicController;
    hal::XWalkMotors* motors; /**< Non-owning motor HAL pointer. */
    XWalkPicarx* picarx; /**< Non-owning vehicle Agent pointer. */
    hal::XWalkConfigStore* configuration; /**< Non-owning configuration HAL pointer. */
};

/** @brief Assertion callback executed inside one complete in-memory composition. */
using controllercommandtestcallback = void (*)(ControllerCommandTestContext& context);

/**
 * @brief Runs one command scenario with a complete controller-to-HAL composition.
 * @param[in] argumentCount Must be two.
 * @param[in] argumentValues Test name and build-local configuration path.
 * @param[in] callback Non-null synchronous assertion callback.
 * @return Zero after the callback completes; one for invalid arguments.
 */
::ctrl::int32 runControllerCommandHostTest(::ctrl::int32 argumentCount,
    ::ctrl::charpointer argumentValues[], controllercommandtestcallback callback);

/**
 * @brief Reports whether required events occur in the supplied order.
 * @param[in] events Complete observed event stream.
 * @param[in] required Ordered event names to locate, allowing intervening events.
 * @return `true` when every required event occurs in order.
 */
::ctrl::boolean containsOrderedEvents(const ::ctrl::stringvector& events,
    const ::ctrl::stringvector& required);

} /* namespace xwalk::agent::test */

#endif /* XCONTROLLER_COMMAND_TEST_SUPPORT_H */
