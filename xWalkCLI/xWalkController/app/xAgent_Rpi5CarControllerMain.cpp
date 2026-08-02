/******************************************************************************
 * @file        xAgent_Rpi5CarControllerMain.cpp
 * @brief       Provides the Raspberry Pi PiCar-X CLI entry point.
 *
 * @details
 * Parses process arguments, selects one xWalkBoot mode, and runs the CLI
 * through services owned for the complete command lifetime.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
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

#include "xAgent_Rpi5CarController.h"
#include "xAgent_Rpi5CarBootRpi.h"
#include "xAgent_Rpi5CarPicarxConfiguration.h"
#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

#include <iostream>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains application boot state and callbacks private to this translation unit. */
namespace
{

/******************************************************************************
 * Static global variables
 ******************************************************************************/

/**
 * @brief Indicates whether the foreground line-tracking loop may continue.
 *
 * @details
 * Initialized before signal handlers are installed and changed only by the
 * controlling thread or an asynchronous SIGINT/SIGTERM handler.
 */
volatile sig_atomic_t operationRequested = 1;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @struct XWalkControllerBootContext
 * @brief Carries validated process arguments into one backend boot attempt.
 */
struct XWalkControllerBootContext
{
    /**
     * @brief Non-owning pointer to command arguments that remains non-null until boot returns.
     */
    const XWalkHal::stringvector* commandArguments{nullptr};
    /** @brief Absolute packaged-data directory selected before boot. */
    XWalkHal::string resourceDirectory{};
};

/** @brief Carries audio and resource-path state through Controller callbacks. */
struct XWalkControllerApplicationContext
{
    /** @brief Nullable non-owning Music pointer provided by the selected boot graph. */
    xwalk::hal::XWalkMusic* music{nullptr};
    /** @brief Absolute packaged-data directory that remains valid during command execution. */
    XWalkHal::string resourceDirectory{};
};

/**
 * @brief Writes one CLI line to standard output.
 * @param[in] context Optional context; unused.
 * @param[in] line Text written synchronously followed by a newline.
 */
void outputLine(XWalkHal::contextpointer context, XWalkHal::stringview line)
{
    static_cast<void>(context);
    std::cout << line << std::endl;
}

/**
 * @brief Writes one prompt and reads one line from standard input.
 * @param[in] context Optional context; unused.
 * @param[in] prompt Prompt text written without a newline.
 * @return Owned response line, or `skip` when input reaches end-of-file.
 */
XWalkHal::string inputLine(XWalkHal::contextpointer context, XWalkHal::stringview prompt)
{
    static_cast<void>(context);
    std::cout << prompt << std::flush;
    XWalkHal::string response;
    if (!std::getline(std::cin, response))
    {
        return "skip";
    }
    return response;
}

/**
 * @brief Suspends the CLI on the calling thread.
 * @param[in] context Optional context; unused.
 * @param[in] durationMs Requested duration in milliseconds.
 */
void delayMilliseconds(XWalkHal::contextpointer context, XWalkHal::uint32 durationMs)
{
    static_cast<void>(context);
    xwalk::hal::common::sleepMilliseconds(durationMs);
}

/**
 * @brief Requests graceful shutdown of the active operation from a process signal.
 * @param[in] signalNumber Delivered signal number; ignored after dispatch.
 */
void requestOperationStop(int signalNumber)
{
    static_cast<void>(signalNumber);
    operationRequested = 0;
}

/**
 * @brief Reports whether the active operation may perform another bounded step.
 * @param[in] context Optional context; unused.
 * @return `true` until SIGINT or SIGTERM requests shutdown.
 */
XWalkHal::boolean continueOperation(XWalkHal::contextpointer context)
{
    static_cast<void>(context);
    return operationRequested != 0;
}

/**
 * @brief Removes process-wide deployment options before command parsing.
 * @param[in,out] arguments Process arguments excluding the executable name.
 * @param[out] configFilePath Absolute mutable deployment configuration path.
 * @param[out] resourceDirectory Absolute packaged-resource directory.
 * @return `true` when every supplied global option is complete and absolute.
 */
XWalkHal::boolean parseGlobalOptions(XWalkHal::stringvector& arguments,
    XWalkHal::string& configFilePath, XWalkHal::string& resourceDirectory)
{
    while (!arguments.empty())
    {
        XWalkHal::size consumed{};
        XWalkHal::string value;
        if ((arguments[0U] == "--deployment-config") ||
            (arguments[0U] == "--resource-directory"))
        {
            if (arguments.size() < 2U)
            {
                return false;
            }
            value = arguments[1U];
            consumed = 2U;
        }
        else if (arguments[0U].rfind("--deployment-config=", 0U) == 0U)
        {
            value = arguments[0U].substr(20U);
            consumed = 1U;
        }
        else if (arguments[0U].rfind("--resource-directory=", 0U) == 0U)
        {
            value = arguments[0U].substr(21U);
            consumed = 1U;
        }
        else
        {
            break;
        }
        if (value.empty() || !XWalkHal::filesystempath(value).is_absolute())
        {
            return false;
        }
        if (arguments[0U].rfind("--deployment-config", 0U) == 0U)
        {
            configFilePath = value;
        }
        else
        {
            resourceDirectory = value;
        }
        arguments.erase(arguments.begin(), arguments.begin() +
            static_cast<XWalkHal::stringvector::difference_type>(consumed));
    }
    return true;
}

/**
 * @brief Executes one CLI audio operation through a caller-owned Music object.
 *
 * @details
 * Sound-effect and music-file operations are synchronous because the one-shot
 * CLI must retain its ALSA composition until playback completes.
 *
 * @param[in,out] context Non-null pointer to the Music object that outlives this call.
 * @param[in] operation Requested audio operation.
 * @param[in] filePath File path for sound or music playback.
 * @param[in] volumePercent Optional volume in the inclusive range zero through one hundred percent.
 * @return `true` after the Music backend accepts and completes the operation.
 */
XWalkHal::boolean performSound(XWalkHal::contextpointer context,
    xwalk::agent::XWalkSoundOperation operation, XWalkHal::stringview filePath,
    XWalkHal::optionalfloat64 volumePercent)
{
    XWalkControllerApplicationContext& applicationContext =
        *static_cast<XWalkControllerApplicationContext*>(context);
    if (applicationContext.music == nullptr)
    {
        return false;
    }
    xwalk::hal::XWalkMusic& music = *applicationContext.music;
    XWalkHal::string resolvedFilePath(filePath);
    if ((operation == xwalk::agent::XWalkSoundOperation::Play) ||
        (operation == xwalk::agent::XWalkSoundOperation::Music))
    {
        const XWalkHal::filesystempath resolved = xwalk::hal::resolveResourcePath(
            applicationContext.resourceDirectory, XWalkHal::filesystempath(filePath));
        resolvedFilePath = resolved.string();
        if (!xwalk::hal::isReadableRegularFile(resolved))
        {
            std::cerr << "Unreadable sound resource: " << resolvedFilePath << std::endl;
            return false;
        }
    }
    switch (operation)
    {
        case xwalk::agent::XWalkSoundOperation::Play:
        case xwalk::agent::XWalkSoundOperation::Music:
            music.soundPlay(resolvedFilePath, volumePercent);
            break;
        case xwalk::agent::XWalkSoundOperation::Volume:
            if (!volumePercent.has_value())
            {
                return false;
            }
            music.musicSetVolume(*volumePercent);
            break;
        case xwalk::agent::XWalkSoundOperation::Stop:
            music.musicStop();
            break;
    }
    return true;
}

/**
 * @brief Selects the minimum boot graph required by one parsed command group.
 * @param[in] commandArguments Complete process command arguments.
 * @return Command-specific boot mode, or Base when no specialized service is required.
 */
xwalk::agent::XWalkBootMode selectBootMode(
    const XWalkHal::stringvector& commandArguments) noexcept
{
    if (!commandArguments.empty())
    {
        if (commandArguments[0U] == "line-track")
        {
            return xwalk::agent::XWalkBootMode::LineTracking;
        }
        if (commandArguments[0U] == "doctor")
        {
            return xwalk::agent::XWalkBootMode::Doctor;
        }
        if (commandArguments[0U] == "self-drive")
        {
            return xwalk::agent::XWalkBootMode::SelfDrive;
        }
        if (commandArguments[0U] == "sound")
        {
            return xwalk::agent::XWalkBootMode::Sound;
        }
        if (commandArguments[0U] == "voice-chat")
        {
            return xwalk::agent::XWalkBootMode::VoiceChat;
        }
        if (commandArguments[0U] == "voice-active-car")
        {
            return xwalk::agent::XWalkBootMode::VoiceActiveCar;
        }
        if (commandArguments[0U] == "voice-active-car-gpt")
        {
            return xwalk::agent::XWalkBootMode::VoiceActiveCarGpt;
        }
        if (commandArguments[0U] == "voice-controlled-car")
        {
            return xwalk::agent::XWalkBootMode::VoiceControlledCar;
        }
        if (commandArguments[0U] == "voice-prompt-car")
        {
            return xwalk::agent::XWalkBootMode::VoicePromptCar;
        }
        if (commandArguments[0U] == "spi")
        {
            return xwalk::agent::XWalkBootMode::SpiTransfer;
        }
    }
    return xwalk::agent::XWalkBootMode::Base;
}

/**
 * @brief Executes one CLI command through services retained by xWalkBoot.
 * @param[in,out] context Non-null controller boot context.
 * @param[in,out] services Command-specific services owned by xWalkBoot.
 * @return CLI status after command completion.
 */
XWalkHal::int32 runController(XWalkHal::contextpointer context,
    xwalk::agent::XWalkBootServices& services)
{
    const XWalkControllerBootContext& bootContext =
        *static_cast<XWalkControllerBootContext*>(context);
    const XWalkHal::stringvector& commandArguments = *bootContext.commandArguments;
    const xwalk::agent::XWalkControllerCallbacks callbacks{&outputLine, &inputLine,
        &delayMilliseconds, &continueOperation, &performSound};
    XWalkControllerApplicationContext applicationContext{
        services.music, bootContext.resourceDirectory};
    if (services.doctorLines != nullptr)
    {
        xwalk::agent::XWalkController cli(*services.doctorLines, &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.spiTransfer != nullptr)
    {
        xwalk::agent::XWalkController cli(*services.spiTransfer, &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.picarx == nullptr)
    {
        XHAL_THROW_LOGIC_ERROR("xWalkBoot did not provide the base PiCar-X service");
    }
    if (services.selfDrive != nullptr)
    {
        services.selfDrive->setCancellation(&applicationContext, &continueOperation);
    }
    if (services.lineTracking != nullptr)
    {
        xwalk::agent::XWalkController cli(*services.picarx, *services.lineTracking,
            &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.voiceAssistant != nullptr)
    {
        if (!commandArguments.empty() && (commandArguments[0U] == "voice-chat"))
        {
            const xwalk::agent::XWalkLocalVoiceChatbotCallbacks voiceCallbacks{
                &outputLine, &continueOperation, &delayMilliseconds};
            xwalk::agent::XWalkLocalVoiceChatbot chatbot(
                *services.voiceAssistant, nullptr, voiceCallbacks);
            xwalk::agent::XWalkController cli(
                *services.picarx, chatbot, &applicationContext, callbacks);
            return cli.run(commandArguments);
        }
        if ((services.selfDrive == nullptr) || (services.voiceStatusLed == nullptr) ||
            (services.cameraCapture == nullptr))
        {
            XHAL_THROW_LOGIC_ERROR(
                "xWalkBoot did not provide the voice-active-car hardware graph");
        }
        const xwalk::agent::XWalkVoiceActiveCarCallbacks voiceCallbacks{
            &outputLine, &continueOperation, &delayMilliseconds,
            &xwalk::agent::XWalkCameraCapture::captureImage};
        xwalk::agent::XWalkVoiceActiveCarConfiguration voiceConfiguration{};
        if (!commandArguments.empty() &&
            (commandArguments[0U] == "voice-active-car-gpt"))
        {
            voiceConfiguration =
                xwalk::agent::XWalkVoiceActiveCarGpt::carConfiguration();
        }
        voiceConfiguration.withImage = true;
        xwalk::agent::XWalkVoiceActiveCar voiceActiveCar(
            *services.picarx, *services.selfDrive, *services.voiceAssistant,
            *services.voiceStatusLed, services.cameraCapture,
            voiceCallbacks, voiceConfiguration);
        xwalk::agent::XWalkController cli(
            *services.picarx, voiceActiveCar, &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.selfDrive != nullptr)
    {
        xwalk::agent::XWalkController cli(*services.picarx, *services.selfDrive,
            &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.localVoiceChatbot != nullptr)
    {
        xwalk::agent::XWalkController cli(*services.picarx,
            *services.localVoiceChatbot, &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.voiceActiveCar != nullptr)
    {
        xwalk::agent::XWalkController cli(*services.picarx,
            *services.voiceActiveCar, &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.voiceControlledCar != nullptr)
    {
        xwalk::agent::XWalkController cli(*services.picarx,
            *services.voiceControlledCar, &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.speechToText != nullptr)
    {
        const xwalk::agent::XWalkVoiceActiveCarCallbacks voiceCallbacks{
            &outputLine, &continueOperation, &delayMilliseconds, nullptr};
        xwalk::agent::XWalkVoiceControlledCar voiceControlledCar(
            *services.picarx, *services.speechToText, nullptr, voiceCallbacks);
        xwalk::agent::XWalkController cli(*services.picarx,
            voiceControlledCar, &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.voicePromptCar != nullptr)
    {
        xwalk::agent::XWalkController cli(*services.picarx,
            *services.voicePromptCar, &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    if (services.textToSpeech != nullptr)
    {
        const xwalk::agent::XWalkVoiceActiveCarCallbacks voiceCallbacks{
            &outputLine, &continueOperation, &delayMilliseconds, nullptr};
        xwalk::agent::XWalkVoicePromptCar voicePromptCar(
            *services.picarx, *services.textToSpeech, nullptr, voiceCallbacks);
        xwalk::agent::XWalkController cli(*services.picarx,
            voicePromptCar, &applicationContext, callbacks);
        return cli.run(commandArguments);
    }
    xwalk::agent::XWalkController cli(*services.picarx, &applicationContext, callbacks);
    return cli.run(commandArguments);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Parses process arguments and performs one guarded RPi backend boot.
 * @param[in] argumentCount Number of process arguments including the executable name.
 * @param[in] arguments Non-owning process argument array.
 * @return CLI or help status after successful completion.
 * @warning Non-help commands may claim physical I2C, GPIO, motor, servo, and audio resources.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    XWalkHal::stringvector commandArguments;
    for (XWalkHal::int32 index = 1; index < argumentCount; ++index)
    {
        commandArguments.emplace_back(arguments[index]);
    }

    XWalkHal::string configurationFilePath = XWALK_PICARX_CONFIG_FILE;
    XWalkHal::string resourceDirectory = XWALK_RUNTIME_DATA_DIRECTORY;
    if (!parseGlobalOptions(commandArguments, configurationFilePath, resourceDirectory))
    {
        std::cerr << "Global options require absolute non-empty paths" << std::endl;
        return 2;
    }

    if ((commandArguments.size() == 1U) &&
        ((commandArguments[0U] == "-h") || (commandArguments[0U] == "--help") ||
         (commandArguments[0U] == "help")))
    {
        std::cout << xwalk::agent::XWalkController::usage() << std::endl;
        return 0;
    }
    if (!xwalk::hal::isReadableRegularFile(configurationFilePath))
    {
        std::cerr << "Unreadable deployment configuration: " << configurationFilePath << std::endl;
        return 2;
    }

    operationRequested = 1;
    static_cast<void>(::signal(SIGINT, &requestOperationStop));
    static_cast<void>(::signal(SIGTERM, &requestOperationStop));
    XWalkControllerBootContext bootContext{&commandArguments, resourceDirectory};
    xwalk::agent::XWalkBootRpi boot(selectBootMode(commandArguments),
        configurationFilePath);
    return boot.run(&bootContext, &runController);
}
