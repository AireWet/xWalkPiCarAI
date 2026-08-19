/******************************************************************************
 * @file        xControllerLifecycle.cpp
 * @brief       Implements PiCar-X CLI lifecycle and callback forwarding.
 *
 * @details
 * Binds caller-owned dependencies and validates the synchronous platform
 *boundary.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-07-31
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
#include "xController.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs a CLI around one caller-owned PiCar-X coordinator.
     * @param[in] picarx Coordinator that must outlive this CLI.
     * @param[in,out] context Optional platform context that must outlive this CLI
     * when non-null.
     * @param[in] backendCallbacks Complete non-null synchronous callback table.
     */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /**
     * @brief Constructs a CLI containing only twelve-channel servo zeroing.
     * @param[in] servoZeroing Agent that must outlive this CLI.
     * @param[in,out] context Optional platform context that outlives this CLI.
     * @param[in] backendCallbacks Complete non-null synchronous callback table.
     */
    XWalkController::XWalkController(agent::XWalkServoZeroing& servoZeroing,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : servoZeroingObject(&servoZeroing), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
        servoZeroingObject->setCancellation(callbackContext, callbacks.continueOperation);
    }

    /**
     * @brief Constructs a CLI with base calibration and ported example support.
     * @param[in] picarx Coordinator that must outlive this CLI.
     * @param[in] grayscaleCalibration Calibration Agent that must outlive this CLI.
     * @param[in] servoMotorCalibration Servo/motor Agent that must outlive this
     * CLI.
     * @param[in] moveExample Movement-example Agent that must outlive this CLI.
     * @param[in] keyboardControl Keyboard-control Agent that must outlive this CLI.
     * @param[in] obstacleAvoidance Obstacle-avoidance Agent that must outlive this
     * CLI.
     * @param[in] cliffDetection Cliff-detection Agent that must outlive this CLI.
     * @param[in,out] context Optional platform callback context.
     * @param[in] backendCallbacks Complete non-null synchronous callback table.
     */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkGrayscaleCalibration& grayscaleCalibration,
                                     agent::XWalkServoMotorCalibration& servoMotorCalibration,
                                     agent::XWalkMoveExample& moveExample,
                                     agent::XWalkKeyboardControl& keyboardControl,
                                     agent::XWalkObstacleAvoidance& obstacleAvoidance,
                                     agent::XWalkCliffDetection& cliffDetection,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), grayscaleCalibrationObject(&grayscaleCalibration),
          servoMotorCalibrationObject(&servoMotorCalibration), moveExampleObject(&moveExample),
          keyboardControlObject(&keyboardControl), obstacleAvoidanceObject(&obstacleAvoidance),
          cliffDetectionObject(&cliffDetection), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    XWalkController::XWalkController(agent::XWalkVideoRecording& videoRecording,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : videoRecordingObject(&videoRecording), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkVideoCar& videoCar,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), videoCarObject(&videoCar), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkAppControl& appControl,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), appControlObject(&appControl), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    XWalkController::XWalkController(agent::XWalkSoundBackgroundMusic& soundBackgroundMusic,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : soundBackgroundMusicObject(&soundBackgroundMusic), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /**
     * @brief Constructs a CLI with foreground line-tracking command support.
     * @param[in] picarx Coordinator that must outlive this CLI.
     * @param[in] lineTracking Line-tracking coordinator that must outlive this CLI.
     * @param[in,out] context Optional platform context that must outlive this CLI
     * when non-null.
     * @param[in] backendCallbacks Complete non-null synchronous callback table.
     */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkLineTracking& lineTracking,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), lineTrackingObject(&lineTracking), callbackContext(context),
          callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /**
     * @brief Constructs a CLI containing only interactive computer vision.
     * @param[in] computerVision Vision Agent that must outlive this CLI.
     * @param[in,out] context Optional platform callback context.
     * @param[in] backendCallbacks Complete non-null synchronous callback table.
     */
    XWalkController::XWalkController(agent::XWalkComputerVision& computerVision,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : computerVisionObject(&computerVision), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkFaceTracking& faceTracking,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), faceTrackingObject(&faceTracking), callbackContext(context),
          callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkBullFight& bullFight,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), bullFightObject(&bullFight), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /**
     * @brief Constructs a CLI containing the interactive color treasure hunt.
     * @param[in] picarx Coordinator that must outlive this CLI.
     * @param[in] treasureHunt Treasure-hunt Agent that must outlive this CLI.
     * @param[in,out] context Optional platform context that must outlive this CLI.
     * @param[in] backendCallbacks Complete non-null synchronous callback table.
     */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkTreasureHunt& treasureHunt,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), treasureHuntObject(&treasureHunt), callbackContext(context),
          callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /**
     * @brief Constructs a CLI with named self-drive action support.
     * @param[in] picarx Coordinator that must outlive this CLI.
     * @param[in] selfDrive Self-drive coordinator that must outlive this CLI.
     * @param[in,out] context Optional platform context that must outlive this CLI
     * when non-null.
     * @param[in] backendCallbacks Complete non-null synchronous callback table.
     */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkSelfDrive& selfDrive,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), selfDriveObject(&selfDrive), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
        selfDriveObject->setCancellation(callbackContext, callbacks.continueOperation);
    }

    /** @brief Constructs a CLI with local voice-chatbot support. */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkLocalVoiceChatbot& localVoiceChatbot,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), localVoiceChatbotObject(&localVoiceChatbot), callbackContext(context),
          callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /**
     * @brief Constructs a CLI containing only an SPI transfer Agent.
     * @param[in] spiTransfer SPI Agent that must outlive this CLI.
     * @param[in,out] context Optional platform callback context.
     * @param[in] backendCallbacks Complete non-null synchronous callback table.
     */
    XWalkController::XWalkController(agent::XWalkSpiTransfer& spiTransfer,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : spiTransferObject(&spiTransfer), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /**
     * @brief Constructs a CLI containing only a bounded preflight report.
     * @param[in] doctorLines Report lines that must outlive this CLI.
     * @param[in,out] context Optional platform callback context.
     * @param[in] backendCallbacks Complete non-null synchronous callback table.
     */
    XWalkController::XWalkController(const ::ctrl::stringvector& doctorLines,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : doctorLinesObject(&doctorLines), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /** @brief Constructs a CLI with wake-word movement-control support. */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkVoiceControlledCar& voiceControlledCar,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), voiceControlledCarObject(&voiceControlledCar), callbackContext(context),
          callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /** @brief Constructs a CLI with the spoken movement demonstration. */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkVoicePromptCar& voicePromptCar,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), voicePromptCarObject(&voicePromptCar), callbackContext(context),
          callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /** @brief Constructs a CLI with the storytelling movement Agent. */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkStorytellingRobot& storytellingRobot,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), storytellingRobotObject(&storytellingRobot), callbackContext(context),
          callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /** @brief Constructs a CLI with image-grounded text conversation support. */
    XWalkController::XWalkController(agent::XWalkTextVisionTalk& textVisionTalk,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : textVisionTalkObject(&textVisionTalk), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /** @brief Constructs a CLI with online text-only conversation support. */
    XWalkController::XWalkController(agent::XWalkOnlineLlmTest& onlineLlmTest,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : onlineLlmTestObject(&onlineLlmTest), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /** @brief Constructs a CLI with the upstream GPT-car coordinator. */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkGptCar& gptCar,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), gptCarObject(&gptCar), callbackContext(context), callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /** @brief Constructs a CLI with sensor-aware voice-active-car support. */
    XWalkController::XWalkController(agent::XWalkPicarx& picarx,
                                     agent::XWalkVoiceActiveCar& voiceActiveCar,
                                     ::ctrl::contextpointer context,
                                     const XWalkControllerCallbacks& backendCallbacks)
        : picarxObject(&picarx), voiceActiveCarObject(&voiceActiveCar), callbackContext(context),
          callbacks(backendCallbacks)
    {
        validateCallbacks(callbacks);
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /** @brief Destroys the CLI without changing or releasing its dependencies. */
    XWalkController::~XWalkController() = default;

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Validates that every required callback is non-null.
     * @param[in] backendCallbacks Callback table to validate.
     * @throws std::invalid_argument If any callback is null.
     */
    void XWalkController::validateCallbacks(const XWalkControllerCallbacks& backendCallbacks)
    {
        if ((backendCallbacks.output == nullptr) || (backendCallbacks.input == nullptr) ||
            (backendCallbacks.delay == nullptr) || (backendCallbacks.monotonicMilliseconds == nullptr) ||
            (backendCallbacks.continueOperation == nullptr) || (backendCallbacks.sound == nullptr))
        {
            XWALK_CTRL_ERROR(XWALK_INVAL, "PiCar-X CLI callbacks must be complete");
        }
    }

    /** @brief Writes one complete output line through the injected backend. */
    void XWalkController::output(::ctrl::stringview line) const
    {
        callbacks.output(callbackContext, line);
        XWALK_CTRL_TRACE_UID0(CTRL .002, "Controller dispatched one output line");
    }

    /** @brief Requests one response through the injected backend. */
    ::ctrl::string XWalkController::input(::ctrl::stringview prompt) const
    {
        return callbacks.input(callbackContext, prompt);
    }

    /** @brief Delays through the injected backend. */
    void XWalkController::delay(::ctrl::uint32 durationMs) const
    {
        callbacks.delay(callbackContext, durationMs);
    }

} /* namespace xwalk::ctrl */
