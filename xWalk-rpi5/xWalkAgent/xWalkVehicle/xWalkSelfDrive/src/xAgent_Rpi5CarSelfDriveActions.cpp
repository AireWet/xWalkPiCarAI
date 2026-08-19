/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveActions.cpp
 * @brief       Implements self-drive action selection and simple presets.
 *
 * @details
 * Maps the upstream action and sound names to their corresponding gesture,
 * movement, or asynchronous sound operation.
 *
 * @project     xWalk Firmware
 * @module      xWalkSelfDrive
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
#include "xAgent_Rpi5CarSelfDrive.h"

#include "xHal_Rpi5CarFileFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains preset values private to this translation unit.
 */
namespace
{

    constexpr xwalk::agent::float64 PRESET_DRIVE_SPEED_PERCENT = 5.0;
    constexpr xwalk::agent::uint32 PRESET_DRIVE_DURATION_MS = 1'000U;
    constexpr xwalk::agent::cstring HORN_SOUND_FILE = "car-double-horn.wav";
    constexpr xwalk::agent::cstring ENGINE_SOUND_FILE = "car-start-engine.wav";
    constexpr xwalk::agent::float64 BACKGROUND_MUSIC_VOLUME_PERCENT = 20.0;

} /* namespace */

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
     * @brief Returns whether text names one supported movement, stop, or sound action.
     *
     * @param[in] action
     * Exact lowercase action name.
     *
     * @return
     * `true` for a preset action-map, serialized stop, or sound-map name; otherwise `false`.
     */
    agent::boolean XWalkSelfDrive::isActionSupported(agent::stringview action) noexcept
    {
        return (action == "shake head") || (action == "nod") || (action == "wave hands") || (action == "resist") ||
               (action == "act cute") || (action == "rub hands") || (action == "think") || (action == "twist body") ||
               (action == "celebrate") || (action == "depressed") || (action == "forward") || (action == "backward") ||
               (action == "stop") || (action == "honking") || (action == "start engine") ||
               (action == "play background music") || (action == "stop background music");
    }

    /**
     * @brief Drives forward briefly and then stops.
     *
     * @post
     * Both drive motors are stopped.
     */
    void XWalkSelfDrive::forward()
    {
        picarxObject->forward(PRESET_DRIVE_SPEED_PERCENT);
        delay(PRESET_DRIVE_DURATION_MS);
        picarxObject->stop();
    }

    /**
     * @brief Drives backward briefly and then stops.
     *
     * @post
     * Both drive motors are stopped.
     */
    void XWalkSelfDrive::backward()
    {
        picarxObject->backward(PRESET_DRIVE_SPEED_PERCENT);
        delay(PRESET_DRIVE_DURATION_MS);
        picarxObject->stop();
    }

    /**
     * @brief Starts the upstream horn sound asynchronously.
     *
     * @post
     * The music backend has received the horn path at one hundred percent volume.
     */
    agent::boolean XWalkSelfDrive::honking()
    {
        const agent::filesystempath soundPath = hal::resolveResourcePath(soundDirectoryValue, HORN_SOUND_FILE);
        const agent::boolean readableRegularFileNotMatched =
            static_cast<agent::boolean>(!hal::isReadableRegularFile(soundPath));
        if (readableRegularFileNotMatched)
        {
            return false;
        }
        musicObject->soundPlayBackground(soundPath.string(), 100.0);
        return true;
    }

    /**
     * @brief Starts the upstream engine sound asynchronously.
     *
     * @post
     * The music backend has received the engine path at fifty percent volume.
     */
    agent::boolean XWalkSelfDrive::startEngine()
    {
        const agent::filesystempath soundPath = hal::resolveResourcePath(soundDirectoryValue, ENGINE_SOUND_FILE);
        const agent::boolean soundFileUnreadable = static_cast<agent::boolean>(!hal::isReadableRegularFile(soundPath));
        if (soundFileUnreadable)
        {
            return false;
        }
        musicObject->soundPlayBackground(soundPath.string(), 50.0);
        return true;
    }

    /**
     * @brief Starts the configured background song after validating its resource path.
     * @return `true` after playback starts; otherwise `false` when the file is unavailable.
     * @post Successful playback uses the bounded configured resource at twenty-percent volume.
     */
    agent::boolean XWalkSelfDrive::playBackgroundMusic()
    {
        const agent::filesystempath musicPath =
            hal::resolveResourcePath(musicDirectoryValue, backgroundMusicFilenameValue);
        const agent::boolean musicFileUnreadable = static_cast<agent::boolean>(!hal::isReadableRegularFile(musicPath));
        if (musicFileUnreadable)
        {
            return false;
        }
        musicObject->musicPlay(musicPath.string(), 1, 0.0, BACKGROUND_MUSIC_VOLUME_PERCENT);
        backgroundMusicPlayingValue = true;
        return true;
    }

    /**
     * @brief Stops background music previously started by this coordinator.
     * @return `true`; an already stopped stream is treated as a completed idempotent action.
     * @post `backgroundMusicPlayingValue` is `false`.
     */
    agent::boolean XWalkSelfDrive::stopBackgroundMusic()
    {
        if (backgroundMusicPlayingValue)
        {
            musicObject->musicStop();
            backgroundMusicPlayingValue = false;
        }
        return true;
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Executes one supported preset action synchronously.
     *
     * @param[in] action
     * Exact lowercase preset action name, including the serialized `stop` action.
     *
     * @return
     * `true` when the action was recognized and completed; otherwise `false`.
     */
    agent::boolean XWalkSelfDrive::doAction(agent::stringview action)
    {
        if (action == "shake head")
        {
            shakeHead();
        }
        else if (action == "nod")
        {
            nod();
        }
        else if (action == "wave hands")
        {
            waveHands();
        }
        else if (action == "resist")
        {
            resist();
        }
        else if (action == "act cute")
        {
            actCute();
        }
        else if (action == "rub hands")
        {
            rubHands();
        }
        else if (action == "think")
        {
            think();
        }
        else if (action == "twist body")
        {
            twistBody();
        }
        else if (action == "celebrate")
        {
            celebrate();
        }
        else if (action == "depressed")
        {
            depressed();
        }
        else if (action == "forward")
        {
            forward();
        }
        else if (action == "backward")
        {
            backward();
        }
        else if (action == "stop")
        {
            picarxObject->stop();
        }
        else if (action == "honking")
        {
            return honking();
        }
        else if (action == "start engine")
        {
            return startEngine();
        }
        else if (action == "play background music")
        {
            return playBackgroundMusic();
        }
        else if (action == "stop background music")
        {
            return stopBackgroundMusic();
        }
        else
        {
            return false;
        }
        return true;
    }

} /* namespace xwalk::agent */
