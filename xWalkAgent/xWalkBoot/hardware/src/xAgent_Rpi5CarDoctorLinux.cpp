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
#include "xHal_Rpi5CarLinuxHeaders.h"

#include <fstream>

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
 * @brief Reads one flat configuration value without modifying its file.
 * @param[in] filePath Existing configuration path.
 * @param[in] key Key whose final occurrence is selected.
 * @param[in] fallback Value returned when the key or file is unavailable.
 * @return Owned normalized configuration value or `fallback`.
 */
hal::string XWalkDoctorLinux::configurationValue(hal::stringview filePath,
    hal::stringview key, hal::stringview fallback)
{
    std::ifstream stream{hal::string(filePath)};
    hal::string selected(fallback);
    hal::string line;
    while (std::getline(stream, line))
    {
        const hal::size separator = line.find('=');
        if (separator == hal::string::npos)
        {
            continue;
        }
        hal::string name = line.substr(0U, separator);
        hal::string value = line.substr(separator + 1U);
        const hal::size nameFirst = name.find_first_not_of(" \t");
        const hal::size nameLast = name.find_last_not_of(" \t");
        const hal::size valueFirst = value.find_first_not_of(" \t");
        const hal::size valueLast = value.find_last_not_of(" \t\r");
        if ((nameFirst == hal::string::npos) || (nameLast == hal::string::npos))
        {
            continue;
        }
        name = name.substr(nameFirst, (nameLast - nameFirst) + 1U);
        value = (valueFirst == hal::string::npos) ? hal::string{} :
            value.substr(valueFirst, (valueLast - valueFirst) + 1U);
        if (name == key)
        {
            selected = value;
        }
    }
    return selected;
}

/**
 * @brief Appends one consistently formatted report result.
 * @param[in,out] lines Report receiving one line.
 * @param[in] passed Whether the check passed.
 * @param[in] name Stable check name.
 * @param[in] detail Human-readable result detail.
 */
void XWalkDoctorLinux::appendResult(hal::stringvector& lines, hal::boolean passed,
    hal::stringview name, hal::stringview detail)
{
    lines.emplace_back(hal::string(passed ? "[PASS] " : "[FAIL] ") +
        hal::string(name) + ": " + hal::string(detail));
}

/**
 * @brief Appends one non-failing advisory result.
 * @param[in,out] lines Report receiving one line.
 * @param[in] name Stable check name.
 * @param[in] detail Human-readable advisory detail.
 */
void XWalkDoctorLinux::appendWarning(hal::stringvector& lines,
    hal::stringview name, hal::stringview detail)
{
    lines.emplace_back(hal::string("[WARN] ") + hal::string(name) + ": " + hal::string(detail));
}

/**
 * @brief Reports whether one regular path is readable.
 * @param[in] path Non-empty filesystem path.
 * @return `true` when the path exists and is readable; otherwise `false`.
 */
hal::boolean XWalkDoctorLinux::readablePath(hal::stringview path)
{
    return !path.empty() && (::access(hal::string(path).c_str(), R_OK) == 0);
}

/**
 * @brief Reports whether one named or absolute executable is available.
 * @param[in] executable Executable name or absolute path.
 * @return `true` when an executable regular file is found; otherwise `false`.
 */
