/******************************************************************************
 * @file        xHal_Rpi5CarTestFunctions.h
 * @brief       Provides host-test process isolation helpers.
 *
 * @details
 * Runs failure scenarios in child processes so tests can verify rejected
 * operations without installing C++ exception handlers.
 *
 * @project     xWalk Firmware
 * @module      xWalkLibraryCommon
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

#ifndef XHAL_RPI5CAR_TEST_FUNCTIONS_H
#define XHAL_RPI5CAR_TEST_FUNCTIONS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLinuxHeaders.h"
#include "xHal_Rpi5CarTypes.h"

#if defined(XWALK_GCC_COVERAGE)
extern "C" void __gcov_dump() noexcept;
#endif

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-safe verification helpers for xWalk firmware tests.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Inline function definitions
 ******************************************************************************/

/**
 * @brief Verifies that an operation terminates its isolated child process.
 *
 * @details
 * An uncaught exception invokes the child process termination handler and
 * produces a nonzero exit status. Normal return produces a zero exit status
 * and fails the parent assertion.
 *
 * @param[in] operation
 * Callable copied into the child process and invoked exactly once there.
 *
 * @pre
 * The host provides Linux process creation and wait operations.
 */
template <typename OperationType>
void expectFailure(OperationType operation)
{
#ifdef __linux__
    const auto childProcess = ::fork();
    assert(childProcess >= 0);
    if (childProcess == 0)
    {
        std::set_terminate([]() noexcept
        {
#if defined(XWALK_GCC_COVERAGE)
            __gcov_dump();
#endif
            ::_exit(EXIT_FAILURE);
        });
        operation();
#if defined(XWALK_GCC_COVERAGE)
        __gcov_dump();
#endif
        ::_exit(EXIT_SUCCESS);
    }

    int32 childStatus{};
    const auto completedProcess = ::waitpid(childProcess, &childStatus, 0);
    assert(completedProcess == childProcess);
    assert(!WIFEXITED(childStatus) || (WEXITSTATUS(childStatus) != EXIT_SUCCESS));
#else
    static_cast<void>(operation);
    assert(false && "Failure isolation requires a Linux host");
#endif
}

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_TEST_FUNCTIONS_H */
