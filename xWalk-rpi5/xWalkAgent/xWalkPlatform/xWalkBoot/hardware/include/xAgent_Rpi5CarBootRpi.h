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
            agent::int32 runDoctor(agent::contextpointer context, bootapplicationcallback callback);
            /** @brief Runs the camera-only text-and-vision mode. */
            agent::int32 runTextVisionTalk(agent::contextpointer context,
                                           bootapplicationcallback callback,
                                           hal::XWalkConfigStore& config);
            /** @brief Runs the online language-model mode. */
            agent::int32 runOnlineLlmTest(agent::contextpointer context,
                                          bootapplicationcallback callback,
                                          hal::XWalkConfigStore& config);
            /** @brief Runs the camera-only computer-vision mode. */
            agent::int32 runComputerVision(agent::contextpointer context,
                                           bootapplicationcallback callback,
                                           hal::XWalkConfigStore& config);
            /** @brief Runs the camera-only video-recording mode. */
            agent::int32 runVideoRecording(agent::contextpointer context,
                                           bootapplicationcallback callback,
                                           hal::XWalkConfigStore& config);
            /** @brief Runs the isolated SPI-transfer mode. */
            agent::int32 runSpiTransfer(agent::contextpointer applicationContext,
                                        bootapplicationcallback callback,
                                        hal::XWalkConfigStore& config);
            /** @brief Composes the Robot HAT graph used by actuator modes. */
            agent::int32
            runVehicle(agent::contextpointer context, bootapplicationcallback callback, hal::XWalkConfigStore& config);
            /** @brief Runs the isolated twelve-channel servo-zeroing mode. */
            agent::int32
            runServoZeroing(agent::contextpointer context, bootapplicationcallback callback, hal::XWalkI2c& i2c);
            /** @brief Selects one mode after the common PiCar-X graph is available. */
            agent::int32 runVehicleMode(agent::contextpointer context,
                                        bootapplicationcallback callback,
                                        hal::XWalkConfigStore& config,
                                        hal::XWalkBoardControl& boardControl,
                                        XWalkPicarx& picarx,
                                        agent::stringview gpioDevice,
                                        agent::stringview gpioChipName,
                                        agent::stringview gpioChipLabel,
                                        agent::uint32 minimumGpioLineCount,
                                        const hal::XWalkGpioCallbacks& gpioCallbacks);
            /** @brief Runs the base PiCar-X mode. */
            agent::int32 runBase(agent::contextpointer context, bootapplicationcallback callback, XWalkPicarx& picarx);
            /** @brief Runs face tracking. */
            agent::int32 runFaceTracking(agent::contextpointer context,
                                         bootapplicationcallback callback,
                                         hal::XWalkConfigStore& config,
                                         XWalkPicarx& picarx);
            /** @brief Runs treasure hunt. */
            agent::int32 runTreasureHunt(agent::contextpointer context,
                                         bootapplicationcallback callback,
                                         hal::XWalkConfigStore& config,
                                         hal::XWalkBoardControl& boardControl,
                                         XWalkPicarx& picarx);
            /** @brief Runs bull fight. */
            agent::int32 runBullFight(agent::contextpointer context,
                                      bootapplicationcallback callback,
                                      hal::XWalkConfigStore& config,
                                      XWalkPicarx& picarx);
            /** @brief Runs video car. */
            agent::int32 runVideoCar(agent::contextpointer context,
                                     bootapplicationcallback callback,
                                     hal::XWalkConfigStore& config,
                                     XWalkPicarx& picarx);
            /** @brief Runs mobile application control. */
            agent::int32 runAppControl(agent::contextpointer context,
                                       bootapplicationcallback callback,
                                       hal::XWalkConfigStore& config,
                                       XWalkPicarx& picarx);
            /** @brief Runs line tracking. */
            agent::int32
            runLineTracking(agent::contextpointer context, bootapplicationcallback callback, XWalkPicarx& picarx);
            /** @brief Runs self drive. */
            agent::int32 runSelfDrive(agent::contextpointer context,
                                      bootapplicationcallback callback,
                                      hal::XWalkConfigStore& config,
                                      XWalkPicarx& picarx);
            /** @brief Runs standalone sound control. */
            agent::int32 runSound(agent::contextpointer context,
                                  bootapplicationcallback callback,
                                  hal::XWalkConfigStore& config,
                                  XWalkPicarx& picarx);
            /** @brief Runs sound and background music. */
            agent::int32 runSoundBackgroundMusic(agent::contextpointer context,
                                                 bootapplicationcallback callback,
                                                 hal::XWalkConfigStore& config,
                                                 XWalkPicarx& picarx);
            /** @brief Runs wake-word vehicle control. */
            agent::int32 runVoiceControlledCar(agent::contextpointer context,
                                               bootapplicationcallback callback,
                                               hal::XWalkConfigStore& config,
                                               XWalkPicarx& picarx);
            /** @brief Runs the spoken movement demonstration. */
            agent::int32 runVoicePromptCar(agent::contextpointer context,
                                           bootapplicationcallback callback,
                                           hal::XWalkConfigStore& config,
                                           hal::XWalkBoardControl& boardControl,
                                           XWalkPicarx& picarx);
            /** @brief Runs the storytelling robot. */
            agent::int32 runStorytellingRobot(agent::contextpointer context,
                                              bootapplicationcallback callback,
                                              hal::XWalkConfigStore& config,
                                              hal::XWalkBoardControl& boardControl,
                                              XWalkPicarx& picarx);
            /** @brief Runs the local voice chatbot. */
            agent::int32 runVoiceChat(agent::contextpointer context,
                                      bootapplicationcallback callback,
                                      hal::XWalkConfigStore& config,
                                      hal::XWalkBoardControl& boardControl,
                                      XWalkPicarx& picarx);
            /** @brief Runs the Rolly voice-active-car profile. */
            agent::int32 runVoiceActiveCar(agent::contextpointer context,
                                           bootapplicationcallback callback,
                                           hal::XWalkConfigStore& config,
                                           hal::XWalkBoardControl& boardControl,
                                           XWalkPicarx& picarx,
                                           agent::stringview gpioDevice,
                                           agent::stringview gpioChipName,
                                           agent::stringview gpioChipLabel,
                                           agent::uint32 minimumGpioLineCount,
                                           const hal::XWalkGpioCallbacks& gpioCallbacks);
            /** @brief Runs the Buddy voice-active-car profile. */
            agent::int32 runVoiceActiveCarGpt(agent::contextpointer context,
                                              bootapplicationcallback callback,
                                              hal::XWalkConfigStore& config,
                                              hal::XWalkBoardControl& boardControl,
                                              XWalkPicarx& picarx,
                                              agent::stringview gpioDevice,
                                              agent::stringview gpioChipName,
                                              agent::stringview gpioChipLabel,
                                              agent::uint32 minimumGpioLineCount,
                                              const hal::XWalkGpioCallbacks& gpioCallbacks);
            /** @brief Runs the GPT-car profile. */
            agent::int32 runGptCar(agent::contextpointer context,
                                   bootapplicationcallback callback,
                                   hal::XWalkConfigStore& config,
                                   hal::XWalkBoardControl& boardControl,
                                   XWalkPicarx& picarx,
                                   agent::stringview gpioDevice,
                                   agent::stringview gpioChipName,
                                   agent::stringview gpioChipLabel,
                                   agent::uint32 minimumGpioLineCount,
                                   const hal::XWalkGpioCallbacks& gpioCallbacks);
            /** @brief Composes one of the three voice-active vehicle profiles. */
            agent::int32 runVoiceActiveMode(agent::uint8 mode,
                                            agent::contextpointer context,
                                            bootapplicationcallback callback,
                                            hal::XWalkConfigStore& config,
                                            hal::XWalkBoardControl& boardControl,
                                            XWalkPicarx& picarx,
                                            agent::stringview gpioDevice,
                                            agent::stringview gpioChipName,
                                            agent::stringview gpioChipLabel,
                                            agent::uint32 minimumGpioLineCount,
                                            const hal::XWalkGpioCallbacks& gpioCallbacks);

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
             * @param[in,out] context Nullable caller-owned application context.
             * @param[in] callback Non-null callback completed before hardware teardown.
             * @return Status returned by `callback`.
             * @throws std::invalid_argument If `callback` is null.
             * @throws std::logic_error If this object already started once.
             * @warning Claims only the physical resources required by the selected mode.
             */
            agent::int32 run(agent::contextpointer context, bootapplicationcallback callback);
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_BOOT_RPI_H */
