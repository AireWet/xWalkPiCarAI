/******************************************************************************
 * @file        TestConfig.hpp
 * @brief       Declares centralized GoogleTest XML configuration types.
 *
 * @details
 * Defines the enabled-suite and enabled-case model loaded from the checked-in
 * xGoogleTest configuration file.
 *
 * @project     xWalk Firmware
 * @module      xGoogleTest
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef TEST_CONFIG_HPP
#define TEST_CONFIG_HPP

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @brief Stores one configured GoogleTest case.
     */
    struct TestCaseConfig
    {
            /** @brief Exact GoogleTest case name. */
            string name;
            /** @brief Whether the case is selected by this configuration. */
            boolean enabled{};
    };

    /** @brief Ordered collection of configured GoogleTest cases. */
    using testcaseconfigvector = std::vector<TestCaseConfig>;

    /**
     * @brief Stores one configured GoogleTest suite and its cases.
     */
    struct TestSuiteConfig
    {
            /** @brief Exact GoogleTest suite name. */
            string name;
            /** @brief Whether the suite is selected by this configuration. */
            boolean enabled{};
            /** @brief Cases belonging to the suite in stable execution order. */
            testcaseconfigvector cases;
    };

    /** @brief Ordered collection of configured GoogleTest suites. */
    using testsuiteconfigvector = std::vector<TestSuiteConfig>;

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @brief Loads and validates the centralized test-selection XML.
     *
     * @details
     * Rejects malformed XML, invalid enabled values, duplicate entries, missing
     * entries, and names that are not registered by the centralized runner.
     */
    class TestConfig
    {
        public:
            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Loads one complete XML configuration.
             *
             * @param[in] path
             * Readable XML configuration path.
             *
             * @param[in] availableSuites
             * Complete suite and case inventory registered by the runner.
             *
             * @param[out] error
             * Empty on success; otherwise a concise validation diagnostic.
             *
             * @return
             * `true` when the configuration is complete and valid; otherwise `false`.
             */
            boolean load(const filesystempath& path, const testsuiteconfigvector& availableSuites, string& error);

            /**
             * @brief Returns the validated suite configuration.
             *
             * @return
             * Read-only configuration that remains valid for this object's lifetime.
             */
            const testsuiteconfigvector& suites() const noexcept;

        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Validated configuration stored in XML order. */
            testsuiteconfigvector suitesValue;
    };

} /* namespace xwalk::hal::test */

#endif /* TEST_CONFIG_HPP */
