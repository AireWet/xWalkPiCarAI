/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorLinux.cpp
 * @brief       Implements the bounded Linux hardware preflight.
 *
 * @details
 * Pulses only the configured MCU reset GPIO before collecting deployment status
 * and emits a stable textual report for the CLI Doctor command.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi Doctor
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

#include "xAgent_Rpi5CarDoctorLinux.h"
#include "xHal_Rpi5CarTrace.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

#include <charconv>
#include <cstdio>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Reads one layered configuration value without modifying its files.
     * @param[in] filePath Existing configuration path.
     * @param[in] key Key whose final occurrence is selected.
     * @param[in] fallback Value returned when the key or file is unavailable.
     * @return Owned normalized configuration value or `fallback`.
     */
    agent::string
    XWalkDoctorLinux::configurationValue(agent::stringview filePath, agent::stringview key, agent::stringview fallback)
    {
        const agent::boolean filePathNotSet =
            static_cast<agent::boolean>(!hal::isReadableRegularFile(agent::filesystempath(filePath)));
        if (filePathNotSet)
        {
            return agent::string(fallback);
        }
        hal::XWalkConfigStore configuration(filePath);
        return configuration.get(key, fallback);
    }

    /**
     * @brief Appends one consistently formatted report result.
     * @param[in,out] lines Report receiving one line.
     * @param[in] passed Whether the check passed.
     * @param[in] name Stable check name.
     * @param[in] detail Human-readable result detail.
     */
    void XWalkDoctorLinux::appendResult(agent::stringvector& lines,
                                        agent::boolean passed,
                                        agent::stringview name,
                                        agent::stringview detail)
    {
        lines.emplace_back(agent::string(passed ? "[PASS] " : "[FAIL] ") + agent::string(name) + ": " +
                           agent::string(detail));
    }

    /**
     * @brief Appends one typed Doctor assessment result.
     * @param[in,out] lines Report receiving one line.
     * @param[in] name Stable check name.
     * @param[in] assessment Typed status and evidence detail.
     */
    void XWalkDoctorLinux::appendAssessment(agent::stringvector& lines,
                                            agent::stringview name,
                                            const XWalkDoctorAssessmentResult& assessment)
    {
        agent::string prefix("[WARN] ");
        if (assessment.status == XWalkDoctorResultStatus::Pass)
        {
            prefix = "[PASS] ";
        }
        else if (assessment.status == XWalkDoctorResultStatus::Fail)
        {
            prefix = "[FAIL] ";
        }
        lines.emplace_back(prefix + agent::string(name) + ": " + assessment.detail);
    }

    /**
     * @brief Reports whether one regular path is readable.
     * @param[in] path Non-empty filesystem path.
     * @return `true` when the path exists and is readable; otherwise `false`.
     */
    agent::boolean XWalkDoctorLinux::readablePath(agent::stringview path)
    {
        return !path.empty() && (::access(agent::string(path).c_str(), R_OK) == 0);
    }

    /**
     * @brief Reports whether one shared library can be loaded without retaining it.
     * @param[in] libraryName Non-empty dynamic-loader library name or path.
     * @return `true` when the library loads and closes successfully; otherwise `false`.
     */
    agent::boolean XWalkDoctorLinux::libraryAvailable(agent::stringview libraryName)
    {
        const agent::boolean libraryNameEmpty = static_cast<agent::boolean>(libraryName.empty());
        if (libraryNameEmpty)
        {
            return false;
        }
        void* const library = ::dlopen(agent::string(libraryName).c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (library == nullptr)
        {
            return false;
        }
        return ::dlclose(library) == 0;
    }

    /**
     * @brief Reads a short property file without throwing on I/O failure.
     * @param[in] path Property path.
     * @return Property content with one terminal null removed, or empty text on failure.
     */
    agent::string XWalkDoctorLinux::readProperty(agent::stringview path)
    {
        std::ifstream stream(agent::string(path), std::ios::binary);
        agent::string value((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        const agent::boolean trailingNullPresent =
            static_cast<agent::boolean>(!value.empty() && (value.back() == '\0'));
        if (trailingNullPresent)
        {
            value.pop_back();
        }
        return value;
    }

    /**
     * @brief Locates the supported Robot HAT v5 UUID below one Device Tree root.
     * @param[in] root Device Tree root whose direct HAT children are inspected.
     * @return `true` only when the supported v5 UUID is present.
     */
    agent::boolean XWalkDoctorLinux::robotHatV5Detected(agent::stringview root)
    {
        DIR* const directory = ::opendir(agent::string(root).c_str());
        if (directory == nullptr)
        {
            return false;
        }
        agent::boolean detected = false;
        for (dirent* entry = ::readdir(directory); entry != nullptr; entry = ::readdir(directory))
        {
            const agent::string name(entry->d_name);
            const agent::boolean nameMatched =
                static_cast<agent::boolean>(name.find(XHAL_RPI5CAR_DEVICE_HAT_NODE_MARKER) == agent::string::npos);
            if (nameMatched)
            {
                continue;
            }
            const agent::string uuidPath = agent::string(root) + "/" + name + "/" + XHAL_RPI5CAR_DEVICE_UUID_PROPERTY;
            const agent::boolean readPropertyUuidPathMatched =
                static_cast<agent::boolean>(readProperty(uuidPath) == XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID);
            if (readPropertyUuidPathMatched)
            {
                detected = true;
                break;
            }
        }
        static_cast<void>(::closedir(directory));
        return detected;
    }

    /**
     * @brief Parses one unsigned configuration value or returns its fallback.
     * @param[in] value Decimal configuration text.
     * @param[in] fallback Value returned when the complete text is not a valid unsigned integer.
     * @return Parsed value or `fallback`.
     */
    agent::uint32 XWalkDoctorLinux::configurationUnsigned(agent::stringview value, agent::uint32 fallback)
    {
        agent::uint32 parsed{};
        const std::from_chars_result result = std::from_chars(value.data(), value.data() + value.size(), parsed);
        return ((result.ec == std::errc{}) && (result.ptr == (value.data() + value.size()))) ? parsed : fallback;
    }

    /**
     * @brief Inspects GPIO identity and performs the bounded MCU reset pulse.
     * @param[in,out] lines Report receiving GPIO results.
     * @param[in] device GPIO character-device path.
     * @param[in] expectedName Configured exact chip name.
     * @param[in] expectedLabel Configured exact chip label.
     * @param[in] minimumLineCount Configured minimum number of GPIO lines.
     * @param[in] resetPin Configured Robot HAT reset-pin name.
     * @param[in] resetSettleMilliseconds Delay after the line is driven high.
     * @return Collected GPIO identity and reset-operation evidence.
     */
    XWalkDoctorGpioEvidence XWalkDoctorLinux::inspectAndResetGpio(agent::stringvector& lines,
                                                                  agent::stringview device,
                                                                  agent::stringview expectedName,
                                                                  agent::stringview expectedLabel,
                                                                  agent::uint32 minimumLineCount,
                                                                  agent::stringview resetPin,
                                                                  agent::uint32 resetSettleMilliseconds)
    {
        XWalkDoctorGpioEvidence evidence{};
        const agent::int32 descriptor = ::open(agent::string(device).c_str(), O_RDONLY | O_CLOEXEC);
        if (descriptor < 0)
        {
            appendResult(lines, false, "GPIO", agent::string(device) + " is unavailable");
            appendResult(lines, false, "MCU reset", "GPIO chip is unavailable");
            return evidence;
        }
        gpiochip_info information{};
        const agent::boolean inspected = ::ioctl(descriptor, GPIO_GET_CHIPINFO_IOCTL, &information) == 0;
        if (!inspected)
        {
            appendResult(lines, false, "GPIO", "chip metadata could not be read");
            appendResult(lines, false, "MCU reset", "GPIO chip metadata is unavailable");
            static_cast<void>(::close(descriptor));
            return evidence;
        }
        const agent::string name(information.name);
        const agent::string label(information.label);
        const agent::boolean countPassed = information.lines >= minimumLineCount;
        evidence.lineCountMatched = countPassed;
        appendResult(lines,
                     countPassed,
                     "GPIO chip",
                     name + ", label=" + label + ", lines=" + hal::common::uint32ToString(information.lines));
        const agent::boolean expectedNameExpectedLabelInvalid =
            static_cast<agent::boolean>(expectedName.empty() || expectedLabel.empty());
        if (expectedNameExpectedLabelInvalid)
        {
            appendResult(lines, false, "GPIO identity", "run provisioning to record chip name and label");
            appendResult(lines, false, "MCU reset", "GPIO identity is not configured");
            static_cast<void>(::close(descriptor));
            return evidence;
        }
        const agent::boolean matches = (name == expectedName) && (label == expectedLabel);
        evidence.identityMatched = matches;
        appendResult(lines,
                     matches,
                     "GPIO identity",
                     matches ? "configured identity matches" : "configured identity does not match the selected chip");
        if (!countPassed || !matches)
        {
            appendResult(lines, false, "MCU reset", "GPIO chip validation failed");
            static_cast<void>(::close(descriptor));
            return evidence;
        }

        hal::uint8 resetLine{};
        const agent::boolean resetPinResolved = hal::XWalkGpio::tryResolvePin(resetPin, resetLine);
        if (!resetPinResolved)
        {
            appendResult(lines, false, "MCU reset", agent::string(resetPin) + " is not a supported Robot HAT pin");
            static_cast<void>(::close(descriptor));
            return evidence;
        }
        gpiohandle_request request{};
        request.lineoffsets[0U] = resetLine;
        request.flags = GPIOHANDLE_REQUEST_OUTPUT;
        request.default_values[0U] = 0U;
        request.lines = 1U;
        static_cast<void>(
            std::snprintf(request.consumer_label, sizeof(request.consumer_label), "%s", "xwalk-doctor-reset"));
        const agent::boolean lineRequested = ::ioctl(descriptor, GPIO_GET_LINEHANDLE_IOCTL, &request) == 0;
        static_cast<void>(::close(descriptor));
        if (!lineRequested)
        {
            appendResult(lines, false, "MCU reset", "configured reset GPIO could not be requested");
            return evidence;
        }
        evidence.resetRequested = true;
        hal::common::sleepMilliseconds(10U);
        gpiohandle_data highLevel{};
        highLevel.values[0U] = 1U;
        const agent::boolean drivenHigh = ::ioctl(request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &highLevel) == 0;
        static_cast<void>(::close(request.fd));
        if (!drivenHigh)
        {
            appendResult(lines, false, "MCU reset", "reset GPIO could not be driven high");
            return evidence;
        }
        hal::common::sleepMilliseconds(resetSettleMilliseconds);
        evidence.resetCompleted = true;
        appendResult(lines,
                     true,
                     "MCU reset",
                     agent::string(resetPin) + " driven low for 10 ms, then high; settled for " +
                         hal::common::uint32ToString(resetSettleMilliseconds) + " ms");
        return evidence;
    }

    /**
     * @brief Reads Robot HAT firmware and battery data without constructing actuators.
     * @param[in,out] lines Report receiving I2C results.
     * @param[in] device Linux I2C character-device path.
     * @return Collected MCU response, firmware, address, and battery evidence.
     */
    XWalkDoctorI2cEvidence XWalkDoctorLinux::inspectI2c(agent::stringvector& lines, agent::stringview device)
    {
        XWalkDoctorI2cEvidence evidence{};
        const agent::int32 descriptor = ::open(agent::string(device).c_str(), O_RDWR | O_CLOEXEC);
        if (descriptor < 0)
        {
            appendResult(lines, false, "I2C", agent::string(device) + " is unavailable");
            appendResult(lines, false, "Robot HAT firmware", "not read");
            appendResult(lines, false, "Battery", "not read");
            return evidence;
        }
        evidence.deviceOpened = true;
        appendResult(lines, true, "I2C", agent::string(device) + " opened");
        const agent::fixedarray<agent::uint8, 2U> addresses{0x14U, 0x15U};
        agent::uint8 selectedAddress{};
        agent::fixedarray<agent::uint8, 3U> firmware{};
        for (const agent::uint8 address : addresses)
        {
            union i2c_smbus_data data = {};
            data.block[0U] = 3U;
            struct i2c_smbus_ioctl_data operation = {I2C_SMBUS_READ, 0x05U, I2C_SMBUS_I2C_BLOCK_DATA, &data};
            const agent::boolean addressSelected = ::ioctl(descriptor, I2C_SLAVE, address) == 0;
            const agent::boolean firmwareRead =
                addressSelected && (::ioctl(descriptor, I2C_SMBUS, &operation) == 0) && (data.block[0U] == 3U);
            if (firmwareRead)
            {
                selectedAddress = address;
                firmware[0U] = data.block[1U];
                firmware[1U] = data.block[2U];
                firmware[2U] = data.block[3U];
                break;
            }
        }
        if (selectedAddress == 0U)
        {
            appendResult(lines, false, "Robot HAT firmware", "no response at 0x14 or 0x15");
            appendResult(lines, false, "Battery", "not read because no Robot HAT responded");
            static_cast<void>(::close(descriptor));
            return evidence;
        }
        evidence.mcuResponded = true;
        evidence.firmwareRead = true;
        evidence.mcuAddress = selectedAddress;
        const agent::string firmwareText = hal::common::uint32ToString(firmware[0U]) + "." +
                                           hal::common::uint32ToString(firmware[1U]) + "." +
                                           hal::common::uint32ToString(firmware[2U]);
        appendResult(lines, true, "Robot HAT firmware", firmwareText);

        const agent::fixedarray<agent::uint8, 3U> adcCommand{0x13U, 0U, 0U};
        agent::fixedarray<agent::uint8, 2U> adcBytes{};
        const agent::boolean addressSelected = ::ioctl(descriptor, I2C_SLAVE, selectedAddress) == 0;
        const agent::boolean commandWritten =
            addressSelected &&
            (::write(descriptor, adcCommand.data(), adcCommand.size()) == static_cast<ssize_t>(adcCommand.size()));
        const agent::boolean sampleRead = commandWritten && (::read(descriptor, adcBytes.data(), adcBytes.size()) ==
                                                             static_cast<ssize_t>(adcBytes.size()));
        if (sampleRead)
        {
            evidence.batterySampleRead = true;
            const agent::uint16 count =
                static_cast<agent::uint16>((static_cast<agent::uint16>(adcBytes[0U]) << 8U) | adcBytes[1U]);
            const agent::float64 countValue = static_cast<agent::float64>(count);
            const agent::float64 scaledCount = countValue * 3.3;
            const agent::float64 adcVoltage = scaledCount / 4095.0;
            const agent::float64 batteryVoltage = adcVoltage * 3.0;
            appendResult(lines, true, "Battery", hal::common::float64ToString(batteryVoltage) + " V");
        }
        else
        {
            appendResult(lines, false, "Battery", "ADC A4 sample failed");
        }
        static_cast<void>(::close(descriptor));
        return evidence;
    }

    /**
     * @brief Checks one SPI device by opening and closing it without transferring data.
     * @param[in,out] lines Report receiving the SPI result.
     * @param[in] device Linux spidev path.
     */
    void XWalkDoctorLinux::inspectSpi(agent::stringvector& lines, agent::stringview device)
    {
        const agent::int32 descriptor = ::open(agent::string(device).c_str(), O_RDWR | O_CLOEXEC);
        const agent::boolean available = descriptor >= 0;
        if (available)
        {
            static_cast<void>(::close(descriptor));
        }
        appendResult(lines,
                     available,
                     "SPI",
                     available ? agent::string(device) + " opened without transfer"
                               : agent::string(device) + " is unavailable");
    }

    /**
     * @brief Checks passive camera, audio, model, executable, and library prerequisites.
     * @param[in,out] lines Report receiving optional-service results.
     * @param[in] configurationFilePath Existing deployment configuration path.
     */
    void XWalkDoctorLinux::inspectOptionalServices(agent::stringvector& lines, agent::stringview configurationFilePath)
    {
        const agent::string cameraConnection = configurationValue(configurationFilePath, "camera_connection", "csi");
        const agent::string cameraExecutable =
            configurationValue(configurationFilePath,
                               cameraConnection == "usb" ? "camera_usb_executable" : "camera_csi_executable",
                               cameraConnection == "usb" ? "ffmpeg" : "rpicam-still");
        const agent::string cameraDevice =
            configurationValue(configurationFilePath, "camera_usb_device", "/dev/video0");
        const agent::string csiDevice = configurationValue(configurationFilePath, "camera_csi_device", "/dev/media0");
        const agent::boolean cameraAvailable =
            executableAvailable(cameraExecutable) &&
            ((cameraConnection == "csi") ? readablePath(csiDevice) : readablePath(cameraDevice));
        appendResult(lines,
                     cameraAvailable,
                     "Camera",
                     cameraAvailable ? "provider and device metadata available"
                                     : "provider executable or USB device is missing");

        const agent::string visionDevice =
            configurationValue(configurationFilePath, "computer_vision_camera_device", "/dev/video0");
        const agent::string visionCascade =
            configurationValue(configurationFilePath,
                               "computer_vision_face_cascade",
                               "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml");
        const agent::boolean visionAvailable = readablePath(visionDevice) && readablePath(visionCascade);
        appendResult(lines,
                     visionAvailable,
                     "Computer vision",
                     visionAvailable ? "camera device and face cascade are readable"
                                     : "camera device or face cascade is unavailable");

        const agent::string pcmMetadata = readProperty("/proc/asound/pcm");
        const agent::boolean microphoneAvailable = pcmMetadata.find("capture") != agent::string::npos;
        const agent::boolean speakerAvailable = pcmMetadata.find("playback") != agent::string::npos;
        const agent::boolean mixerAvailable = readablePath("/dev/snd/controlC0");
        appendResult(lines,
                     microphoneAvailable,
                     "Microphone",
                     microphoneAvailable ? "ALSA capture metadata available; no stream was opened"
                                         : "no ALSA capture metadata found");
        appendResult(lines,
                     speakerAvailable,
                     "Speaker",
                     speakerAvailable ? "ALSA playback metadata available; output was not enabled"
                                      : "no ALSA playback metadata found");
        appendResult(lines,
                     mixerAvailable,
                     "Mixer",
                     mixerAvailable ? "ALSA control device available; mixer was not opened"
                                    : "ALSA control device unavailable");

        const agent::boolean alsaLibraryAvailable = libraryAvailable("libasound.so.2");
        const agent::boolean soundFileLibraryAvailable = libraryAvailable("libsndfile.so.1");
        appendResult(lines,
                     alsaLibraryAvailable,
                     "ALSA library",
                     alsaLibraryAvailable ? "loadable" : "libasound.so.2 is missing");
        appendResult(lines,
                     soundFileLibraryAvailable,
                     "SoundFile library",
                     soundFileLibraryAvailable ? "loadable" : "libsndfile.so.1 is missing");

        const agent::string voskLibrary =
            configurationValue(configurationFilePath, "voice_vosk_library", "/usr/lib/xwalk/libvosk.so");
        const agent::boolean voskAvailable = libraryAvailable(voskLibrary);
        appendResult(lines, voskAvailable, "Vosk library", voskAvailable ? "loadable" : voskLibrary + " is missing");
        const agent::string voskModel = configurationValue(configurationFilePath, "voice_vosk_model", "");
        appendResult(lines,
                     readablePath(voskModel),
                     "Voice model",
                     readablePath(voskModel) ? voskModel : "configured Vosk model is missing");
        const agent::string espeak = configurationValue(configurationFilePath, "voice_espeak_executable", "espeak-ng");
        appendResult(lines,
                     executableAvailable(espeak),
                     "Espeak",
                     executableAvailable(espeak) ? "executable available" : espeak + " is missing");
        const agent::string modelProvider =
            configurationValue(configurationFilePath, "voice_language_model_provider", "ollama");
        const agent::string modelEndpoint =
            configurationValue(configurationFilePath,
                               "voice_language_model_endpoint",
                               configurationValue(configurationFilePath, "voice_ollama_endpoint", ""));
        const agent::string modelEnvironment =
            configurationValue(configurationFilePath, "voice_language_model_model_environment", "");
        const agent::cstring configuredModel =
            modelEnvironment.empty() ? nullptr : std::getenv(modelEnvironment.c_str());
        const agent::string modelName =
            configuredModel == nullptr
                ? configurationValue(configurationFilePath,
                                     "voice_language_model_model",
                                     configurationValue(configurationFilePath, "voice_ollama_model", ""))
                : agent::string(configuredModel);
        const agent::string modelApiKeyEnvironment =
            configurationValue(configurationFilePath, "voice_language_model_api_key_environment", "");
        const agent::cstring modelApiKey =
            modelApiKeyEnvironment.empty() ? nullptr : std::getenv(modelApiKeyEnvironment.c_str());
        if (modelProvider == "ollama")
        {
            const agent::string ollamaManifest =
                configurationValue(configurationFilePath, "voice_ollama_model_manifest", "");
            const agent::boolean languageModelAvailable = executableAvailable("ollama") && !modelEndpoint.empty() &&
                                                          !modelName.empty() && readablePath(ollamaManifest);
            appendResult(lines,
                         languageModelAvailable,
                         "Language model",
                         languageModelAvailable
                             ? "Ollama executable, endpoint, model, and manifest available; endpoint not contacted"
                             : "Ollama executable, endpoint, model, or configured manifest is missing");
        }
        else
        {
            const agent::boolean supportedProvider =
                (modelProvider == "openai") || (modelProvider == "chatgpt") || (modelProvider == "gemini") ||
                (modelProvider == "grok") || (modelProvider == "xai") || (modelProvider == "claude") ||
                (modelProvider == "anthropic") || (modelProvider == "openai_compatible");
            const agent::boolean languageModelAvailable =
                supportedProvider && (modelEndpoint.substr(0U, 8U) == "https://") && !modelName.empty() &&
                (modelApiKey != nullptr) && (modelApiKey[0U] != '\0');
            appendResult(
                lines,
                languageModelAvailable,
                "Language model",
                languageModelAvailable
                    ? "Cloud provider, HTTPS endpoint, model, and credential environments are available; "
                      "endpoint not contacted"
                    : "Cloud provider configuration, model environment, or credential environment is incomplete "
                      "or unsupported");
        }
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Builds one bounded hardware preflight report.
     * @param[in] configurationFilePath Layered deployment configuration path inspected without mutation.
     * @return Owned report lines prefixed with `[PASS]`, `[WARN]`, or `[FAIL]`.
     */
    agent::stringvector XWalkDoctorLinux::inspect(agent::stringview configurationFilePath)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .073, "Doctor bounded Linux inspection started");
        agent::stringvector lines{"=== PiCar-X Bounded Hardware Preflight ==="};
        const agent::string path(configurationFilePath);
        const agent::boolean readable = ::access(path.c_str(), R_OK) == 0;
        const agent::boolean writable = ::access(path.c_str(), W_OK) == 0;
        appendResult(lines, readable, "Configuration readable", readable ? path : "file is not readable");
        appendResult(lines, writable, "Configuration writable", writable ? path : "file is not writable");

        const agent::string deviceTreeRoot =
            configurationValue(path, "hardware_device_tree_root", XHAL_RPI5CAR_DEVICE_TREE_ROOT);
        const agent::boolean v5Detected = robotHatV5Detected(deviceTreeRoot);
        const agent::string profile = configurationValue(path, "hardware_board", "auto");
        agent::boolean profileValid = false;
        agent::string profileDetail;
        if (profile == "robot_hat_v4")
        {
            profileValid = !v5Detected;
            profileDetail = profileValid ? "explicit Robot HAT v4 profile selected"
                                         : "v4 profile conflicts with detected Robot HAT v5 overlay";
        }
        else if (profile == "robot_hat_v5")
        {
            profileValid = v5Detected;
            profileDetail =
                profileValid ? "Robot HAT v5 profile and UUID agree" : "v5 profile is not verified by Device Tree";
        }
        else if (profile == "auto")
        {
            profileValid = v5Detected;
            profileDetail = profileValid ? "Robot HAT v5 UUID selected automatically"
                                         : "no v5 UUID found; select robot_hat_v4 manually when applicable";
        }
        else
        {
            profileDetail = "unsupported hardware_board value";
        }
        appendResult(lines, profileValid, "Robot HAT profile", profileDetail);

        const agent::string gpioDevice =
            configurationValue(path, "hardware_gpio_device", XHAL_RPI5CAR_GPIO_DEFAULT_DEVICE);
        const agent::uint32 minimumLineCount =
            configurationUnsigned(configurationValue(path, "hardware_gpio_minimum_line_count", "28"), 28U);
        const agent::uint32 resetSettleMilliseconds =
            configurationUnsigned(configurationValue(path, "hardware_mcu_reset_settle_ms", "200"), 200U);
        const XWalkDoctorGpioEvidence gpioEvidence =
            inspectAndResetGpio(lines,
                                gpioDevice,
                                configurationValue(path, "hardware_gpio_chip_name", ""),
                                configurationValue(path, "hardware_gpio_chip_label", ""),
                                minimumLineCount,
                                configurationValue(path, "hardware_mcu_reset_pin", "MCURST"),
                                resetSettleMilliseconds);
        const agent::string i2cDevice =
            configurationValue(path, "hardware_i2c_device", XHAL_RPI5CAR_I2C_DEFAULT_DEVICE);
        XWalkDoctorI2cEvidence i2cEvidence{};
        if (gpioEvidence.resetCompleted)
        {
            i2cEvidence = inspectI2c(lines, i2cDevice);
        }
        else
        {
            appendResult(lines, false, "I2C", "inspection skipped because MCU reset did not complete");
            appendResult(lines, false, "Robot HAT firmware", "not read");
            appendResult(lines, false, "Battery", "not read");
        }
        const XWalkDoctorRobotHatEvidence robotHatEvidence{v5Detected,
                                                           gpioEvidence.identityMatched,
                                                           gpioEvidence.resetCompleted,
                                                           i2cEvidence.mcuResponded,
                                                           i2cEvidence.firmwareRead,
                                                           i2cEvidence.batterySampleRead,
                                                           i2cEvidence.mcuAddress};
        appendAssessment(
            lines, "Robot HAT verification", XWalkDoctorAssessment::assessRobotHat(profile, robotHatEvidence));
        inspectSpi(lines, configurationValue(path, "hardware_spi_device", XHAL_RPI5CAR_SPI_DEFAULT_DEVICE));
        inspectOptionalServices(lines, path);
        const XWalkDoctorOperationState operationState{
            gpioEvidence.resetRequested, gpioEvidence.resetCompleted, false, false, false, false, false};
        appendAssessment(lines, "Safety", XWalkDoctorAssessment::assessSafety(operationState));
        return lines;
    }

} /* namespace xwalk::agent */
