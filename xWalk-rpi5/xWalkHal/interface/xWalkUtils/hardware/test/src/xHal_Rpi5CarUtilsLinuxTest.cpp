/******************************************************************************
 * @file        xHal_Rpi5CarUtilsLinuxTest.cpp
 * @brief       Verifies the Linux utility backend with safe host operations.
 *
 * @details
 * Exercises callback composition, ANSI output, process output collection,
 * executable lookup, unavailable-interface lookup, username lookup, and stderr
 *RAII. Mixer state and other physical Raspberry Pi resources are not accessed.
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
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarUtilsSimulationConfig.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains safe Linux-backend test scenarios private to this
 * translation unit. */
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

    /** @brief Verifies callback-bound ANSI output without writing to the test
     * terminal. */
    void testOutput()
    {
        fixedarray<int32, 2U> outputPipe{};
        xwalk::hal::test::requireTestCondition(::pipe(outputPipe.data()) == 0);
        const int32 savedOutput = ::dup(STDOUT_FILENO);
        xwalk::hal::test::requireTestCondition(savedOutput >= 0);
        xwalk::hal::test::requireTestCondition(::dup2(outputPipe[1U], STDOUT_FILENO) >= 0);
        xwalk::hal::test::requireTestCondition(::close(outputPipe[1U]) == 0);

        XWalkUtilsLinux backend;
        XWalkUtils utilities(&backend, backend.utilityCallbacks());
        utilities.info("linux", "!", true);

        xwalk::hal::test::requireTestCondition(::dup2(savedOutput, STDOUT_FILENO) >= 0);
        xwalk::hal::test::requireTestCondition(::close(savedOutput) == 0);
        const string output = readPipe(outputPipe[0U]);
        xwalk::hal::test::requireTestCondition(::close(outputPipe[0U]) == 0);
        xwalk::hal::test::requireTestCondition(output.find("\033[0;37mlinux\033[0m!") != string::npos);
    }

    /** @brief Verifies safe process, executable, network, and user queries. */
    void testPlatformQueries()
    {
        XWalkUtilsLinux backend;
        XWalkUtils utilities(&backend, backend.utilityCallbacks());

        const XWalkCommandResult result = utilities.runCommand("printf 'standard'; printf 'error' >&2; exit 7");
        xwalk::hal::test::requireTestCondition(result.status == 7);
        xwalk::hal::test::requireTestCondition(result.output == "standarderror");
        xwalk::hal::test::requireTestCondition(utilities.commandExists("sh"));
        xwalk::hal::test::requireTestCondition(utilities.checkExecutable("/bin/sh"));
        xwalk::hal::test::requireTestCondition(!utilities.isInstalled("xwalk-command-that-does-not-exist"));
        xwalk::hal::test::requireTestCondition(utilities.ipAddress("xwalk-interface-that-does-not-exist").empty());
        xwalk::hal::test::requireTestCondition(!utilities.username().empty());
    }

    /** @brief Verifies that the RAII guard suppresses and then restores standard
     * error. */
    void testStderrGuard()
    {
        fixedarray<int32, 2U> errorPipe{};
        xwalk::hal::test::requireTestCondition(::pipe(errorPipe.data()) == 0);
        const int32 savedError = ::dup(STDERR_FILENO);
        xwalk::hal::test::requireTestCondition(savedError >= 0);
        xwalk::hal::test::requireTestCondition(::dup2(errorPipe[1U], STDERR_FILENO) >= 0);
        xwalk::hal::test::requireTestCondition(::close(errorPipe[1U]) == 0);

        XWalkUtilsLinux backend;
        {
            XWalkStderrGuard guard(&backend, backend.stderrRedirectCallback(), backend.stderrRestoreCallback());
            const stringview suppressed{"suppressed"};
            xwalk::hal::test::requireTestCondition(::write(STDERR_FILENO, suppressed.data(), suppressed.size()) >= 0);
            static_cast<void>(guard);
        }
        const stringview restored{"restored"};
        xwalk::hal::test::requireTestCondition(::write(STDERR_FILENO, restored.data(), restored.size()) >= 0);

        xwalk::hal::test::requireTestCondition(::dup2(savedError, STDERR_FILENO) >= 0);
        xwalk::hal::test::requireTestCondition(::close(savedError) == 0);
        const string output = readPipe(errorPipe[0U]);
        xwalk::hal::test::requireTestCondition(::close(errorPipe[0U]) == 0);
        xwalk::hal::test::requireTestCondition(output.find("suppressed") == string::npos);
        xwalk::hal::test::requireTestCondition(output.find("restored") != string::npos);
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
    XWalkTrace::configureGlobal(XWALK_UTILS_SIMULATION_TRACE_CONFIG_PATH, XWALK_UTILS_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .141, "xWalkUtils Linux software tests started");
    testOutput();
    testPlatformQueries();
    testStderrGuard();
    XWALK_HAL_TRACE_UID0(RPI .142, "xWalkUtils Linux software tests completed");
    return 0;
}
