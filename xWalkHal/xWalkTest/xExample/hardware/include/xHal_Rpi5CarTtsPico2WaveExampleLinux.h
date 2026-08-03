/******************************************************************************
 * @file        xHal_Rpi5CarTtsPico2WaveExampleLinux.h
 * @brief       Declares Linux composition for the Pico2Wave example.
 *
 * @details
 * Stores deployment-selected synthesis and playback executables and composes
 * shell-free temporary-WAV synthesis and synchronous playback.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_TTS_PICO2WAVE_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_TTS_PICO2WAVE_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTtsPico2WaveExample.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains Linux composition for ported Robot HAT examples.
 */
namespace xwalk::hal::example
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkTtsPico2WaveExampleLinux
 * @brief Executes one live Pico2Wave synthesis and WAV playback request.
 *
 * @details Owns executable names but no persistent descriptor or audio file.
 * Each synchronous request creates a private temporary WAV and removes it
 * after synthesis or playback completes.
 */
class XWalkTtsPico2WaveExampleLinux final
{
private:

    /**************************************************************************
     * Private data members
     **************************************************************************/

    /** @brief Owned non-empty Pico2Wave executable name or path. */
    string synthesisExecutableName;

    /** @brief Owned non-empty WAV playback executable name or path. */
    string playbackExecutableName;

protected:

    /**************************************************************************
     * Protected member functions
     **************************************************************************/

    /**
     * @brief Resolves a callback context into its required Linux adapter.
     * @param[in,out] context Non-null pointer to a live adapter.
     * @return Referenced live adapter.
     * @throws std::invalid_argument If `context` is null.
     */
    static XWalkTtsPico2WaveExampleLinux& adapter(contextpointer context);

    /**
     * @brief Synthesizes and plays one request through shell-free child processes.
     * @param[in,out] context Non-null pointer to a live adapter.
     * @param[in] language Non-empty Pico2Wave language identifier.
     * @param[in] text Non-empty speech text.
     * @throws std::runtime_error If temporary-file or child-process work fails.
     */
    static void speak(contextpointer context,
        stringview language, stringview text);

public:

    /**************************************************************************
     * Public constructors and destructor
     **************************************************************************/

    /**
     * @brief Stores deployment-selected synthesis and playback executables.
     * @param[in] synthesisExecutable Non-empty Pico2Wave executable name or path.
     * @param[in] playbackExecutable Non-empty WAV player executable name or path.
     * @throws std::invalid_argument If either executable is empty.
     */
    XWalkTtsPico2WaveExampleLinux(stringview synthesisExecutable,
        stringview playbackExecutable);

    /**************************************************************************
     * Public special member functions
     **************************************************************************/

    XWalkTtsPico2WaveExampleLinux(
        const XWalkTtsPico2WaveExampleLinux&) = delete;
    XWalkTtsPico2WaveExampleLinux(XWalkTtsPico2WaveExampleLinux&&) = delete;
    XWalkTtsPico2WaveExampleLinux& operator=(
        const XWalkTtsPico2WaveExampleLinux&) = delete;
    XWalkTtsPico2WaveExampleLinux& operator=(
        XWalkTtsPico2WaveExampleLinux&&) = delete;

    /**************************************************************************
     * Public member functions
     **************************************************************************/

    /**
     * @brief Synthesizes and plays the fixed Pico2Wave message once.
     * @warning Creates a temporary file and produces audible output.
     */
    void run();
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_TTS_PICO2WAVE_EXAMPLE_LINUX_H */
