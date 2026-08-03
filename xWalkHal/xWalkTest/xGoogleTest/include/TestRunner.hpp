/******************************************************************************
 * @file        TestRunner.hpp
 * @brief       Declares centralized GoogleTest registration and selection.
 *
 * @details
 * Registers existing HAL host scenarios, parses temporary command-line
 * overrides, and applies the final GoogleTest filter.
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

#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "TestConfig.hpp"

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
 * Enumeration declarations
 ******************************************************************************/

/** @brief Selects the host-safe or physical-hardware test inventory. */
enum class TestProfile : uint8
{
    Host,
    Hardware
};

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @brief Stores one temporary command-line test selection.
 */
struct TestSelection
{
    /** @brief Exact suite name selected by the caller. */
    string suiteName;
    /** @brief Empty for a suite selection; otherwise the exact case name. */
    string caseName;
    /** @brief Requested enabled state for this run. */
    boolean enabled{};
};

/** @brief Ordered list of command-line selections. */
using testselectionvector = std::vector<TestSelection>;

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @brief Registers and selects all centralized xWalk HAL host tests.
 *
 * @details
 * Existing assertion-based test entry points execute in isolated child
 * processes so one failure becomes a GoogleTest failure instead of terminating
 * the complete central executable.
 */
class TestRunner
{
public:

/**************************************************************************
 * Public constructors and destructor
 **************************************************************************/

    /**
     * @brief Creates the complete centralized test inventory.
     *
     * @param[in] profile
     * Host or hardware inventory selected before registration.
     *
     * @param[in] binaryDirectory
     * Absolute HAL build directory containing generated host-test data.

     * @param[in] runtimeConfigurationPath
     * YAML file containing board and AI hardware-test arguments.
     *
     * @post
     * `availableTests()` contains every host-safe scenario compiled into the
     * central executable.
     */
    TestRunner(TestProfile profile, filesystempath binaryDirectory,
        filesystempath runtimeConfigurationPath);

/**************************************************************************
 * Public member functions
 **************************************************************************/

    /**
     * @brief Registers the inventory with GoogleTest.
     *
     * @post
     * Every available suite and case can be selected by a GoogleTest filter.
     */
    void registerTests() const;

    /**
     * @brief Parses and removes the explicit test-profile argument.
     *
     * @param[in,out] argumentCount
     * Original process argument count, compacted after parsing.
     *
     * @param[in,out] argumentValues
     * Mutable process argument array.
     *
     * @param[out] profile
     * Host by default, or the explicitly selected profile.

     * @param[in,out] runtimeConfigurationPath
     * Default YAML path replaced by an explicit `--runtime-config` value.
     *
     * @param[out] error
     * Empty on success; otherwise a profile-selection diagnostic.
     *
     * @return
     * `true` when the profile arguments are valid; otherwise `false`.
     */
    static boolean processProfile(int32& argumentCount, charpointer argumentValues[],
        TestProfile& profile, filesystempath& runtimeConfigurationPath, string& error);

    /**
     * @brief Returns the registered suite and case inventory.
     *
     * @return
     * Read-only inventory valid for this object's lifetime.
     */
    const testsuiteconfigvector& availableTests() const noexcept;

    /**
     * @brief Detects an explicit standard GoogleTest filter.
     *
     * @param[in] argumentCount
     * Number of original process arguments.
     *
     * @param[in] argumentValues
     * Non-owning process argument array.
     *
     * @return
     * `true` when `--gtest_filter` is present; otherwise `false`.
     */
    boolean hasStandardFilter(int32 argumentCount, charpointer argumentValues[]) const;

    /**
     * @brief Parses and removes custom suite or case selections.
     *
     * @param[in,out] argumentCount
     * GoogleTest-filtered argument count; reduced after recognized selections.
     *
     * @param[in,out] argumentValues
     * Non-owning argument array compacted in place.
     *
     * @param[out] error
     * Empty on success; otherwise the invalid selection and valid inventory.
     *
     * @return
     * `true` when every remaining argument is a valid selection; otherwise
     * `false`.
     */
    boolean processSelections(int32& argumentCount, charpointer argumentValues[], string& error);

    /**
     * @brief Applies the filter selected by the documented precedence.
     *
     * @param[in] configuration
     * Validated XML configuration used when no higher-priority selection exists.
     *
     * @param[in] standardFilterSelected
     * Whether the original command line supplied `--gtest_filter`.
     *
     * @details
     * Custom selections override a standard filter, which overrides XML.
     */
    void applyFilter(const TestConfig& configuration, boolean standardFilterSelected) const;

    /**
     * @brief Formats every valid suite and case for an error diagnostic.
     *
     * @return
     * Multi-line owned text listing the complete registered inventory.
     */
    string validTestsText() const;

private:

/**************************************************************************
 * Private data members
 **************************************************************************/

    /** @brief Inventory profile selected before GoogleTest registration. */
    TestProfile profileValue;
    /** @brief Absolute HAL build directory containing generated test data. */
    filesystempath binaryDirectoryValue;
    /** @brief YAML file supplying board and AI hardware-test arguments. */
    filesystempath runtimeConfigurationPathValue;
    /** @brief Complete host-safe suite and case inventory. */
    testsuiteconfigvector availableSuites;
    /** @brief Validated temporary command-line overrides in caller order. */
    testselectionvector selections;
};

} /* namespace xwalk::hal::test */

#endif /* TEST_RUNNER_HPP */
