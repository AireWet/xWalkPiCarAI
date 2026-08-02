/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpi.cpp
 * @brief       Implements the Raspberry Pi xWalk hardware composition owner.
 *
 * @details
 * Claims the command-specific graph, performs MCU reset, selects the Robot HAT
 * motor topology, and releases every resource after the application callback.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xAgent_Rpi5CarCameraCapture.h"
#include "xAgent_Rpi5CarDoctorLinux.h"
#include "xAgent_Rpi5CarPicarxConfiguration.h"
#include "xAgent_Rpi5CarSpiTransfer.h"
#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarCameraLinux.h"
#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarDevice.h"
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarI2cLinux.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarMusicAlsa.h"
#include "xHal_Rpi5CarMusicSndFileDecoder.h"
#include "xHal_Rpi5CarSpiLinux.h"
#include "xHal_Rpi5CarSpeechRecognizerVosk.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTextToSpeechAlsa.h"
#include "xHal_Rpi5CarTextToSpeechEspeak.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Stores one boot selection without claiming hardware.
 * @param[in] mode Minimum hardware graph required by the application command.
 * @param[in] configFilePath Non-empty writable PiCar-X configuration path.
 * @throws std::invalid_argument If the path or mode is invalid.
 */
XWalkBootRpi::XWalkBootRpi(XWalkBootMode mode, hal::stringview configFilePath):
    selectedMode(mode), configurationFilePath(configFilePath)
{
    if (configurationFilePath.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("xWalkBoot configuration file path is required");
    }
    switch (selectedMode)
    {
        case XWalkBootMode::Base:
        case XWalkBootMode::Doctor:
        case XWalkBootMode::LineTracking:
        case XWalkBootMode::SelfDrive:
        case XWalkBootMode::Sound:
        case XWalkBootMode::VoiceChat:
        case XWalkBootMode::VoiceActiveCar:
        case XWalkBootMode::VoiceActiveCarGpt:
        case XWalkBootMode::VoiceControlledCar:
        case XWalkBootMode::VoicePromptCar:
        case XWalkBootMode::SpiTransfer:
            break;
        default:
            XHAL_THROW_INVALID_ARGUMENT("xWalkBoot mode is invalid");
    }
}

/******************************************************************************
 * Private member function definitions
 ******************************************************************************/

/**
 * @brief Suspends one Agent action on the calling thread.
 * @param[in] context Optional context; unused.
 * @param[in] durationMs Requested duration in milliseconds.
 */
void XWalkBootRpi::delayMilliseconds(hal::contextpointer context,
    hal::uint32 durationMs)
{
    static_cast<void>(context);
    hal::common::sleepMilliseconds(durationMs);
}

/**
 * @brief Suspends one self-drive action and reports completion.
 * @param[in] context Optional context; unused.
 * @param[in] durationMs Requested duration in milliseconds.
 * @return Always `true` after the requested delay completes.
 */
hal::boolean XWalkBootRpi::selfDriveDelayMilliseconds(hal::contextpointer context,
    hal::uint32 durationMs) noexcept
{
    static_cast<void>(context);
    hal::common::sleepMilliseconds(durationMs);
    return true;
}

/**
 * @brief Accepts the bounded speaker-prime request without emitting PCM.
 * @param[in] context Optional context; unused.
 * @param[in] durationMs Requested duration; unused by this silent callback.
 */
void XWalkBootRpi::primeSpeaker(hal::contextpointer context,
    hal::uint32 durationMs)
{
    static_cast<void>(context);
    static_cast<void>(durationMs);
}

/**
 * @brief Parses one bounded unsigned decimal deployment value.
 * @param[in] value Non-empty decimal digits without a sign or separator.
 * @param[in] optionName Non-empty option name included in validation failures.
 * @param[in] maximum Inclusive maximum accepted value.
 * @return Parsed unsigned value.
 * @throws std::invalid_argument If the value is empty or contains a non-digit.
 * @throws std::out_of_range If the value exceeds `maximum`.
 */
