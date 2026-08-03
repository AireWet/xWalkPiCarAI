/******************************************************************************
 * @file        xHal_Rpi5CarTtsEdgeExampleLinux.h
 * @brief       Declares Linux process composition for the Edge TTS example.
 *
 * @details
 * Stores a deployment-selected Edge playback executable and invokes it without
 * a shell for the exact fixed cloud voice and speech message.
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

#ifndef XHAL_RPI5CAR_TTS_EDGE_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_TTS_EDGE_EXAMPLE_LINUX_H

#include "xHal_Rpi5CarTtsEdgeExample.h"

namespace xwalk::hal::example
{

/** @brief Executes one live Edge TTS playback request without a shell. */
class XWalkTtsEdgeExampleLinux final
{
private:

    /** @brief Owned non-empty executable name or path. */
    string executableName;

protected:

    /** @brief Resolves a callback context into its required Linux adapter. */
    static XWalkTtsEdgeExampleLinux& adapter(contextpointer context);
    /** @brief Executes one synchronous Edge cloud synthesis and playback. */
    static void speak(contextpointer context, stringview voice, stringview text);

public:

    /**
     * @brief Stores one deployment-selected Edge playback executable.
     * @param[in] executable Non-empty executable name or path.
     * @throws std::invalid_argument If `executable` is empty.
     */
    explicit XWalkTtsEdgeExampleLinux(stringview executable);

    XWalkTtsEdgeExampleLinux(const XWalkTtsEdgeExampleLinux&) = delete;
    XWalkTtsEdgeExampleLinux(XWalkTtsEdgeExampleLinux&&) = delete;
    XWalkTtsEdgeExampleLinux& operator=(const XWalkTtsEdgeExampleLinux&) = delete;
    XWalkTtsEdgeExampleLinux& operator=(XWalkTtsEdgeExampleLinux&&) = delete;

    /**
     * @brief Synthesizes and plays the fixed Edge TTS message once.
     * @warning Uses a remote cloud service and produces audible output.
     */
    void run();
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_TTS_EDGE_EXAMPLE_LINUX_H */
