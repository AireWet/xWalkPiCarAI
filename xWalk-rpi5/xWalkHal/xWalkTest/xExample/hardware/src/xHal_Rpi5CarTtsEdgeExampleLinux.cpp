/******************************************************************************
 * @file        xHal_Rpi5CarTtsEdgeExampleLinux.cpp
 * @brief       Implements Linux process composition for the Edge TTS example.
 *
 * @details
 * Forks one bounded child, supplies voice and text as distinct arguments to a
 * deployment-selected executable, waits synchronously, and validates status.
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

#include "xHal_Rpi5CarTtsEdgeExampleLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::example
{

    /** @brief Stores and validates the deployment-selected executable. */
    XWalkTtsEdgeExampleLinux::XWalkTtsEdgeExampleLinux(stringview executable) : executableName(executable)
    {
        const hal::boolean executableNameEmpty = static_cast<hal::boolean>(executableName.empty());
        if (executableNameEmpty)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Edge TTS playback executable is required");
        }
    }

    /** @brief Delivers the fixed request through the live process adapter. */
    void XWalkTtsEdgeExampleLinux::run()
    {
        XWalkTtsEdgeExample example(this, &speak);
        example.run();
    }

    /** @brief Resolves one non-null Linux callback context. */
    XWalkTtsEdgeExampleLinux& XWalkTtsEdgeExampleLinux::adapter(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Edge TTS Linux context must not be null");
        }
        return *static_cast<XWalkTtsEdgeExampleLinux*>(context);
    }

    /** @brief Executes and waits for one shell-free Edge playback process. */
    void XWalkTtsEdgeExampleLinux::speak(contextpointer context, stringview voice, stringview text)
    {
        XWalkTtsEdgeExampleLinux& self = adapter(context);
        const string ownedVoice(voice);
        const string ownedText(text);
        const auto childProcess = ::fork();
        if (childProcess < 0)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Edge TTS process creation failed");
        }
        if (childProcess == 0)
        {
            ::execlp(self.executableName.c_str(),
                     self.executableName.c_str(),
                     "--voice",
                     ownedVoice.c_str(),
                     "--text",
                     ownedText.c_str(),
                     static_cast<charpointer>(nullptr));
            ::_exit(127);
        }

        int32 processStatus{};
        auto waitResult = ::waitpid(childProcess, &processStatus, 0);
        while ((waitResult < 0) && (errno == EINTR))
        {
            waitResult = ::waitpid(childProcess, &processStatus, 0);
        }
        const hal::boolean waitResultChildProcessProcessStatusInvalid = static_cast<hal::boolean>(
            (waitResult != childProcess) || !WIFEXITED(processStatus) || (WEXITSTATUS(processStatus) != 0));
        if (waitResultChildProcessProcessStatusInvalid)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Edge TTS playback executable failed");
        }
    }

} /* namespace xwalk::hal::example */
