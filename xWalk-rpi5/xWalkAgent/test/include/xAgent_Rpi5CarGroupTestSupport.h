/******************************************************************************
 * @file        xAgent_Rpi5CarGroupTestSupport.h
 * @brief       Resolves Agent group test paths without build-defined macros.
 *
 * @details
 * Derives child test executables and writable test-data directories from the
 * running GoogleTest executable so editors and static analyzers see complete
 * C++ declarations without depending on target compile definitions.
 *
 * @project     xWalk Firmware
 * @module      xWalkAgent Group Test Support
 *
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_GROUP_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_GROUP_TEST_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

#include <filesystem>

/******************************************************************************
 * Namespace
 ******************************************************************************/

namespace xwalk
{
    namespace agent
    {
        namespace test
        {

            /**
             * @brief Resolves the directory containing the running group test executable.
             * @return Canonical parent directory of the Linux process executable.
             * @throws std::filesystem::filesystem_error When `/proc/self/exe` cannot be resolved.
             */
            inline xwalk::agent::filesystempath groupExecutableDirectory()
            {
                return std::filesystem::canonical("/proc/self/exe").parent_path();
            }

            /**
             * @brief Resolves one child module test beside its owning group executable.
             * @param[in] moduleDirectory Child module build-directory name.
             * @param[in] executableName Child test executable name.
             * @return Absolute child test executable path.
             * @throws std::filesystem::filesystem_error When the process executable cannot be resolved.
             */
            inline xwalk::agent::filesystempath childTestExecutable(const char* moduleDirectory,
                                                                    const char* executableName)
            {
                return groupExecutableDirectory() / moduleDirectory / executableName;
            }

            /**
             * @brief Resolves a writable directory for one child module test.
             * @param[in] moduleName Stable child module test-data name.
             * @return Absolute group-local test-data directory.
             * @throws std::filesystem::filesystem_error When the process executable cannot be resolved.
             */
            inline xwalk::agent::filesystempath groupTestDataDirectory(const char* moduleName)
            {
                return groupExecutableDirectory() / "test-data" / moduleName;
            }

        } /* namespace test */
    } /* namespace agent */
} /* namespace xwalk */

#endif /* XAGENT_RPI5CAR_GROUP_TEST_SUPPORT_H */