hal::uint32 XWalkBootRpi::parseUnsigned(hal::stringview value,
    hal::stringview optionName, hal::uint32 maximum)
{
    if (value.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT_DETAIL(optionName, " must not be empty");
    }
    hal::uint32 result{};
    for (const char character : value)
    {
        if ((character < '0') || (character > '9'))
        {
            XHAL_THROW_INVALID_ARGUMENT_DETAIL(optionName,
                " must contain decimal digits only");
        }
        const hal::uint32 digit = static_cast<hal::uint32>(character - '0');
        const hal::uint32 maximumPrefix = maximum / 10U;
        const hal::uint32 maximumDigit = maximum % 10U;
        if ((result > maximumPrefix) ||
            ((result == maximumPrefix) && (digit > maximumDigit)))
        {
            XHAL_THROW_OUT_OF_RANGE_DETAIL(optionName, " exceeds its range");
        }
        result = (result * 10U) + digit;
    }
    return result;
}

/**
 * @brief Applies fail-safe automatic or explicit Robot HAT selection.
 * @param[in] detectedInformation Read-only Device Tree discovery result.
 * @param[in] requestedBoard Exact `auto`, `robot_hat_v4`, or `robot_hat_v5` value.
 * @return Validated board information used for hardware composition.
 * @throws std::runtime_error If the requested board cannot be verified safely.
 * @throws std::invalid_argument If the requested board name is unsupported.
 */
