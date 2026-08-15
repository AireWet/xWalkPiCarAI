/******************************************************************************
 * @file        TestRunnerTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by TestRunner.cpp.
 *
 * @project     xWalk Firmware
 * @module      Source Type Support
 *
 * @author      Joxy John
 * @date        2026-08-15
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef TESTRUNNERTYPES_H
#define TESTRUNNERTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "TestRunner.hpp"
#include "xHal_Rpi5CarLinuxHeaders.h"
#include "xHal_Rpi5CarTrace.h"
#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>
#include <cerrno>
#include <cstring>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::testrunner
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::testrunner
{

    using namespace xwalk::hal;

    /** @brief Legacy test entry point that accepts no arguments. */
    using legacytestnoargs = int (*)();

    /** @brief Legacy test entry point that accepts process-style arguments. */
    using legacytestwithargs = int (*)(int argumentCount, char* argumentValues[]);

    /**
     * @brief Defines one dynamically registered legacy test scenario.
     */
    struct LegacyTestDefinition
    {
            /** @brief Exact GoogleTest suite name. */
            xwalk::hal::string suiteName;
            /** @brief Exact GoogleTest case name. */
            xwalk::hal::string caseName;
            /** @brief Non-null no-argument entry point when this form is used. */
            legacytestnoargs noArgumentFunction{};
            /** @brief Non-null argument-taking entry point when this form is used. */
            legacytestwithargs argumentFunction{};
            /** @brief Arguments following the synthesized executable name. */
            xwalk::hal::stringvector arguments;
            /** @brief External executable used by a physical-hardware case. */
            xwalk::hal::string executablePath;
            /** @brief Whether ThreadSanitizer must skip this failure-isolation scenario.
             */
            xwalk::hal::boolean skipWithThreadSanitizer{};
            /** @brief Whether the case is registered natively by its module source. */
            xwalk::hal::boolean nativeGoogleTest{};
    };

    /** @brief Executes one existing assertion-based test as a GoogleTest case. */
    class LegacyGoogleTest : public ::testing::Test
    {
        public:
            /**
             * @brief Stores one immutable legacy test definition.
             * @param[in] definition Test entry point and owned arguments.
             */
            explicit LegacyGoogleTest(LegacyTestDefinition definition);

        protected:
            /** @brief Runs the legacy scenario in an isolated child process. */
            void TestBody() override;

        private:
            /** @brief Owned entry-point definition executed by this instance. */
            LegacyTestDefinition definitionValue;
    };

} /* namespace xwalk::source_types::testrunner */

#endif /* TESTRUNNERTYPES_H */
