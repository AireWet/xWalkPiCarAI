/******************************************************************************
 * @file        xHal_Rpi5CarTtsEspeakExampleLinux.h
 * @brief       Declares Linux process composition for the Espeak example.
 *
 * @details
 * Stores a deployment-selected Espeak executable and invokes it without a
 * shell using every exact source amplitude, speed, gap, pitch, and text value.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_TTS_ESPEAK_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_TTS_ESPEAK_EXAMPLE_LINUX_H

#include "xHal_Rpi5CarTtsEspeakExample.h"

namespace xwalk::hal::example
{

/** @brief Executes one configured Espeak playback request without a shell. */
class XWalkTtsEspeakExampleLinux final
{
private:

    /** @brief Owned non-empty executable name or path. */
    string executableName;

protected:

    /** @brief Resolves a callback context into its required Linux adapter. */
    static XWalkTtsEspeakExampleLinux& adapter(contextpointer context);
    /** @brief Executes one synchronous configured Espeak request. */
    static void speak(contextpointer context, uint8 amplitude, uint16 speed,
        uint16 gap, uint8 pitch, stringview text);

public:

    /**
     * @brief Stores one deployment-selected Espeak executable.
     * @param[in] executable Non-empty executable name or path.
     * @throws std::invalid_argument If `executable` is empty.
     */
    explicit XWalkTtsEspeakExampleLinux(stringview executable);

    XWalkTtsEspeakExampleLinux(const XWalkTtsEspeakExampleLinux&) = delete;
    XWalkTtsEspeakExampleLinux(XWalkTtsEspeakExampleLinux&&) = delete;
    XWalkTtsEspeakExampleLinux& operator=(const XWalkTtsEspeakExampleLinux&) = delete;
    XWalkTtsEspeakExampleLinux& operator=(XWalkTtsEspeakExampleLinux&&) = delete;

    /**
     * @brief Synthesizes and plays the configured `Hello world!` request once.
     * @warning Produces audible output through the executable's selected device.
     */
    void run();
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_TTS_ESPEAK_EXAMPLE_LINUX_H */