hal::XWalkDeviceInformation XWalkBootRpi::selectBoard(
    const hal::XWalkDeviceInformation& detectedInformation,
    hal::stringview requestedBoard)
{
    if (requestedBoard == "auto")
    {
        if (!detectedInformation.detected)
        {
            XHAL_THROW_RUNTIME_ERROR(
                "Robot HAT v5 was not detected; select robot_hat_v4 explicitly when applicable");
        }
        return detectedInformation;
    }
    if (requestedBoard == "robot_hat_v4")
    {
        if (detectedInformation.detected)
        {
            XHAL_THROW_RUNTIME_ERROR(
                "Configured Robot HAT v4 conflicts with detected Robot HAT v5");
        }
        return {};
    }
    if (requestedBoard == "robot_hat_v5")
    {
        if (!detectedInformation.detected ||
            (detectedInformation.model != hal::XWalkDeviceModel::RobotHatV5))
        {
            XHAL_THROW_RUNTIME_ERROR(
                "Configured Robot HAT v5 was not verified by Device Tree");
        }
        return detectedInformation;
    }
    XHAL_THROW_INVALID_ARGUMENT(
        "hardware_board must be auto, robot_hat_v4, or robot_hat_v5");
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Claims hardware and executes one application callback.
 * @param[in,out] context Nullable caller-owned application context.
 * @param[in] callback Non-null callback completed before hardware teardown.
 * @return Status returned by `callback`.
 * @throws std::invalid_argument If `callback` is null.
 * @throws std::logic_error If this object already started once.
 * @warning Claims physical I2C, GPIO, motor, servo, and optional audio resources.
 */
hal::int32 XWalkBootRpi::run(hal::contextpointer context,
    bootapplicationcallback callback)
{
    begin(callback);

    if (selectedMode == XWalkBootMode::Doctor)
    {
        const hal::stringvector doctorLines =
            XWalkDoctorLinux::inspect(configurationFilePath);
        XWalkBootServices services{};
        services.doctorLines = &doctorLines;
        return callback(context, services);
    }

    hal::XWalkConfigStore config(configurationFilePath);
    const hal::string soundDirectory = config.get(
        "resource_sound_directory", "/usr/share/xwalk/sounds");
    if (selectedMode == XWalkBootMode::SpiTransfer)
    {
        const hal::string spiDevice = config.get(
            "hardware_spi_device", XHAL_RPI5CAR_SPI_DEFAULT_DEVICE);
        hal::XWalkSpiConfiguration spiConfiguration{};
        spiConfiguration.speedHz = parseUnsigned(config.get(
            "hardware_spi_speed_hz", "500000"), "hardware_spi_speed_hz",
            hal::common::UINT32_MAXIMUM);
        spiConfiguration.mode = static_cast<hal::uint8>(parseUnsigned(config.get(
            "hardware_spi_mode", "0"), "hardware_spi_mode",
            XHAL_RPI5CAR_SPI_MAXIMUM_MODE));
        spiConfiguration.bitsPerWord = static_cast<hal::uint8>(parseUnsigned(config.get(
            "hardware_spi_bits_per_word", "8"), "hardware_spi_bits_per_word",
            XHAL_RPI5CAR_SPI_MAXIMUM_BITS_PER_WORD));
        hal::XWalkSpiLinux spiBackend(spiDevice.c_str(), spiConfiguration);
        hal::XWalkSpi spi(&spiBackend,
            XHAL_SPI_TRANSFER_CALLBACK(hal::XWalkSpiLinux));
        XWalkSpiTransfer spiTransfer(spi);
        XWalkBootServices services{};
        services.spiTransfer = &spiTransfer;
        return callback(context, services);
    }

    const hal::string i2cDevice = config.get(
        "hardware_i2c_device", XHAL_RPI5CAR_I2C_DEFAULT_DEVICE);
    const hal::string gpioDevice = config.get(
        "hardware_gpio_device", XHAL_RPI5CAR_GPIO_DEFAULT_DEVICE);
    const hal::string deviceTreeRoot = config.get(
        "hardware_device_tree_root", XHAL_RPI5CAR_DEVICE_TREE_ROOT);
    const hal::string requestedBoard = config.get("hardware_board", "auto");
    const hal::string gpioChipName = config.get("hardware_gpio_chip_name", "");
    const hal::string gpioChipLabel = config.get("hardware_gpio_chip_label", "");
    constexpr hal::uint32 minimumGpioLineCount{28U};

    hal::XWalkDevice device(deviceTreeRoot);
    const hal::XWalkDeviceInformation deviceInformation = selectBoard(
        device.information(), requestedBoard);
    hal::XWalkI2cLinux i2cBackend(i2cDevice.c_str());
    hal::XWalkI2c i2c(&i2cBackend, XHAL_I2C_PROBE_CALLBACK(hal::XWalkI2cLinux),
        XHAL_I2C_WRITE_REGISTER_CALLBACK(hal::XWalkI2cLinux),
        XHAL_I2C_READ_CALLBACK(hal::XWalkI2cLinux),
        XHAL_I2C_READ_REGISTER_CALLBACK(hal::XWalkI2cLinux),
        XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(hal::XWalkI2cLinux));
    const hal::XWalkGpioCallbacks gpioCallbacks =
        XHAL_GPIO_CALLBACKS(hal::XWalkGpioLinux);
    hal::XWalkGpioLinux speakerBackend(gpioDevice.c_str(), gpioChipName,
        gpioChipLabel, minimumGpioLineCount);
    hal::XWalkGpio speakerGpio(
        &speakerBackend, gpioCallbacks, deviceInformation.speakerEnablePin);

    hal::XWalkGpioLinux resetBackend(gpioDevice.c_str(), gpioChipName,
        gpioChipLabel, minimumGpioLineCount);
    hal::XWalkGpio resetGpio(&resetBackend, gpioCallbacks, "MCURST");
    hal::XWalkAdc batteryAdc(i2c, "A4");
    hal::XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc,
        nullptr, &primeSpeaker);
    boardControl.resetMcu();
    hal::common::sleepMilliseconds(200U);
    if ((selectedMode == XWalkBootMode::SelfDrive) ||
        (selectedMode == XWalkBootMode::Sound))
    {
        boardControl.enableSpeaker();
    }

    hal::XWalkPwmTimerState timerState;
    hal::XWalkPwm panPwm(i2c, "P0", {}, timerState);
    hal::XWalkPwm tiltPwm(i2c, "P1", {}, timerState);
    hal::XWalkPwm directionPwm(i2c, "P2", {}, timerState);
    hal::XWalkServo panServo(panPwm);
    hal::XWalkServo tiltServo(tiltPwm);
    hal::XWalkServo directionServo(directionPwm);
    hal::XWalkGpioLinux triggerBackend(gpioDevice.c_str(), gpioChipName,
        gpioChipLabel, minimumGpioLineCount);
    hal::XWalkGpioLinux echoBackend(gpioDevice.c_str(), gpioChipName,
        gpioChipLabel, minimumGpioLineCount);
    hal::XWalkGpio trigger(&triggerBackend, gpioCallbacks, "D2");
    hal::XWalkGpio echo(&echoBackend, gpioCallbacks, "D3");
    hal::XWalkAdc adc0(i2c, "A0");
    hal::XWalkAdc adc1(i2c, "A1");
    hal::XWalkAdc adc2(i2c, "A2");
    hal::XWalkGrayscaleModule grayscale(adc0, adc1, adc2);
    hal::XWalkUltrasonic ultrasonic(trigger, echo);
    const auto runApplication = [&](hal::XWalkMotors& motors) -> hal::int32
    {
        XWalkPicarx picarx(motors, directionServo, panServo, tiltServo,
            grayscale, ultrasonic, config);
        if (selectedMode == XWalkBootMode::LineTracking)
        {
            XWalkLineTracking lineTracking(picarx, nullptr, &delayMilliseconds);
            XWalkBootServices services{};
            services.picarx = &picarx;
            services.lineTracking = &lineTracking;
            return callback(context, services);
        }
        if ((selectedMode == XWalkBootMode::SelfDrive) ||
            (selectedMode == XWalkBootMode::Sound))
        {
            hal::XWalkAudioAlsa audioBackend;
            hal::XWalkMusicAlsa musicBackend(audioBackend, nullptr,
                hal::XWalkMusicSndFileDecoder::operations());
            hal::XWalkMusic music(&musicBackend, musicBackend.callbacks());
            if (selectedMode == XWalkBootMode::SelfDrive)
            {
                XWalkSelfDrive selfDrive(picarx, music, nullptr,
                    &selfDriveDelayMilliseconds, nullptr, soundDirectory);
                XWalkBootServices services{};
                services.picarx = &picarx;
                services.selfDrive = &selfDrive;
                services.music = &music;
                return callback(context, services);
            }
            XWalkBootServices services{};
            services.picarx = &picarx;
            services.music = &music;
            return callback(context, services);
        }
        if (selectedMode == XWalkBootMode::VoiceControlledCar)
        {
            const hal::string voskLibrary = config.get(
                "voice_vosk_library", "libvosk.so");
            const hal::string voskModel = config.get(
                "voice_vosk_model", "/usr/share/vosk-model-small-en-us-0.15");
            const hal::string captureDevice = config.get(
                "voice_capture_device", "default");
            hal::XWalkSpeechRecognizerVosk recognizer(voskLibrary, voskModel);
            hal::XWalkSpeechToTextAlsa speechBackend(
                captureDevice, &recognizer, recognizer.operations());
            hal::XWalkSpeechToText speechToText(
                &speechBackend, speechBackend.callbacks());
            XWalkBootServices services{};
            services.picarx = &picarx;
            services.speechToText = &speechToText;
            return callback(context, services);
        }
        if (selectedMode == XWalkBootMode::VoicePromptCar)
        {
            const hal::string playbackDevice = config.get(
                "voice_playback_device", "default");
            const hal::string mixerDevice = config.get(
                "voice_mixer_device", "default");
            const hal::string mixerElement = config.get(
                "voice_mixer_element", "PCM");
            const hal::string espeakExecutable = config.get(
                "voice_espeak_executable", "espeak-ng");
            const hal::string espeakVoice = config.get("voice_espeak_voice", "en");
            hal::XWalkAudioAlsa audioBackend(
                playbackDevice, mixerDevice, mixerElement);
            hal::XWalkTextToSpeechEspeak espeak(espeakExecutable, espeakVoice);
            hal::XWalkTextToSpeechAlsa speechBackend(
                audioBackend, &espeak, espeak.operations());
            hal::XWalkTextToSpeech textToSpeech(
                boardControl, &speechBackend, speechBackend.callback());
            XWalkBootServices services{};
            services.picarx = &picarx;
            services.textToSpeech = &textToSpeech;
            return callback(context, services);
        }
        if ((selectedMode == XWalkBootMode::VoiceChat) ||
            (selectedMode == XWalkBootMode::VoiceActiveCar) ||
            (selectedMode == XWalkBootMode::VoiceActiveCarGpt))
        {
            const hal::string voskLibrary = config.get(
                "voice_vosk_library", "libvosk.so");
            const hal::string voskModel = config.get(
                "voice_vosk_model", "/usr/share/vosk-model-small-en-us-0.15");
            const hal::string captureDevice = config.get(
                "voice_capture_device", "default");
            const hal::string playbackDevice = config.get(
                "voice_playback_device", "default");
            const hal::string mixerDevice = config.get(
                "voice_mixer_device", "default");
            const hal::string mixerElement = config.get(
                "voice_mixer_element", "PCM");
            const hal::string espeakExecutable = config.get(
                "voice_espeak_executable", "espeak-ng");
            const hal::string espeakVoice = config.get(
                "voice_espeak_voice", "en");
            const hal::string modelProvider = config.get(
                "voice_language_model_provider", "ollama");
            const hal::string modelEndpoint = config.get(
                "voice_language_model_endpoint",
                config.get("voice_ollama_endpoint", "http://127.0.0.1:11434/api/chat"));
            const hal::string modelName = config.get(
                "voice_language_model_model",
                config.get("voice_ollama_model", "qwen2.5:0.5b"));
            const hal::string modelApiKey = config.get(
                "voice_language_model_api_key", "");
            const hal::uint32 maximumOutputTokens = parseUnsigned(config.get(
                "voice_language_model_maximum_output_tokens", "1024"),
                "voice_language_model_maximum_output_tokens",
                XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_MAXIMUM_OUTPUT_TOKENS);
            const hal::XWalkLanguageModelHttpDialect modelDialect =
                hal::XWalkLanguageModelHttp::dialectFromString(modelProvider);

            hal::XWalkSpeechRecognizerVosk recognizer(voskLibrary, voskModel);
            hal::XWalkSpeechToTextAlsa speechToTextBackend(
                captureDevice, &recognizer, recognizer.operations());
            hal::XWalkSpeechToText speechToText(
                &speechToTextBackend, speechToTextBackend.callbacks());
            hal::XWalkAudioAlsa audioBackend(
                playbackDevice, mixerDevice, mixerElement);
            hal::XWalkTextToSpeechEspeak espeak(espeakExecutable, espeakVoice);
            hal::XWalkTextToSpeechAlsa textToSpeechBackend(
                audioBackend, &espeak, espeak.operations());
            hal::XWalkTextToSpeech textToSpeech(
                boardControl, &textToSpeechBackend, textToSpeechBackend.callback());
            hal::XWalkLanguageModelHttp modelBackend(modelDialect,
                modelEndpoint, modelName, modelApiKey,
                XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_DEFAULT_TIMEOUT_MS,
                maximumOutputTokens);
            hal::XWalkLanguageModel languageModel(
                &modelBackend, modelBackend.callbacks());
            hal::XWalkVoiceAssistantConfiguration assistantConfiguration{};
            if (selectedMode == XWalkBootMode::VoiceChat)
            {
                assistantConfiguration.instructions =
                    XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_INSTRUCTIONS;
                assistantConfiguration.welcome =
                    XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME;
            }
            else if (selectedMode == XWalkBootMode::VoiceActiveCarGpt)
            {
                assistantConfiguration =
                    XWalkVoiceActiveCarGpt::assistantConfiguration();
            }
            hal::XWalkVoiceAssistant voiceAssistant(
                speechToText, languageModel, textToSpeech, assistantConfiguration);

            XWalkBootServices services{};
            services.picarx = &picarx;
            services.voiceAssistant = &voiceAssistant;
            if (selectedMode == XWalkBootMode::VoiceChat)
            {
                return callback(context, services);
            }

            hal::XWalkMusicAlsa musicBackend(audioBackend, nullptr,
                hal::XWalkMusicSndFileDecoder::operations());
            hal::XWalkMusic music(&musicBackend, musicBackend.callbacks());
            XWalkSelfDrive selfDrive(picarx, music, nullptr,
                &selfDriveDelayMilliseconds, nullptr, soundDirectory);
            hal::XWalkGpioLinux ledBackend(gpioDevice.c_str(), gpioChipName,
                gpioChipLabel, minimumGpioLineCount);
            hal::XWalkGpio ledGpio(&ledBackend, gpioCallbacks, "LED");
            hal::XWalkLed statusLed(ledGpio);
            const hal::string cameraConnectionText = config.get(
                "camera_connection", "csi");
            const hal::XWalkCameraConnection cameraConnection =
                hal::XWalkCamera::connectionFromString(cameraConnectionText);
            const hal::string cameraExecutable =
                (cameraConnection == hal::XWalkCameraConnection::Csi)
                    ? config.get("camera_csi_executable", "rpicam-still")
                    : config.get("camera_usb_executable", "ffmpeg");
            const hal::string cameraDevice = config.get(
                "camera_usb_device", "/dev/video0");
            const hal::string cameraOutput = config.get(
                "camera_output", "/tmp/xwalk-voice-image.jpg");
            hal::XWalkCameraLinux cameraBackend(
                cameraConnection, cameraExecutable, cameraDevice);
            hal::XWalkCamera camera(&cameraBackend, cameraBackend.callback());
            XWalkCameraCapture cameraCapture(camera, cameraOutput);
            services.selfDrive = &selfDrive;
            services.music = &music;
            services.voiceStatusLed = &statusLed;
            services.cameraCapture = &cameraCapture;
            return callback(context, services);
        }
        XWalkBootServices services{};
        services.picarx = &picarx;
        return callback(context, services);
    };

    if (deviceInformation.motorMode == XHAL_RPI5CAR_DEVICE_V5_MOTOR_MODE)
    {
        hal::XWalkPwm leftForwardPwm(i2c, "P12", {}, timerState);
        hal::XWalkPwm leftReversePwm(i2c, "P13", {}, timerState);
        hal::XWalkPwm rightForwardPwm(i2c, "P14", {}, timerState);
        hal::XWalkPwm rightReversePwm(i2c, "P15", {}, timerState);
        hal::XWalkMotor leftMotor(leftForwardPwm, leftReversePwm);
        hal::XWalkMotor rightMotor(rightForwardPwm, rightReversePwm);
        hal::XWalkMotors motors(leftMotor, rightMotor);
        return runApplication(motors);
    }

    hal::XWalkPwm leftPwm(i2c, "P13", {}, timerState);
    hal::XWalkPwm rightPwm(i2c, "P12", {}, timerState);
    hal::XWalkGpioLinux leftDirectionBackend(gpioDevice.c_str(), gpioChipName,
        gpioChipLabel, minimumGpioLineCount);
    hal::XWalkGpioLinux rightDirectionBackend(gpioDevice.c_str(), gpioChipName,
        gpioChipLabel, minimumGpioLineCount);
    hal::XWalkGpio leftDirection(&leftDirectionBackend, gpioCallbacks, "D4");
    hal::XWalkGpio rightDirection(&rightDirectionBackend, gpioCallbacks, "D5");
    hal::XWalkMotor leftMotor(leftPwm, leftDirection);
    hal::XWalkMotor rightMotor(rightPwm, rightDirection);
    hal::XWalkMotors motors(leftMotor, rightMotor);
    return runApplication(motors);
}

} /* namespace xwalk::agent */
