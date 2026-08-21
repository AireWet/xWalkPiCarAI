/******************************************************************************
 * @file        xController.h
 * @brief       Declares the PiCar-X command-line interface.
 *
 * @details
 * Parses CLI arguments and coordinates movement, line tracking, preset actions,
 * sensors, sound, and calibration.
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

#ifndef XCONTROLLER_H
#define XCONTROLLER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"
#include "xControllerTypes.h"
#include "xAgent_Rpi5CarCliffDetection.h"
#include "xAgent_Rpi5CarComputerVision.h"
#include "xAgent_Rpi5CarFaceTracking.h"
#include "xAgent_Rpi5CarBullFight.h"
#include "xAgent_Rpi5CarTreasureHunt.h"
#include "xAgent_Rpi5CarVideoRecording.h"
#include "xAgent_Rpi5CarVideoCar.h"
#include "xAgent_Rpi5CarVideoStreaming.h"
#include "xAgent_Rpi5CarAppControl.h"
#include "xAgent_Rpi5CarSoundBackgroundMusic.h"
#include "xAgent_Rpi5CarGrayscaleCalibration.h"
#include "xAgent_Rpi5CarKeyboardControl.h"
#include "xAgent_Rpi5CarLineTracking.h"
#include "xAgent_Rpi5CarLocalVoiceChatbot.h"
#include "xAgent_Rpi5CarMoveExample.h"
#include "xAgent_Rpi5CarObstacleAvoidance.h"
#include "xAgent_Rpi5CarSelfDrive.h"
#include "xAgent_Rpi5CarServoMotorCalibration.h"
#include "xAgent_Rpi5CarServoZeroing.h"
#include "xAgent_Rpi5CarSpiTransfer.h"
#include "xAgent_Rpi5CarVoiceActiveCar.h"
#include "xAgent_Rpi5CarVoiceControlledCar.h"
#include "xAgent_Rpi5CarVoicePromptCar.h"
#include "xAgent_Rpi5CarStorytellingRobot.h"
#include "xAgent_Rpi5CarTextVisionTalk.h"
#include "xAgent_Rpi5CarOnlineLlmTest.h"
#include "xAgent_Rpi5CarGptCar.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkController
     * @brief Executes one bounded PiCar-X command from caller-supplied arguments.
     *
     * @details
     * Stores non-owning pointers to caller-selected Agent coordinators, a non-owning
     * platform context, and a copied callback table. The caller owns every dependency
     * and must serialize command execution when sharing the CLI.
     */
    class XWalkController
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning PiCar-X pointer that is never null after construction. */
            agent::XWalkPicarx* picarxObject{nullptr};
            /** @brief Nullable non-owning line-tracking pointer supplied for line-track commands. */
            agent::XWalkLineTracking* lineTrackingObject{nullptr};
            /** @brief Nullable non-owning automatic grayscale-calibration pointer. */
            agent::XWalkGrayscaleCalibration* grayscaleCalibrationObject{nullptr};
            /** @brief Nullable non-owning servo/motor-calibration pointer. */
            agent::XWalkServoMotorCalibration* servoMotorCalibrationObject{nullptr};
            /** @brief Nullable non-owning twelve-channel servo-zeroing pointer. */
            agent::XWalkServoZeroing* servoZeroingObject{nullptr};
            /** @brief Nullable non-owning bounded movement-example pointer. */
            agent::XWalkMoveExample* moveExampleObject{nullptr};
            /** @brief Nullable non-owning keyboard-control pointer. */
            agent::XWalkKeyboardControl* keyboardControlObject{nullptr};
            /** @brief Nullable non-owning obstacle-avoidance pointer. */
            agent::XWalkObstacleAvoidance* obstacleAvoidanceObject{nullptr};
            /** @brief Nullable non-owning cliff-detection pointer. */
            agent::XWalkCliffDetection* cliffDetectionObject{nullptr};
            /** @brief Nullable non-owning interactive computer-vision pointer. */
            agent::XWalkComputerVision* computerVisionObject{nullptr};
            /** @brief Nullable non-owning face-tracking pointer. */
            agent::XWalkFaceTracking* faceTrackingObject{nullptr};
            /** @brief Nullable non-owning red-target pursuit pointer. */
            agent::XWalkBullFight* bullFightObject{nullptr};
            /** @brief Nullable non-owning color treasure-hunt pointer. */
            agent::XWalkTreasureHunt* treasureHuntObject{nullptr};
            /** @brief Nullable non-owning interactive video-recording pointer. */
            agent::XWalkVideoRecording* videoRecordingObject{nullptr};
            /** @brief Nullable non-owning interactive video-car pointer. */
            agent::XWalkVideoCar* videoCarObject{nullptr};
            /** @brief Nullable non-owning foreground video-streaming pointer. */
            agent::XWalkVideoStreaming* videoStreamingObject{nullptr};
            /** @brief Nullable non-owning mobile-app control pointer. */
            agent::XWalkAppControl* appControlObject{nullptr};
            /** @brief Nullable non-owning sound-background-music pointer. */
            agent::XWalkSoundBackgroundMusic* soundBackgroundMusicObject{nullptr};
            /** @brief Nullable non-owning self-drive pointer supplied for self-drive commands. */
            agent::XWalkSelfDrive* selfDriveObject{nullptr};
            /** @brief Nullable non-owning local voice-chatbot pointer. */
            agent::XWalkLocalVoiceChatbot* localVoiceChatbotObject{nullptr};
            /** @brief Nullable non-owning voice-active-car pointer. */
            agent::XWalkVoiceActiveCar* voiceActiveCarObject{nullptr};
            /** @brief Nullable non-owning wake-word voice-controlled-car pointer. */
            agent::XWalkVoiceControlledCar* voiceControlledCarObject{nullptr};
            /** @brief Nullable non-owning spoken movement-demonstration pointer. */
            agent::XWalkVoicePromptCar* voicePromptCarObject{nullptr};
            /** @brief Nullable non-owning storytelling movement Agent pointer. */
            agent::XWalkStorytellingRobot* storytellingRobotObject{nullptr};
            /** @brief Nullable non-owning image-grounded text conversation pointer. */
            agent::XWalkTextVisionTalk* textVisionTalkObject{nullptr};
            /** @brief Nullable non-owning online text conversation pointer. */
            agent::XWalkOnlineLlmTest* onlineLlmTestObject{nullptr};
            /** @brief Nullable non-owning upstream GPT-car pointer. */
            agent::XWalkGptCar* gptCarObject{nullptr};
            /** @brief Nullable non-owning SPI transfer Agent pointer. */
            agent::XWalkSpiTransfer* spiTransferObject{nullptr};
            /** @brief Nullable non-owning bounded preflight lines supplied for Doctor. */
            const ::ctrl::stringvector* doctorLinesObject{nullptr};
            /** @brief Nullable non-owning context forwarded synchronously to platform callbacks. */
            ::ctrl::contextpointer callbackContext{nullptr};
            /** @brief Complete callback table copied during construction. */
            XWalkControllerCallbacks callbacks{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Validates that every required callback is non-null. */
            static void validateCallbacks(const XWalkControllerCallbacks& backendCallbacks);
            /** @brief Reports whether the active command may continue and stops an attached vehicle otherwise. */
            ::ctrl::boolean operationMayContinue();
            /** @brief Performs a cancellable delay using bounded application-owned slices. */
            ::ctrl::boolean delayWhileOperationRequested(::ctrl::uint32 durationMs);
            /** @brief Delays until the next bounded-movement refresh or the movement deadline. */
            ::ctrl::boolean
            delayUntilNextMoveRefresh(::ctrl::uint64 endMs, ::ctrl::uint64& currentMs, ::ctrl::uint64& nextRefreshMs);
            /** @brief Executes the move command. */
            ::ctrl::int32 XWALK_handlerMove(const XWalkMoveRequest& request);
            /** @brief Runs the bounded movement sequence ported from `2.move.py`. */
            ::ctrl::int32 XWALK_handlerMoveExample();
            /** @brief Runs interactive keyboard control ported from `3.keyboard_control.py`. */
            ::ctrl::int32 XWALK_handlerKeyboardControl(const XWalkNoArgumentRequest& request);
            /** @brief Runs foreground obstacle avoidance ported from `4.avoiding_obstacles.py`. */
            ::ctrl::int32 XWALK_handlerObstacleAvoidance(const XWalkLifecycleRequest& request);
            /** @brief Runs foreground cliff detection ported from `5.cliff_detection.py`. */
            ::ctrl::int32 XWALK_handlerCliffDetection(const XWalkLifecycleRequest& request);
            /** @brief Runs interactive computer vision ported from `7.computer_vision.py`. */
            ::ctrl::int32 XWALK_handlerComputerVision(const XWalkNoArgumentRequest& request);
            /** @brief Runs camera-servo face tracking ported from `8.stare_at_you.py`. */
            ::ctrl::int32 XWALK_handlerFaceTracking(const XWalkLifecycleRequest& request);
            /** @brief Runs red-target pursuit ported from `10.bull_fight.py`. */
            ::ctrl::int32 XWALK_handlerBullFight(const XWalkLifecycleRequest& request);
            /** @brief Runs the interactive color game ported from `20.treasure_hunt.py`. */
            ::ctrl::int32 XWALK_handlerTreasureHunt(const XWalkNoArgumentRequest& request);
            /** @brief Runs interactive recording ported from `9.record_video.py`. */
            ::ctrl::int32 XWALK_handlerVideoRecording(const XWalkNoArgumentRequest& request);
            /** @brief Runs interactive driving ported from `11.video_car.py`. */
            ::ctrl::int32 XWALK_handlerVideoCar(const XWalkNoArgumentRequest& request);
            /** @brief Runs foreground MJPEG video streaming until cancellation. */
            ::ctrl::int32 XWALK_handlerVideoStreaming(const XWalkNoArgumentRequest& request);
            /** @brief Runs mobile-app control ported from `12.app_control.py`. */
            ::ctrl::int32 XWALK_handlerAppControl(const XWalkLifecycleRequest& request);
            /** @brief Runs interactive audio ported from example 13. */
            ::ctrl::int32 XWALK_handlerSoundBackgroundMusic(const XWalkNoArgumentRequest& request);
            /** @brief Executes the turn command. */
            ::ctrl::int32 XWALK_handlerTurn(const XWalkTurnRequest& request);
            /** @brief Executes the camera command. */
            ::ctrl::int32 XWALK_handlerCamera(const XWalkCameraRequest& request);
            /** @brief Executes the sensor command. */
            ::ctrl::int32 XWALK_handlerSensor(const XWalkSensorRequest& request);
            /** @brief Executes foreground line-tracking start or immediate stop. */
            ::ctrl::int32 XWALK_handlerLineTracking(const XWalkLifecycleRequest& request);
            /** @brief Executes one named self-drive preset action. */
            ::ctrl::int32 XWALK_handlerSelfDrive(const XWalkSelfDriveRequest& request);
            /** @brief Executes the sound command. */
            ::ctrl::int32 XWALK_handlerSound(const XWalkSoundRequest& request);
            /** @brief Executes one bounded full-duplex SPI transfer. */
            ::ctrl::int32 XWALK_handlerSpi(const XWalkSpiRequest& request);
            /** @brief Traces one bounded hardware preflight report. */
            ::ctrl::int32 XWALK_handlerDoctor(const XWalkNoArgumentRequest& request);
            /** @brief Runs the all-channel sequence ported from `servo_zeroing.py`. */
            ::ctrl::int32 XWALK_handlerServoZeroing(const XWalkNoArgumentRequest& request);
            /** @brief Executes the foreground local voice-chatbot command. */
            ::ctrl::int32 XWALK_handlerVoiceChat(const XWalkLifecycleRequest& request);
            /** @brief Executes one voice-active-car start or stop command. */
            ::ctrl::int32 XWALK_handlerVoiceActiveCar(const XWalkLifecycleRequest& request);
            /** @brief Executes one wake-word voice-control start or stop command. */
            ::ctrl::int32 XWALK_handlerVoiceControlledCar(const XWalkLifecycleRequest& request);
            /** @brief Executes one spoken movement demonstration command. */
            ::ctrl::int32 XWALK_handlerVoicePromptCar(const XWalkLifecycleRequest& request);
            /** @brief Executes one Piper storytelling demonstration command. */
            ::ctrl::int32 XWALK_handlerStorytellingRobot(const XWalkLifecycleRequest& request);
            /** @brief Executes one image-grounded typed conversation command. */
            ::ctrl::int32 XWALK_handlerTextVisionTalk(const XWalkLifecycleRequest& request);
            /** @brief Executes one online text-only conversation command. */
            ::ctrl::int32 XWALK_handlerOnlineLlmTest(const XWalkLifecycleRequest& request);
            /** @brief Executes the upstream GPT-car loop with optional source flags. */
            ::ctrl::int32 XWALK_handlerGptCar(const XWalkGptCarRequest& request);
            /** @brief Executes interactive servo calibration. */
            ::ctrl::int32 XWALK_handlerCalibration(const XWalkCalibrationRequest& request);
            /** @brief Performs capped raised-wheel motor and steering verification. */
            ::ctrl::boolean XWALK_handlerFirstRunVerification();
            /** @brief Calibrates servos, motor balance, optional directions, and verification. */
            ::ctrl::boolean calibrateServoMotor(::ctrl::boolean configureMotorDirections);
            /** @brief Optionally persists one explicit motor direction. */
            void calibrateMotorDirection(::ctrl::uint8 motorId, ::ctrl::stringview motorName);
            /** @brief Runs, confirms, and persists automatic grayscale calibration. */
            ::ctrl::boolean calibrateGrayscaleReferences();
            /** @brief Calibrates one servo through repeated platform prompts. */
            ::ctrl::boolean calibrateServo(const XWalkServoCalibrationConfig& configuration);
            /** @brief Writes one complete output line through the injected backend. */
            void output(::ctrl::stringview line) const;
            /** @brief Requests one response through the injected backend. */
            ::ctrl::string input(::ctrl::stringview prompt) const;
            /** @brief Delays through the injected backend. */
            void delay(::ctrl::uint32 durationMs) const;

        public:
            /**************************************************************************
             * Public friend functions
             **************************************************************************/

            /**
             * @brief Grants application dispatch access without exposing handlers publicly.
             * @param[in,out] controller Controller whose dependencies remain valid for the call.
             * @param[in] arguments Command arguments excluding the executable name.
             * @return Application command status.
             */
            friend ::ctrl::int32 XWALK_runControllerCommand(XWalkController& controller,
                                                            const ::ctrl::stringvector& arguments);

            /**
             * @brief Grants PiCar-X application routing access to protected handlers.
             * @param[in,out] controller Controller whose dependencies remain valid for the call.
             * @param[in] request Parsed command and validated non-empty arguments.
             * @return Command-specific Controller status.
             */
            friend ::ctrl::int32 XWALK_runPicarxControllerCommand(XWalkController& controller,
                                                                  const XWalkControllerCommandRequest& request);

            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a CLI around one caller-owned PiCar-X coordinator.
             * @param[in] picarx Coordinator that must outlive this CLI.
             * @param[in,out] context Optional platform context that must outlive this CLI when non-null.
             * @param[in] backendCallbacks Complete non-null synchronous callback table.
             */
            XWalkController(agent::XWalkPicarx& picarx,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing only twelve-channel servo zeroing. */
            XWalkController(agent::XWalkServoZeroing& servoZeroing,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /**
             * @brief Constructs a CLI with base calibration and ported example support.
             * @param[in] picarx Coordinator that must outlive this CLI.
             * @param[in] grayscaleCalibration Calibration Agent that must outlive this CLI.
             * @param[in] servoMotorCalibration Servo/motor Agent that must outlive this CLI.
             * @param[in] moveExample Movement-example Agent that must outlive this CLI.
             * @param[in] keyboardControl Keyboard-control Agent that must outlive this CLI.
             * @param[in] obstacleAvoidance Obstacle-avoidance Agent that must outlive this CLI.
             * @param[in] cliffDetection Cliff-detection Agent that must outlive this CLI.
             * @param[in,out] context Optional platform callback context.
             * @param[in] backendCallbacks Complete non-null synchronous callback table.
             */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkGrayscaleCalibration& grayscaleCalibration,
                            agent::XWalkServoMotorCalibration& servoMotorCalibration,
                            agent::XWalkMoveExample& moveExample,
                            agent::XWalkKeyboardControl& keyboardControl,
                            agent::XWalkObstacleAvoidance& obstacleAvoidance,
                            agent::XWalkCliffDetection& cliffDetection,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /**
             * @brief Constructs a CLI with foreground line-tracking command support.
             * @param[in] picarx Coordinator that must outlive this CLI.
             * @param[in] lineTracking Line-tracking coordinator that must outlive this CLI.
             * @param[in,out] context Optional platform context that must outlive this CLI when non-null.
             * @param[in] backendCallbacks Complete non-null synchronous callback table.
             */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkLineTracking& lineTracking,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /**
             * @brief Constructs a CLI containing only interactive computer vision.
             * @param[in] computerVision Vision Agent that must outlive this CLI.
             * @param[in,out] context Optional platform callback context.
             * @param[in] backendCallbacks Complete non-null synchronous callback table.
             */
            XWalkController(agent::XWalkComputerVision& computerVision,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing face tracking and its PiCar-X graph. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkFaceTracking& faceTracking,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing PiCar-X red-target pursuit. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkBullFight& bullFight,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing the interactive color treasure hunt. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkTreasureHunt& treasureHunt,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing only interactive video recording. */
            XWalkController(agent::XWalkVideoRecording& videoRecording,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing interactive video-car control. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkVideoCar& videoCar,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing foreground MJPEG video streaming. */
            XWalkController(agent::XWalkVideoStreaming& videoStreaming,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing mobile-app vehicle control. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkAppControl& appControl,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing interactive example 13 audio. */
            XWalkController(agent::XWalkSoundBackgroundMusic& soundBackgroundMusic,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /**
             * @brief Constructs a CLI with named self-drive action support.
             * @param[in] picarx Coordinator that must outlive this CLI.
             * @param[in] selfDrive Self-drive coordinator that must outlive this CLI.
             * @param[in,out] context Optional platform context that must outlive this CLI when non-null.
             * @param[in] backendCallbacks Complete non-null synchronous callback table.
             */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkSelfDrive& selfDrive,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /**
             * @brief Constructs a CLI with local voice-chatbot support.
             * @param[in] picarx Coordinator that must outlive this CLI.
             * @param[in] localVoiceChatbot Chatbot coordinator that must outlive this CLI.
             * @param[in,out] context Optional platform callback context.
             * @param[in] backendCallbacks Complete non-null synchronous callback table.
             */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkLocalVoiceChatbot& localVoiceChatbot,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI with sensor-aware voice-active-car support. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkVoiceActiveCar& voiceActiveCar,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI with wake-word movement-control support. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkVoiceControlledCar& voiceControlledCar,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI with the spoken movement demonstration. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkVoicePromptCar& voicePromptCar,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI with the storytelling movement Agent. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkStorytellingRobot& storytellingRobot,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing only image-grounded text conversation. */
            XWalkController(agent::XWalkTextVisionTalk& textVisionTalk,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI containing only online text conversation. */
            XWalkController(agent::XWalkOnlineLlmTest& onlineLlmTest,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Constructs a CLI with the upstream GPT-car coordinator. */
            XWalkController(agent::XWalkPicarx& picarx,
                            agent::XWalkGptCar& gptCar,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /**
             * @brief Constructs a CLI containing only an SPI transfer Agent.
             * @param[in] spiTransfer SPI Agent that must outlive this CLI.
             * @param[in,out] context Optional platform callback context.
             * @param[in] backendCallbacks Complete non-null synchronous callback table.
             */
            XWalkController(agent::XWalkSpiTransfer& spiTransfer,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /**
             * @brief Constructs a CLI containing only a bounded preflight report.
             * @param[in] doctorLines Report lines that must outlive this CLI.
             * @param[in,out] context Optional platform callback context.
             * @param[in] backendCallbacks Complete non-null synchronous callback table.
             */
            XWalkController(const ::ctrl::stringvector& doctorLines,
                            ::ctrl::contextpointer context,
                            const XWalkControllerCallbacks& backendCallbacks);

            /** @brief Destroys the CLI without changing or releasing its dependencies. */
            ~XWalkController();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            XWalkController(XWalkController&&) = delete;
            XWalkController(const XWalkController&) = delete;
            XWalkController& operator=(XWalkController&&) = delete;
            XWalkController& operator=(const XWalkController&) = delete;
    };

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_H */
