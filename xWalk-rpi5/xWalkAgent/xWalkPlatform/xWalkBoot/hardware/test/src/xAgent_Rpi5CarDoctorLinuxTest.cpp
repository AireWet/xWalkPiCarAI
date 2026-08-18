/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorLinuxTest.cpp
 * @brief       Verifies host-safe Linux Doctor executable discovery.
 *
 * @details
 * Uses one temporary executable to verify absolute and user-local PATH lookup
 * without opening any hardware device.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi Doctor Test
 * @author      Joxy John
 * @date        2026-08-18
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarDoctorLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include <cstdlib>
#include <iostream>
#include <limits.h>

namespace xwalk::agent::test::doctor
{

    /** @brief Returns a failing test status after writing one diagnostic. */
    int fail(const char* message)
    {
        std::cerr << message << '\n';
        return 1;
    }

} /* namespace xwalk::agent::test::doctor */

/**
 * @brief Verifies absolute lookup, PATH lookup, and ignored empty PATH entries.
 * @return Zero on success; otherwise non-zero.
 */
int main()
{
    char directoryTemplate[] = "/tmp/xwalk-doctor-path.XXXXXX";
    char* const directory = ::mkdtemp(directoryTemplate);
    if (directory == nullptr)
    {
        return xwalk::agent::test::doctor::fail("mkdtemp failed");
    }
    const agent::string executableName("xwalk-doctor-path-fixture");
    const agent::string executablePath = agent::string(directory) + "/" + executableName;
    const agent::int32 descriptor = ::open(executablePath.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0700);
    if (descriptor < 0)
    {
        static_cast<void>(::rmdir(directory));
        return xwalk::agent::test::doctor::fail("fixture creation failed");
    }
    constexpr char fixture[] = "#!/bin/sh\nexit 0\n";
    const agent::boolean fixtureWritten =
        ::write(descriptor, fixture, sizeof(fixture) - 1U) == static_cast<ssize_t>(sizeof(fixture) - 1U);
    static_cast<void>(::close(descriptor));

    char originalDirectory[PATH_MAX]{};
    const agent::boolean directoryRecorded = ::getcwd(originalDirectory, sizeof(originalDirectory)) != nullptr;
    const agent::cstring originalPathValue = std::getenv("PATH");
    const agent::string originalPath =
        originalPathValue == nullptr ? agent::string() : agent::string(originalPathValue);
    const agent::boolean absoluteFound = xwalk::agent::XWalkDoctorLinux::executableAvailable(executablePath);
    const agent::boolean changedDirectory = ::chdir(directory) == 0;
    const agent::boolean emptyPathSet = ::setenv("PATH", ":", 1) == 0;
    const agent::boolean emptyEntryIgnored = !xwalk::agent::XWalkDoctorLinux::executableAvailable(executableName);
    const agent::boolean localPathSet = ::setenv("PATH", directory, 1) == 0;
    const agent::boolean localPathFound = xwalk::agent::XWalkDoctorLinux::executableAvailable(executableName);

    if (directoryRecorded)
    {
        static_cast<void>(::chdir(originalDirectory));
    }
    if (originalPathValue == nullptr)
    {
        static_cast<void>(::unsetenv("PATH"));
    }
    else
    {
        static_cast<void>(::setenv("PATH", originalPath.c_str(), 1));
    }
    static_cast<void>(::unlink(executablePath.c_str()));
    static_cast<void>(::rmdir(directory));

    if (!fixtureWritten || !directoryRecorded || !changedDirectory || !emptyPathSet || !localPathSet)
    {
        return xwalk::agent::test::doctor::fail("test environment setup failed");
    }
    if (!absoluteFound)
    {
        return xwalk::agent::test::doctor::fail("absolute executable was not found");
    }
    if (!emptyEntryIgnored)
    {
        return xwalk::agent::test::doctor::fail("an empty PATH entry was treated as the working directory");
    }
    if (!localPathFound)
    {
        return xwalk::agent::test::doctor::fail("user-local PATH executable was not found");
    }
    return 0;
}