hal::boolean XWalkDoctorLinux::executableAvailable(hal::stringview executable)
{
    if (executable.empty())
    {
        return false;
    }
    const hal::string owned(executable);
    if (owned.find('/') != hal::string::npos)
    {
        return ::access(owned.c_str(), X_OK) == 0;
    }
    const hal::fixedarray<hal::cstring, 4U> roots{
        "/usr/local/bin/", "/usr/bin/", "/bin/", "/opt/homebrew/bin/"};
    for (const hal::cstring root : roots)
    {
        const hal::string candidate = hal::string(root) + owned;
        if (::access(candidate.c_str(), X_OK) == 0)
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
hal::boolean XWalkDoctorLinux::libraryAvailable(hal::stringview libraryName)
{
    if (libraryName.empty())
    {
        return false;
    }
    void* const library = ::dlopen(hal::string(libraryName).c_str(), RTLD_LAZY | RTLD_LOCAL);
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
hal::string XWalkDoctorLinux::readProperty(hal::stringview path)
{
    std::ifstream stream(hal::string(path), std::ios::binary);
    hal::string value((std::istreambuf_iterator<char>(stream)),
        std::istreambuf_iterator<char>());
    if (!value.empty() && (value.back() == '\0'))
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
hal::boolean XWalkDoctorLinux::robotHatV5Detected(hal::stringview root)
{
    DIR* const directory = ::opendir(hal::string(root).c_str());
    if (directory == nullptr)
    {
        return false;
    }
    hal::boolean detected = false;
    for (dirent* entry = ::readdir(directory); entry != nullptr; entry = ::readdir(directory))
    {
        const hal::string name(entry->d_name);
        if (name.find(XHAL_RPI5CAR_DEVICE_HAT_NODE_MARKER) == hal::string::npos)
        {
            continue;
        }
        const hal::string uuidPath = hal::string(root) + "/" + name + "/" +
            XHAL_RPI5CAR_DEVICE_UUID_PROPERTY;
        if (readProperty(uuidPath) == XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID)
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
void XWalkDoctorLinux::inspectGpio(hal::stringvector& lines, hal::stringview device,
    hal::stringview expectedName, hal::stringview expectedLabel)
{
    const hal::int32 descriptor = ::open(hal::string(device).c_str(), O_RDONLY | O_CLOEXEC);
    if (descriptor < 0)
    {
        appendResult(lines, false, "GPIO", hal::string(device) + " is unavailable");
        return;
    }
    gpiochip_info information{};
    const hal::boolean inspected = ::ioctl(descriptor, GPIO_GET_CHIPINFO_IOCTL, &information) == 0;
    static_cast<void>(::close(descriptor));
    if (!inspected)
    {
        appendResult(lines, false, "GPIO", "chip metadata could not be read");
        return;
    }
    const hal::string name(information.name);
    const hal::string label(information.label);
    const hal::boolean countPassed = information.lines >= 28U;
    appendResult(lines, countPassed, "GPIO chip", name + ", label=" + label +
        ", lines=" + hal::common::uint32ToString(information.lines));
    if (expectedName.empty() || expectedLabel.empty())
    {
        appendResult(lines, false, "GPIO identity", "run provisioning to record chip name and label");
        return;
    }
    const hal::boolean matches = (name == expectedName) && (label == expectedLabel);
    appendResult(lines, matches, "GPIO identity", matches ? "configured identity matches" :
        "configured identity does not match the selected chip");
}

/**
 * @brief Reads Robot HAT firmware and battery data without constructing actuators.
 * @param[in,out] lines Report receiving I2C results.
 * @param[in] device Linux I2C character-device path.
 */
void XWalkDoctorLinux::inspectI2c(hal::stringvector& lines, hal::stringview device)
{
    const hal::int32 descriptor = ::open(hal::string(device).c_str(), O_RDWR | O_CLOEXEC);
    if (descriptor < 0)
    {
        appendResult(lines, false, "I2C", hal::string(device) + " is unavailable");
        appendResult(lines, false, "Robot HAT firmware", "not read");
        appendResult(lines, false, "Battery", "not read");
        return;
    }
    appendResult(lines, true, "I2C", hal::string(device) + " opened");
    const hal::fixedarray<hal::uint8, 2U> addresses{0x14U, 0x15U};
    hal::uint8 selectedAddress{};
    hal::fixedarray<hal::uint8, 3U> firmware{};
    for (const hal::uint8 address : addresses)
    {
        hal::uint8 firmwareRegister{0x05U};
        i2c_msg messages[2U]{};
        messages[0U].addr = address;
        messages[0U].len = 1U;
        messages[0U].buf = &firmwareRegister;
        messages[1U].addr = address;
        messages[1U].flags = I2C_M_RD;
        messages[1U].len = 3U;
        messages[1U].buf = firmware.data();
        i2c_rdwr_ioctl_data operation{messages, 2U};
        if (::ioctl(descriptor, I2C_RDWR, &operation) == 0)
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
    const hal::string firmwareText = hal::common::uint32ToString(firmware[0U]) + "." +
        hal::common::uint32ToString(firmware[1U]) + "." +
        hal::common::uint32ToString(firmware[2U]);
    appendResult(lines, true, "Robot HAT firmware", firmwareText);

    const hal::fixedarray<hal::uint8, 3U> adcCommand{0x13U, 0U, 0U};
    hal::fixedarray<hal::uint8, 2U> adcBytes{};
    const hal::boolean addressSelected = ::ioctl(descriptor, I2C_SLAVE, selectedAddress) == 0;
    const hal::boolean commandWritten = addressSelected &&
        (::write(descriptor, adcCommand.data(), adcCommand.size()) ==
            static_cast<ssize_t>(adcCommand.size()));
    const hal::boolean sampleRead = commandWritten &&
        (::read(descriptor, adcBytes.data(), adcBytes.size()) == static_cast<ssize_t>(adcBytes.size()));
    if (sampleRead)
    {
        const hal::uint16 count = static_cast<hal::uint16>(
            (static_cast<hal::uint16>(adcBytes[0U]) << 8U) | adcBytes[1U]);
        const hal::float64 countValue = static_cast<hal::float64>(count);
        const hal::float64 scaledCount = countValue * 3.3;
        const hal::float64 adcVoltage = scaledCount / 4095.0;
        const hal::float64 batteryVoltage = adcVoltage * 3.0;
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
void XWalkDoctorLinux::inspectSpi(hal::stringvector& lines, hal::stringview device)
{
    const hal::int32 descriptor = ::open(hal::string(device).c_str(), O_RDWR | O_CLOEXEC);
    const hal::boolean available = descriptor >= 0;
    if (available)
    {
        static_cast<void>(::close(descriptor));
    }
    appendResult(lines, available, "SPI", available ?
        hal::string(device) + " opened without transfer" : hal::string(device) + " is unavailable");
}

/**
 * @brief Checks passive camera, audio, model, executable, and library prerequisites.
 * @param[in,out] lines Report receiving optional-service results.
 * @param[in] configurationFilePath Existing deployment configuration path.
 */
void XWalkDoctorLinux::inspectOptionalServices(hal::stringvector& lines,
    hal::stringview configurationFilePath)
{
    const hal::string cameraConnection = configurationValue(
        configurationFilePath, "camera_connection", "csi");
    const hal::string cameraExecutable = configurationValue(configurationFilePath,
        cameraConnection == "usb" ? "camera_usb_executable" : "camera_csi_executable",
        cameraConnection == "usb" ? "ffmpeg" : "rpicam-still");
    const hal::string cameraDevice = configurationValue(
        configurationFilePath, "camera_usb_device", "/dev/video0");
    const hal::string csiDevice = configurationValue(
        configurationFilePath, "camera_csi_device", "/dev/media0");
    const hal::boolean cameraAvailable = executableAvailable(cameraExecutable) &&
        ((cameraConnection == "csi") ? readablePath(csiDevice) : readablePath(cameraDevice));
    appendResult(lines, cameraAvailable, "Camera", cameraAvailable ?
        "provider and device metadata available" : "provider executable or USB device is missing");

    const hal::string pcmMetadata = readProperty("/proc/asound/pcm");
    const hal::boolean microphoneAvailable = pcmMetadata.find("capture") != hal::string::npos;
    const hal::boolean speakerAvailable = pcmMetadata.find("playback") != hal::string::npos;
    const hal::boolean mixerAvailable = readablePath("/dev/snd/controlC0");
    appendResult(lines, microphoneAvailable, "Microphone", microphoneAvailable ?
        "ALSA capture metadata available; no stream was opened" : "no ALSA capture metadata found");
    appendResult(lines, speakerAvailable, "Speaker", speakerAvailable ?
        "ALSA playback metadata available; output was not enabled" : "no ALSA playback metadata found");
    appendResult(lines, mixerAvailable, "Mixer", mixerAvailable ?
        "ALSA control device available; mixer was not opened" : "ALSA control device unavailable");

    const hal::boolean alsaLibraryAvailable = libraryAvailable("libasound.so.2");
    const hal::boolean soundFileLibraryAvailable = libraryAvailable("libsndfile.so.1");
    appendResult(lines, alsaLibraryAvailable, "ALSA library",
        alsaLibraryAvailable ? "loadable" : "libasound.so.2 is missing");
    appendResult(lines, soundFileLibraryAvailable, "SoundFile library",
        soundFileLibraryAvailable ? "loadable" : "libsndfile.so.1 is missing");

    const hal::string voskLibrary = configurationValue(
        configurationFilePath, "voice_vosk_library", "libvosk.so");
    const hal::boolean voskAvailable = libraryAvailable(voskLibrary);
    appendResult(lines, voskAvailable, "Vosk library", voskAvailable ?
        "loadable" : voskLibrary + " is missing");
    const hal::string voskModel = configurationValue(
        configurationFilePath, "voice_vosk_model", "");
    appendResult(lines, readablePath(voskModel), "Voice model", readablePath(voskModel) ?
        voskModel : "configured Vosk model is missing");
    const hal::string espeak = configurationValue(
        configurationFilePath, "voice_espeak_executable", "espeak-ng");
    appendResult(lines, executableAvailable(espeak), "Espeak", executableAvailable(espeak) ?
        "executable available" : espeak + " is missing");
    const hal::string modelProvider = configurationValue(
        configurationFilePath, "voice_language_model_provider", "ollama");
    const hal::string modelEndpoint = configurationValue(
        configurationFilePath, "voice_language_model_endpoint",
        configurationValue(configurationFilePath, "voice_ollama_endpoint", ""));
    const hal::string modelName = configurationValue(
        configurationFilePath, "voice_language_model_model",
        configurationValue(configurationFilePath, "voice_ollama_model", ""));
    const hal::string modelApiKey = configurationValue(
        configurationFilePath, "voice_language_model_api_key", "");
    if (modelProvider == "ollama")
    {
        const hal::string ollamaManifest = configurationValue(
            configurationFilePath, "voice_ollama_model_manifest", "");
        const hal::boolean languageModelAvailable = executableAvailable("ollama") &&
            !modelEndpoint.empty() && !modelName.empty() && readablePath(ollamaManifest);
        appendResult(lines, languageModelAvailable, "Language model", languageModelAvailable ?
            "Ollama executable, endpoint, model, and manifest available; endpoint not contacted" :
            "Ollama executable, endpoint, model, or configured manifest is missing");
    }
    else
    {
        const hal::boolean supportedProvider = (modelProvider == "openai") ||
            (modelProvider == "chatgpt") || (modelProvider == "gemini") ||
            (modelProvider == "claude") || (modelProvider == "anthropic") ||
            (modelProvider == "openai_compatible");
        const hal::boolean languageModelAvailable = supportedProvider &&
            (modelEndpoint.substr(0U, 8U) == "https://") &&
            !modelName.empty() && !modelApiKey.empty();
        appendResult(lines, languageModelAvailable, "Language model", languageModelAvailable ?
            "Cloud provider, HTTPS endpoint, model, and API key configured; endpoint not contacted" :
            "Cloud provider configuration is incomplete or unsupported");
    }
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Builds one passive deployment report.
 * @param[in] configurationFilePath Flat deployment configuration path inspected without mutation.
 * @return Owned report lines prefixed with `[PASS]`, `[WARN]`, or `[FAIL]`.
 */
hal::stringvector XWalkDoctorLinux::inspect(hal::stringview configurationFilePath)
{
    hal::stringvector lines{"=== PiCar-X Passive Hardware Preflight ==="};
    const hal::string path(configurationFilePath);
    const hal::boolean readable = ::access(path.c_str(), R_OK) == 0;
    const hal::boolean writable = ::access(path.c_str(), W_OK) == 0;
    appendResult(lines, readable, "Configuration readable", readable ? path : "file is not readable");
    appendResult(lines, writable, "Configuration writable", writable ? path : "file is not writable");

    const hal::string deviceTreeRoot = configurationValue(
        path, "hardware_device_tree_root", XHAL_RPI5CAR_DEVICE_TREE_ROOT);
    const hal::boolean v5Detected = robotHatV5Detected(deviceTreeRoot);
    const hal::string profile = configurationValue(path, "hardware_board", "auto");
    hal::boolean profileValid = false;
    hal::string profileDetail;
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

    const hal::string i2cDevice = configurationValue(
        path, "hardware_i2c_device", XHAL_RPI5CAR_I2C_DEFAULT_DEVICE);
    inspectI2c(lines, i2cDevice);
    const hal::string gpioDevice = configurationValue(
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
