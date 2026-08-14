/******************************************************************************
 * @file        xCliTestConfig.h
 * @brief       Declares centralized CLI GoogleTest configuration types.
 *
 * @details
 * Defines the shared enabled-suite and enabled-case model loaded by both CLI
 * test executables from the checked-in test-selection XML file.
 *
 * @project     xWalk Firmware
 * @module      xWalk CLI GoogleTest
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XCLI_TEST_CONFIG_H
#define XCLI_TEST_CONFIG_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent::test
 * @brief Contains centralized host verification for the xWalk CLI.
 */
namespace xwalk::agent::test
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Stores one configured CLI GoogleTest case. */
struct CliTestCaseConfig
{
    /** @brief Exact GoogleTest case name. */
    ::ctrl::string name;
    /** @brief Whether this configuration enables the case. */
    ::ctrl::boolean enabled{};
};

/** @brief Ordered collection of configured CLI GoogleTest cases. */
using clitestcaseconfigvector = std::vector<CliTestCaseConfig>;

/** @brief Stores one configured CLI GoogleTest suite and its cases. */
struct CliTestSuiteConfig
{
    /** @brief Exact GoogleTest suite name. */
    ::ctrl::string name;
    /** @brief Whether this configuration enables the suite. */
    ::ctrl::boolean enabled{};
    /** @brief Cases belonging to the suite in stable order. */
    clitestcaseconfigvector cases;
};

/** @brief Ordered collection of configured CLI GoogleTest suites. */
using clitestsuiteconfigvector = std::vector<CliTestSuiteConfig>;

/******************************************************************************
 * Function declarations
 ******************************************************************************/

/**
 * @brief Returns every Controller test grouped by its owning functional group.
 * @return Complete suite and case inventory in stable XML order.
 */
clitestsuiteconfigvector availableCliTests();

/**
 * @brief Returns every Controller sequence test grouped by functional group.
 * @return Complete sequence suite and case inventory in stable XML order.
 */
clitestsuiteconfigvector availableCliSequenceTests();

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkCliTestConfig
 * @brief Loads and validates the shared CLI test-selection XML.
 *
 * @details
 * Rejects malformed XML, invalid enabled values, duplicate entries, missing
 * entries, and names absent from the complete CLI test inventory.
 */
class XWalkCliTestConfig
{
public:

/**************************************************************************
 * Public member functions
 **************************************************************************/

    /**
     * @brief Loads one complete CLI test configuration.
     *
     * @param[in] path
     * Readable XML configuration path.
     *
     * @param[in] availableSuites
     * Complete suite and case inventory shared by the CLI test runners.
     *
     * @param[out] error
     * Empty on success; otherwise a concise validation diagnostic.
     *
     * @return
     * `true` when the configuration is complete and valid; otherwise `false`.
     */
    ::ctrl::boolean load(const ::ctrl::filesystempath& path,
        const clitestsuiteconfigvector& availableSuites, ::ctrl::string& error);

    /**
     * @brief Returns the validated suite configuration.
     *
     * @return
     * Read-only configuration valid for this object's lifetime.
     */
    const clitestsuiteconfigvector& suites() const noexcept;

private:

/**************************************************************************
 * Private data members
 **************************************************************************/

    /** @brief Validated configuration stored in XML order. */
    clitestsuiteconfigvector suitesValue;
};

/**
 * @brief Builds a GoogleTest filter from enabled XML suites and cases.
 * @param[in] configuration Complete validated Controller test configuration.
 * @return Colon-separated enabled tests, or the negative-all filter when none are enabled.
 */
::ctrl::string configuredCliTestFilter(const XWalkCliTestConfig& configuration);

/**
 * @brief Reports whether process arguments contain a GoogleTest filter override.
 * @param[in] argumentCount Number of process arguments.
 * @param[in] argumentValues Non-owning process argument array valid for this call.
 * @return `true` when `--gtest_filter` is present; otherwise `false`.
 */
::ctrl::boolean hasGoogleTestFilter(int argumentCount, char* argumentValues[]);

} /* namespace xwalk::agent::test */

#endif /* XCLI_TEST_CONFIG_H */
