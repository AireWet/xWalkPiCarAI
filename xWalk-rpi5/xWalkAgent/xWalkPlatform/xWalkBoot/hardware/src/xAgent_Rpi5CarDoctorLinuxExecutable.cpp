/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorLinuxExecutable.cpp
 * @brief       Implements Linux Doctor executable discovery.
 *
 * @details
 * Resolves absolute executable paths and named executables from non-empty PATH
 * entries while retaining bounded fixed-directory fallback behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi Doctor
 *
 * @author      Joxy John
 * @date        2026-08-19
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
     * @brief Reports whether one path names an executable regular file.
     * @param[in] executable Non-empty filesystem path.
     * @return `true` when the path names an executable regular file; otherwise `false`.
     */
    agent::boolean XWalkDoctorLinux::executablePathAvailable(agent::stringview executable)
    {
        struct stat status = {};
        const agent::string owned(executable);
        return (::stat(owned.c_str(), &status) == 0) && S_ISREG(status.st_mode) && (::access(owned.c_str(), X_OK) == 0);
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Reports whether one named or absolute executable is available.
     * @param[in] executable Executable name or absolute path.
     * @return `true` when an executable regular file is found; otherwise `false`.
     */
    agent::boolean XWalkDoctorLinux::executableAvailable(agent::stringview executable)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .074, "Doctor checking one executable prerequisite");
        const agent::boolean executableEmpty = static_cast<agent::boolean>(executable.empty());
        if (executableEmpty)
        {
            return false;
        }
        const agent::string owned(executable);
        if (owned.front() == '/')
        {
            return executablePathAvailable(owned);
        }
        if (owned.find('/') != agent::string::npos)
        {
            return false;
        }
        const agent::cstring pathEnvironment = std::getenv("PATH");
        if (pathEnvironment != nullptr)
        {
            const agent::string path(pathEnvironment);
            agent::size start{};
            while (start <= path.size())
            {
                const agent::size end = path.find(':', start);
                const agent::size length = end == agent::string::npos ? path.size() - start : end - start;
                if (length != 0U)
                {
                    const agent::string candidate = path.substr(start, length) + "/" + owned;
                    if (executablePathAvailable(candidate))
                    {
                        return true;
                    }
                }
                if (end == agent::string::npos)
                {
                    break;
                }
                start = end + 1U;
            }
        }
        const agent::fixedarray<agent::cstring, 4U> roots{
            "/usr/local/bin/", "/usr/bin/", "/bin/", "/opt/homebrew/bin/"};
        for (const agent::cstring root : roots)
        {
            const agent::string candidate = agent::string(root) + owned;
            const agent::boolean candidateCStrMatched = executablePathAvailable(candidate);
            if (candidateCStrMatched)
            {
                return true;
            }
        }
        return false;
    }

} /* namespace xwalk::agent */
