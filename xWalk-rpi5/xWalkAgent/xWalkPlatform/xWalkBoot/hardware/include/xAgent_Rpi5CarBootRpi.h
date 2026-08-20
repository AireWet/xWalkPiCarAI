/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpi.h
 * @brief       Declares the Raspberry Pi xWalk hardware composition owner.
 *
 * @details
 * Selects one bounded hardware graph and retains it for one synchronous
 * application callback before deterministic reverse-order destruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_BOOT_RPI_H
#define XAGENT_RPI5CAR_BOOT_RPI_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBoot.h"
#include "xWalk_Rpi5CarAgentConfigType.h"

namespace xwalk::hal
{
    class XWalkBoardControl;
    class XWalkConfigStore;
    class XWalkI2c;
    class XWalkMotors;
    struct XWalkDeviceInformation;
    struct XWalkGpioCallbacks;
} // namespace xwalk::hal

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkBootRpi
     * @brief Owns one complete command-specific Raspberry Pi boot lifetime.
     */
    class XWalkBootRpi final : private XWalkBoot
    {
        private:
            /** @brief Hardware graph selected for this process operation. */
            agent::uint8 selectedMode{XWALK_BOOT_BASE_REQ};
            /** @brief Owned writable PiCar-X configuration path. */
            agent::string configurationFilePath{};

        protected:
            /** @brief Suspends one Agent action without application dependencies. */
            static void delayMilliseconds(agent::contextpointer context, agent::uint32 durationMs);
            /** @brief Writes one angle through a boot-owned twelve-servo table. */
            static void
            setServoZeroingAngle(agent::contextpointer context, agent::uint8 servoId, agent::float64 angleDegrees);
            /** @brief Allows provider-local waits while application cancellation is checked externally. */
            static agent::boolean continueComputerVision(agent::contextpointer context) noexcept;
            /** @brief Selects one random source-compatible treasure color. */
            static XWalkComputerVisionColor selectTreasureColor(agent::contextpointer context);
            /** @brief Suspends one self-drive action and reports completion. */
            static agent::boolean selfDriveDelayMilliseconds(agent::contextpointer context,
                                                             agent::uint32 durationMs) noexcept;
            /** @brief Accepts speaker priming without emitting an audible sample. */
            static void primeSpeaker(agent::contextpointer context, agent::uint32 durationMs);
            /** @brief Parses one bounded unsigned decimal deployment value. */
            static agent::uint32
            parseUnsigned(agent::stringview value, agent::stringview optionName, agent::uint32 maximum);
            /** @brief Applies fail-safe automatic or explicit Robot HAT selection. */
            static hal::XWalkDeviceInformation selectBoard(const hal::XWalkDeviceInformation& detectedInformation,
                                                           agent::stringview requestedBoard);
            /** @brief Runs the bounded MCU-reset Doctor mode. */
            agent::int32 runDoctor(const xAgentContext& parameters);
            /** @brief Runs the camera-only text-and-vision mode. */
            agent::int32 runTextVisionTalk(const xAgentContext& parameters);
            /** @brief Runs the online language-model mode. */
            agent::int32 runOnlineLlmTest(const xAgentContext& parameters);
            /** @brief Runs the camera-only computer-vision mode. */
            agent::int32 runComputerVision(const xAgentContext& parameters);
            /** @brief Runs the camera-only video-recording mode. */
            agent::int32 runVideoRecording(const xAgentContext& parameters);
            /** @brief Runs camera-only MJPEG HTTP streaming. */
            agent::int32 runVideoStreaming(const xAgentContext& parameters);
            /** @brief Returns monotonic milliseconds for the stream transport. */
            static agent::uint64 videoStreamClock(agent::contextpointer context) noexcept;
            /** @brief Runs the isolated SPI-transfer mode. */
            agent::int32 runSpiTransfer(const xAgentContext& parameters);
            /** @brief Composes the Robot HAT graph used by actuator modes. */
            agent::int32 runVehicle(const xAgentContext& parameters);
            /** @brief Runs the isolated twelve-channel servo-zeroing mode. */
            agent::int32 runServoZeroing(const xAgentContext& parameters);
            /** @brief Selects one mode after the common PiCar-X graph is available. */
            agent::int32 runVehicleMode(const xAgentContext& parameters);
            /** @brief Runs the base PiCar-X mode. */
            agent::int32 runBase(const xAgentContext& parameters);
            /** @brief Runs face tracking. */
            agent::int32 runFaceTracking(const xAgentContext& parameters);
            /** @brief Runs treasure hunt. */
            agent::int32 runTreasureHunt(const xAgentContext& parameters);
            /** @brief Runs bull fight. */
            agent::int32 runBullFight(const xAgentContext& parameters);
            /** @brief Runs video car. */
            agent::int32 runVideoCar(const xAgentContext& parameters);
            /** @brief Runs mobile application control. */
            agent::int32 runAppControl(const xAgentContext& parameters);
            /** @brief Runs line tracking. */
            agent::int32 runLineTracking(const xAgentContext& parameters);
            /** @brief Runs self drive. */
            agent::int32 runSelfDrive(const xAgentContext& parameters);
            /** @brief Runs standalone sound control. */
            agent::int32 runSound(const xAgentContext& parameters);
            /** @brief Runs sound and background music. */
            agent::int32 runSoundBackgroundMusic(const xAgentContext& parameters);
            /** @brief Runs wake-word vehicle control. */
            agent::int32 runVoiceControlledCar(const xAgentContext& parameters);
            /** @brief Runs the spoken movement demonstration. */
            agent::int32 runVoicePromptCar(const xAgentContext& parameters);
            /** @brief Runs the storytelling robot. */
            agent::int32 runStorytellingRobot(const xAgentContext& parameters);
            /** @brief Runs the local voice chatbot. */
            agent::int32 runVoiceChat(const xAgentContext& parameters);
            /** @brief Runs the Rolly voice-active-car profile. */
            agent::int32 runVoiceActiveCar(const xAgentContext& parameters);
            /** @brief Runs the Gemini-backed Jarvis voice-active-car profile. */
            agent::int32 runVoiceActiveCarGpt(const xAgentContext& parameters);
            /** @brief Runs the GPT-car profile. */
            agent::int32 runGptCar(const xAgentContext& parameters);
            /** @brief Composes one of the three voice-active vehicle profiles. */
            agent::int32 runVoiceActiveMode(agent::uint8 mode, const xAgentContext& parameters);

        public:
            /**
             * @brief Stores one boot selection without claiming hardware.
             * @param[in] mode Minimum hardware graph required by the application command.
             * @param[in] configFilePath Non-empty writable PiCar-X configuration path.
             * @throws std::invalid_argument If `configFilePath` is empty.
             */
            XWalkBootRpi(agent::uint8 mode, agent::stringview configFilePath);

            /** @brief Releases retained boot state after the process operation. */
            ~XWalkBootRpi() = default;

            XWalkBootRpi(XWalkBootRpi&&) = delete;
            XWalkBootRpi(const XWalkBootRpi&) = delete;
            XWalkBootRpi& operator=(XWalkBootRpi&&) = delete;
            XWalkBootRpi& operator=(const XWalkBootRpi&) = delete;

            /**
             * @brief Claims hardware and executes one application callback.
             * @param[in] parameters Application context and non-null callback
             * retained through the selected synchronous composition.
             * @return Status returned by the configured callback.
             * @throws std::invalid_argument If the configured callback is null.
             * @throws std::logic_error If this object already started once.
             * @warning Claims only the physical resources required by the selected mode.
             */
            agent::int32 run(const xAgentContext& parameters);
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_BOOT_RPI_H */
