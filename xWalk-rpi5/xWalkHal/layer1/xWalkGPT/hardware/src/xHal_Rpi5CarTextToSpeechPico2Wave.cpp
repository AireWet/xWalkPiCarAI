/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechPico2Wave.cpp
 * @brief       Implements shell-free Pico2Wave synthesis and WAV playback.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Pico2Wave Provider
 *
 * @author      Joxy John
 * @date        2026-08-05
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

#include "xHal_Rpi5CarTextToSpeechPico2Wave.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::hal @brief Contains xWalk hardware abstraction components.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Stores deployment-selected synthesis, playback, and language settings.
     * @param[in] executable Non-empty Pico2Wave executable name or path.
     * @param[in] playbackExecutable Non-empty WAV playback executable name or path.
     * @param[in] language Non-empty language identifier.
     * @throws std::invalid_argument If any setting is empty.
     */
    XWalkTextToSpeechPico2Wave::XWalkTextToSpeechPico2Wave(stringview executable,
                                                           stringview playbackExecutable,
                                                           stringview language)
        : executableName(executable), playbackExecutableName(playbackExecutable), languageName(language)
    {
        const hal::boolean ttsConfigurationInvalid =
            static_cast<hal::boolean>(executableName.empty() || playbackExecutableName.empty() || languageName.empty());
        if (ttsConfigurationInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Pico2Wave executable, playback executable, and language are required");
        }
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Returns the callback consumed by the shared speech coordinator.
     * @return Speech operation requiring this provider as context.
     */
    texttospeechspeakcallback XWalkTextToSpeechPico2Wave::callback() const noexcept
    {
        return &speak;
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Converts callback context into its required live provider.
     * @param[in,out] context Non-null pointer to a provider that outlives this
     * call.
     * @return Referenced provider.
     * @throws std::invalid_argument If `context` is null.
     */
    XWalkTextToSpeechPico2Wave& XWalkTextToSpeechPico2Wave::provider(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Pico2Wave provider context must not be null");
        }
        return *static_cast<XWalkTextToSpeechPico2Wave*>(context);
    }

    /**
     * @brief Generates and plays one non-empty bounded text value.
     * @param[in,out] context Non-null live provider context.
     * @param[in] text Text retained only during this synchronous call.
     * @throws std::out_of_range If text exceeds 4,096 bytes.
     */
    void XWalkTextToSpeechPico2Wave::speak(contextpointer context, stringview text)
    {
        const hal::boolean textEmpty = static_cast<hal::boolean>(text.empty());
        if (textEmpty)
        {
            return;
        }
        const hal::boolean textTooLarge = static_cast<hal::boolean>(text.size() > 4'096U);
        if (textTooLarge)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Pico2Wave text exceeds the bounded input length");
        }
        provider(context).execute(text);
    }

    /**
     * @brief Executes synthesis and playback without shell interpretation.
     * @param[in] text Non-empty bounded text retained until synthesis completes.
     * @throws std::runtime_error If file, process, synthesis, playback, or cleanup
     * fails.
     */
    void XWalkTextToSpeechPico2Wave::execute(stringview text) const
    {
        const string ownedText(text);
        char temporaryPath[]{"/tmp/xwalk-pico2wave-XXXXXX.wav"};
        const int32 descriptor = ::mkstemps(temporaryPath, 4);
        if (descriptor < 0)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Pico2Wave temporary file creation failed");
        }
        const hal::boolean descriptorDifferent = static_cast<hal::boolean>(::close(descriptor) != 0);
        if (descriptorDifferent)
        {
            static_cast<void>(::unlink(temporaryPath));
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Pico2Wave temporary file close failed");
        }

        const auto synthesisProcess = ::fork();
        if (synthesisProcess < 0)
        {
            static_cast<void>(::unlink(temporaryPath));
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Pico2Wave process creation failed");
        }
        if (synthesisProcess == 0)
        {
            const int32 nullDescriptor = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
            if (nullDescriptor >= 0)
            {
                static_cast<void>(::dup2(nullDescriptor, STDERR_FILENO));
                static_cast<void>(::close(nullDescriptor));
            }
            ::execlp(executableName.c_str(),
                     executableName.c_str(),
                     "-l",
                     languageName.c_str(),
                     "-w",
                     temporaryPath,
                     ownedText.c_str(),
                     static_cast<charpointer>(nullptr));
            ::_exit(127);
        }

        int32 synthesisStatus{};
        auto synthesisWait = ::waitpid(synthesisProcess, &synthesisStatus, 0);
        while ((synthesisWait < 0) && (errno == EINTR))
        {
            synthesisWait = ::waitpid(synthesisProcess, &synthesisStatus, 0);
        }
        const hal::boolean synthesisFailed = static_cast<hal::boolean>(
            (synthesisWait != synthesisProcess) || !WIFEXITED(synthesisStatus) || (WEXITSTATUS(synthesisStatus) != 0));
        if (synthesisFailed)
        {
            static_cast<void>(::unlink(temporaryPath));
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Pico2Wave synthesis executable failed");
        }

        const auto playbackProcess = ::fork();
        if (playbackProcess < 0)
        {
            static_cast<void>(::unlink(temporaryPath));
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Pico2Wave playback process creation failed");
        }
        if (playbackProcess == 0)
        {
            ::execlp(playbackExecutableName.c_str(),
                     playbackExecutableName.c_str(),
                     temporaryPath,
                     static_cast<charpointer>(nullptr));
            ::_exit(127);
        }

        int32 playbackStatus{};
        auto playbackWait = ::waitpid(playbackProcess, &playbackStatus, 0);
        while ((playbackWait < 0) && (errno == EINTR))
        {
            playbackWait = ::waitpid(playbackProcess, &playbackStatus, 0);
        }
        const int32 removeResult = ::unlink(temporaryPath);
        const hal::boolean playbackFailed =
            static_cast<hal::boolean>((removeResult != 0) || (playbackWait != playbackProcess) ||
                                      !WIFEXITED(playbackStatus) || (WEXITSTATUS(playbackStatus) != 0));
        if (playbackFailed)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Pico2Wave playback executable failed");
        }
    }

} /* namespace xwalk::hal */
