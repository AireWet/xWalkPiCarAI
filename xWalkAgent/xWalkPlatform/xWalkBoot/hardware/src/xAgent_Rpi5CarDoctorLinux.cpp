/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorLinux.cpp
 * @brief       Implements passive Linux deployment inspection.
 *
 * @details
 * Collects deployment status through non-actuating Linux operations and emits
 * a stable textual report for the CLI Doctor command.
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

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

#include <cstdlib>

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
agent::string XWalkDoctorLinux::configurationValue(agent::stringview filePath,
    agent::stringview key, agent::stringview fallback)
{
    const agent::boolean filePathNotSet =
        static_cast<agent::boolean>(
            !hal::isReadableRegularFile(agent::filesystempath(filePath)));
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
void XWalkDoctorLinux::appendResult(agent::stringvector& lines, agent::boolean passed,
    agent::stringview name, agent::stringview detail)
{
    lines.emplace_back(agent::string(passed ? "[PASS] " : "[FAIL] ") +
        agent::string(name) + ": " + agent::string(detail));
}

/**
 * @brief Appends one non-failing advisory result.
 * @param[in,out] lines Report receiving one line.
 * @param[in] name Stable check name.
 * @param[in] detail Human-readable advisory detail.
 */
void XWalkDoctorLinux::appendWarning(agent::stringvector& lines,
    agent::stringview name, agent::stringview detail)
{
    lines.emplace_back(agent::string("[WARN] ") + agent::string(name) + ": " + agent::string(detail));
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
 * @brief Reports whether one named or absolute executable is available.
 * @param[in] executable Executable name or absolute path.
 * @return `true` when an executable regular file is found; otherwise `false`.
 */
agent::boolean XWalkDoctorLinux::executableAvailable(agent::stringview executable)
{
    const agent::boolean executableEmpty =
        static_cast<agent::boolean>(
            executable.empty());
    if (executableEmpty)
    {
        return false;
    }
    const agent::string owned(executable);
    const agent::boolean ownedDifferent =
        static_cast<agent::boolean>(
            owned.find('/') != agent::string::npos);
    if (ownedDifferent)
    {
        return ::access(owned.c_str(), X_OK) == 0;
    }
    const agent::fixedarray<agent::cstring, 4U> roots{
        "/usr/local/bin/", "/usr/bin/", "/bin/", "/opt/homebrew/bin/"};
    for (const agent::cstring root : roots)
    {
        const agent::string candidate = agent::string(root) + owned;
        const agent::boolean candidateCStrMatched =
            static_cast<agent::boolean>(
                ::access(candidate.c_str(), X_OK) == 0);
        if (candidateCStrMatched)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Reports whether one shared library can be loaded without retaining it.
 * @param[in] libraryName Non-empty dynamic-loader library name or path.
 * @return `true` when the library loads and closes successfully; otherwise `false`.
 */
agent::boolean XWalkDoctorLinux::libraryAvailable(agent::stringview libraryName)
{
    const agent::boolean libraryNameEmpty =
        static_cast<agent::boolean>(
            libraryName.empty());
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
    agent::string value((std::istreambuf_iterator<char>(stream)),
        std::istreambuf_iterator<char>());
    const agent::boolean trailingNullPresent =
        static_cast<agent::boolean>(
            !value.empty() && (value.back() == '\0'));
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
            static_cast<agent::boolean>(
                name.find(XHAL_RPI5CAR_DEVICE_HAT_NODE_MARKER) == agent::string::npos);
        if (nameMatched)
        {
            continue;
        }
        const agent::string uuidPath = agent::string(root) + "/" + name + "/" +
            XHAL_RPI5CAR_DEVICE_UUID_PROPERTY;
        const agent::boolean readPropertyUuidPathMatched =
            static_cast<agent::boolean>(
                readProperty(uuidPath) == XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID);
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
 * @brief Inspects one GPIO chip without requesting a line.
 * @param[in,out] lines Report receiving GPIO results.
 * @param[in] device GPIO character-device path.
 * @param[in] expectedName Configured exact chip name.
 * @param[in] expectedLabel Configured exact chip label.
 */
void XWalkDoctorLinux::inspectGpio(agent::stringvector& lines, agent::stringview device,
    agent::stringview expectedName, agent::stringview expectedLabel)
{
    const agent::int32 descriptor = ::open(agent::string(device).c_str(), O_RDONLY | O_CLOEXEC);
    if (descriptor < 0)
    {
        appendResult(lines, false, "GPIO", agent::string(device) + " is unavailable");
        return;
    }
    gpiochip_info information{};
    const agent::boolean inspected = ::ioctl(descriptor, GPIO_GET_CHIPINFO_IOCTL, &information) == 0;
    static_cast<void>(::close(descriptor));
    if (!inspected)
    {
        appendResult(lines, false, "GPIO", "chip metadata could not be read");
        return;
    }
    const agent::string name(information.name);
    const agent::string label(information.label);
    const agent::boolean countPassed = information.lines >= 28U;
    appendResult(lines, countPassed, "GPIO chip", name + ", label=" + label +
        ", lines=" + hal::common::uint32ToString(information.lines));
    const agent::boolean expectedNameExpectedLabelInvalid =
        static_cast<agent::boolean>(
            expectedName.empty() || expectedLabel.empty());
    if (expectedNameExpectedLabelInvalid)
    {
        appendResult(lines, false, "GPIO identity", "run provisioning to record chip name and label");
        return;
    }
    const agent::boolean matches = (name == expectedName) && (label == expectedLabel);
    appendResult(lines, matches, "GPIO identity", matches ? "configured identity matches" :
        "configured identity does not match the selected chip");
}

/**
 * @brief Reads Robot HAT firmware and battery data without constructing actuators.
 * @param[in,out] lines Report receiving I2C results.
 * @param[in] device Linux I2C character-device path.
 */
void XWalkDoctorLinux::inspectI2c(agent::stringvector& lines, agent::stringview device)
{
    const agent::int32 descriptor = ::open(agent::string(device).c_str(), O_RDWR | O_CLOEXEC);
    if (descriptor < 0)
    {
        appendResult(lines, false, "I2C", agent::string(device) + " is unavailable");
        appendResult(lines, false, "Robot HAT firmware", "not read");
        appendResult(lines, false, "Battery", "not read");
        return;
    }
    appendResult(lines, true, "I2C", agent::string(device) + " opened");
    const agent::fixedarray<agent::uint8, 2U> addresses{0x14U, 0x15U};
    agent::uint8 selectedAddress{};
    agent::fixedarray<agent::uint8, 3U> firmware{};
    for (const agent::uint8 address : addresses)
    {
        agent::uint8 firmwareRegister{0x05U};
        i2c_msg messages[2U]{};
        messages[0U].addr = address;
        messages[0U].len = 1U;
        messages[0U].buf = &firmwareRegister;
        messages[1U].addr = address;
        messages[1U].flags = I2C_M_RD;
        messages[1U].len = 3U;
        messages[1U].buf = firmware.data();
        i2c_rdwr_ioctl_data operation{messages, 2U};
        const agent::boolean descriptorOperationMatched =
            static_cast<agent::boolean>(
                ::ioctl(descriptor, I2C_RDWR, &operation) == 0);
        if (descriptorOperationMatched)
        {
            selectedAddress = address;
            break;
        }
    }
    if (selectedAddress == 0U)
    {
        appendResult(lines, false, "Robot HAT firmware", "no response at 0x14 or 0x15");
        appendResult(lines, false, "Battery", "not read because no Robot HAT responded");
        static_cast<void>(::close(descriptor));
        return;
    }
    const agent::string firmwareText = hal::common::uint32ToString(firmware[0U]) + "." +
        hal::common::uint32ToString(firmware[1U]) + "." +
        hal::common::uint32ToString(firmware[2U]);
    appendResult(lines, true, "Robot HAT firmware", firmwareText);

    const agent::fixedarray<agent::uint8, 3U> adcCommand{0x13U, 0U, 0U};
    agent::fixedarray<agent::uint8, 2U> adcBytes{};
    const agent::boolean addressSelected = ::ioctl(descriptor, I2C_SLAVE, selectedAddress) == 0;
    const agent::boolean commandWritten = addressSelected &&
        (::write(descriptor, adcCommand.data(), adcCommand.size()) ==
            static_cast<ssize_t>(adcCommand.size()));
    const agent::boolean sampleRead = commandWritten &&
        (::read(descriptor, adcBytes.data(), adcBytes.size()) == static_cast<ssize_t>(adcBytes.size()));
    if (sampleRead)
    {
        const agent::uint16 count = static_cast<agent::uint16>(
            (static_cast<agent::uint16>(adcBytes[0U]) << 8U) | adcBytes[1U]);
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
    appendResult(lines, available, "SPI", available ?
        agent::string(device) + " opened without transfer" : agent::string(device) + " is unavailable");
}

/**
 * @brief Checks passive camera, audio, model, executable, and library prerequisites.
 * @param[in,out] lines Report receiving optional-service results.
 * @param[in] configurationFilePath Existing deployment configuration path.
 */
void XWalkDoctorLinux::inspectOptionalServices(agent::stringvector& lines,
    agent::stringview configurationFilePath)
{
    const agent::string cameraConnection = configurationValue(
        configurationFilePath, "camera_connection", "csi");
    const agent::string cameraExecutable = configurationValue(configurationFilePath,
        cameraConnection == "usb" ? "camera_usb_executable" : "camera_csi_executable",
        cameraConnection == "usb" ? "ffmpeg" : "rpicam-still");
    const agent::string cameraDevice = configurationValue(
        configurationFilePath, "camera_usb_device", "/dev/video0");
    const agent::string csiDevice = configurationValue(
        configurationFilePath, "camera_csi_device", "/dev/media0");
    const agent::boolean cameraAvailable = executableAvailable(cameraExecutable) &&
        ((cameraConnection == "csi") ? readablePath(csiDevice) : readablePath(cameraDevice));
    appendResult(lines, cameraAvailable, "Camera", cameraAvailable ?
        "provider and device metadata available" : "provider executable or USB device is missing");

    const agent::string visionDevice = configurationValue(
        configurationFilePath, "computer_vision_camera_device", "/dev/video0");
    const agent::string visionCascade = configurationValue(configurationFilePath,
        "computer_vision_face_cascade",
        "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml");
    const agent::boolean visionAvailable = readablePath(visionDevice) &&
        readablePath(visionCascade);
    appendResult(lines, visionAvailable, "Computer vision", visionAvailable ?
        "camera device and face cascade are readable" :
        "camera device or face cascade is unavailable");

    const agent::string pcmMetadata = readProperty("/proc/asound/pcm");
    const agent::boolean microphoneAvailable = pcmMetadata.find("capture") != agent::string::npos;
    const agent::boolean speakerAvailable = pcmMetadata.find("playback") != agent::string::npos;
    const agent::boolean mixerAvailable = readablePath("/dev/snd/controlC0");
    appendResult(lines, microphoneAvailable, "Microphone", microphoneAvailable ?
        "ALSA capture metadata available; no stream was opened" : "no ALSA capture metadata found");
    appendResult(lines, speakerAvailable, "Speaker", speakerAvailable ?
        "ALSA playback metadata available; output was not enabled" : "no ALSA playback metadata found");
    appendResult(lines, mixerAvailable, "Mixer", mixerAvailable ?
        "ALSA control device available; mixer was not opened" : "ALSA control device unavailable");

    const agent::boolean alsaLibraryAvailable = libraryAvailable("libasound.so.2");
    const agent::boolean soundFileLibraryAvailable = libraryAvailable("libsndfile.so.1");
    appendResult(lines, alsaLibraryAvailable, "ALSA library",
        alsaLibraryAvailable ? "loadable" : "libasound.so.2 is missing");
    appendResult(lines, soundFileLibraryAvailable, "SoundFile library",
        soundFileLibraryAvailable ? "loadable" : "libsndfile.so.1 is missing");

    const agent::string voskLibrary = configurationValue(
        configurationFilePath, "voice_vosk_library", "/usr/lib/xwalk/libvosk.so");
    const agent::boolean voskAvailable = libraryAvailable(voskLibrary);
    appendResult(lines, voskAvailable, "Vosk library", voskAvailable ?
        "loadable" : voskLibrary + " is missing");
    const agent::string voskModel = configurationValue(
        configurationFilePath, "voice_vosk_model", "");
    appendResult(lines, readablePath(voskModel), "Voice model", readablePath(voskModel) ?
        voskModel : "configured Vosk model is missing");
    const agent::string espeak = configurationValue(
        configurationFilePath, "voice_espeak_executable", "espeak-ng");
    appendResult(lines, executableAvailable(espeak), "Espeak", executableAvailable(espeak) ?
        "executable available" : espeak + " is missing");
    const agent::string modelProvider = configurationValue(
        configurationFilePath, "voice_language_model_provider", "ollama");
    const agent::string modelEndpoint = configurationValue(
        configurationFilePath, "voice_language_model_endpoint",
        configurationValue(configurationFilePath, "voice_ollama_endpoint", ""));
    const agent::string modelEnvironment = configurationValue(
        configurationFilePath, "voice_language_model_model_environment", "");
    const agent::cstring configuredModel = modelEnvironment.empty() ?
        nullptr : std::getenv(modelEnvironment.c_str());
    const agent::string modelName = configuredModel == nullptr ?
        configurationValue(configurationFilePath, "voice_language_model_model",
            configurationValue(configurationFilePath, "voice_ollama_model", "")) :
        agent::string(configuredModel);
    const agent::string modelApiKeyEnvironment = configurationValue(
        configurationFilePath, "voice_language_model_api_key_environment", "");
    const agent::cstring modelApiKey = modelApiKeyEnvironment.empty() ?
        nullptr : std::getenv(modelApiKeyEnvironment.c_str());
    if (modelProvider == "ollama")
    {
        const agent::string ollamaManifest = configurationValue(
            configurationFilePath, "voice_ollama_model_manifest", "");
        const agent::boolean languageModelAvailable = executableAvailable("ollama") &&
            !modelEndpoint.empty() && !modelName.empty() && readablePath(ollamaManifest);
        appendResult(lines, languageModelAvailable, "Language model", languageModelAvailable ?
            "Ollama executable, endpoint, model, and manifest available; endpoint not contacted" :
            "Ollama executable, endpoint, model, or configured manifest is missing");
    }
    else
    {
        const agent::boolean supportedProvider = (modelProvider == "openai") ||
            (modelProvider == "chatgpt") || (modelProvider == "gemini") ||
            (modelProvider == "claude") || (modelProvider == "anthropic") ||
            (modelProvider == "openai_compatible");
        const agent::boolean languageModelAvailable = supportedProvider &&
            (modelEndpoint.substr(0U, 8U) == "https://") &&
            !modelName.empty() && (modelApiKey != nullptr) && (modelApiKey[0U] != '\0');
        appendResult(lines, languageModelAvailable, "Language model", languageModelAvailable ?
            "Cloud provider, HTTPS endpoint, model, and credential environments are available; "
            "endpoint not contacted" :
            "Cloud provider configuration, model environment, or credential environment is incomplete "
            "or unsupported");
    }
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Builds one passive deployment report.
 * @param[in] configurationFilePath Layered deployment configuration path inspected without mutation.
 * @return Owned report lines prefixed with `[PASS]`, `[WARN]`, or `[FAIL]`.
 */
agent::stringvector XWalkDoctorLinux::inspect(agent::stringview configurationFilePath)
{
    agent::stringvector lines{"=== PiCar-X Passive Hardware Preflight ==="};
    const agent::string path(configurationFilePath);
    const agent::boolean readable = ::access(path.c_str(), R_OK) == 0;
    const agent::boolean writable = ::access(path.c_str(), W_OK) == 0;
    appendResult(lines, readable, "Configuration readable", readable ? path : "file is not readable");
    appendResult(lines, writable, "Configuration writable", writable ? path : "file is not writable");

    const agent::string deviceTreeRoot = configurationValue(
        path, "hardware_device_tree_root", XHAL_RPI5CAR_DEVICE_TREE_ROOT);
    const agent::boolean v5Detected = robotHatV5Detected(deviceTreeRoot);
    const agent::string profile = configurationValue(path, "hardware_board", "auto");
    agent::boolean profileValid = false;
    agent::string profileDetail;
    if (profile == "robot_hat_v4")
    {
        profileValid = !v5Detected;
        profileDetail = profileValid ? "explicit Robot HAT v4 profile selected" :
            "v4 profile conflicts with detected Robot HAT v5 overlay";
    }
    else if (profile == "robot_hat_v5")
    {
        profileValid = v5Detected;
        profileDetail = profileValid ? "Robot HAT v5 profile and UUID agree" :
            "v5 profile is not verified by Device Tree";
    }
    else if (profile == "auto")
    {
        profileValid = v5Detected;
        profileDetail = profileValid ? "Robot HAT v5 UUID selected automatically" :
            "no v5 UUID found; select robot_hat_v4 manually when applicable";
    }
    else
    {
        profileDetail = "unsupported hardware_board value";
    }
    if (v5Detected)
    {
        appendResult(lines, true, "Robot HAT discovery", "supported Robot HAT v5 UUID detected");
    }
    else
    {
        appendWarning(lines, "Robot HAT discovery",
            "supported v5 UUID not detected; this does not prove that v4 is connected");
    }
    appendResult(lines, profileValid, "Robot HAT profile", profileDetail);

    const agent::string i2cDevice = configurationValue(
        path, "hardware_i2c_device", XHAL_RPI5CAR_I2C_DEFAULT_DEVICE);
    inspectI2c(lines, i2cDevice);
    const agent::string gpioDevice = configurationValue(
        path, "hardware_gpio_device", XHAL_RPI5CAR_GPIO_DEFAULT_DEVICE);
    inspectGpio(lines, gpioDevice,
        configurationValue(path, "hardware_gpio_chip_name", ""),
        configurationValue(path, "hardware_gpio_chip_label", ""));
    inspectSpi(lines, configurationValue(
        path, "hardware_spi_device", XHAL_RPI5CAR_SPI_DEFAULT_DEVICE));
    inspectOptionalServices(lines, path);
    appendWarning(lines, "Safety", "no GPIO line, actuator, speaker, camera, microphone, SPI transfer, "
        "or model endpoint was activated");
    return lines;
}

} /* namespace xwalk::agent */
