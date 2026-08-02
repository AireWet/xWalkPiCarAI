/******************************************************************************
 * @file        xHal_Rpi5CarUtilsLinuxTest.cpp
 * @brief       Verifies the Linux utility backend with safe host operations.
 *
 * @details
 * Exercises callback composition, ANSI output, process output collection,
 * executable lookup, unavailable-interface lookup, username lookup, and stderr RAII.
 * Mixer state and other physical Raspberry Pi resources are not accessed.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils Linux Software Test
 *
 * @author      Joxy John
 * @date        2026-08-01
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

#include "xHal_Rpi5CarUtilsLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains safe Linux-backend test scenarios private to this translation unit. */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Test function definitions
 ******************************************************************************/

/** @brief Reads all currently available bytes through a closed pipe writer. */
string readPipe(int32 descriptor)
{
    string contents;
    fixedarray<char, 256U> buffer{};
    while (true)
    {
        const auto readResult = ::read(descriptor, buffer.data(), buffer.size());
        if (readResult > 0)
        {
            contents.append(buffer.data(), static_cast<size>(readResult));
        }
        else
        {
            break;
        }
    }
    return contents;
}

/** @brief Verifies callback-bound ANSI output without writing to the test terminal. */
void testOutput()
{
    fixedarray<int32, 2U> outputPipe{};
    assert(::pipe(outputPipe.data()) == 0);
    const int32 savedOutput = ::dup(STDOUT_FILENO);
    assert(savedOutput >= 0);
    assert(::dup2(outputPipe[1U], STDOUT_FILENO) >= 0);
    assert(::close(outputPipe[1U]) == 0);

    XWalkUtilsLinux backend;
    XWalkUtils utilities(&backend, backend.utilityCallbacks());
    utilities.info("linux", "!", true);

    assert(::dup2(savedOutput, STDOUT_FILENO) >= 0);
    assert(::close(savedOutput) == 0);
    const string output = readPipe(outputPipe[0U]);
    assert(::close(outputPipe[0U]) == 0);
    assert(output == "\033[0;37mlinux\033[0m!");
}

/** @brief Verifies safe process, executable, network, and user queries. */
void testPlatformQueries()
{
    XWalkUtilsLinux backend;
    XWalkUtils utilities(&backend, backend.utilityCallbacks());

    const XWalkCommandResult result = utilities.runCommand(
        "printf 'standard'; printf 'error' >&2; exit 7");
    assert(result.status == 7);
    assert(result.output == "standarderror");
    assert(utilities.commandExists("sh"));
    assert(utilities.checkExecutable("/bin/sh"));
    assert(!utilities.isInstalled("xwalk-command-that-does-not-exist"));
    assert(utilities.ipAddress("xwalk-interface-that-does-not-exist").empty());
    assert(!utilities.username().empty());
}

/** @brief Verifies that the RAII guard suppresses and then restores standard error. */
void testStderrGuard()
{
    fixedarray<int32, 2U> errorPipe{};
    assert(::pipe(errorPipe.data()) == 0);
    const int32 savedError = ::dup(STDERR_FILENO);
    assert(savedError >= 0);
    assert(::dup2(errorPipe[1U], STDERR_FILENO) >= 0);
    assert(::close(errorPipe[1U]) == 0);

    XWalkUtilsLinux backend;
    {
        XWalkStderrGuard guard(&backend, backend.stderrRedirectCallback(),
            backend.stderrRestoreCallback());
        const stringview suppressed{"suppressed"};
        assert(::write(STDERR_FILENO, suppressed.data(), suppressed.size()) >= 0);
        static_cast<void>(guard);
    }
    const stringview restored{"restored"};
    assert(::write(STDERR_FILENO, restored.data(), restored.size()) >= 0);

    assert(::dup2(savedError, STDERR_FILENO) >= 0);
    assert(::close(savedError) == 0);
    const string output = readPipe(errorPipe[0U]);
    assert(::close(errorPipe[0U]) == 0);
    assert(output == "restored");
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every safe Linux utility backend software test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int main()
{
    testOutput();
    testPlatformQueries();
    testStderrGuard();
    return 0;
}
