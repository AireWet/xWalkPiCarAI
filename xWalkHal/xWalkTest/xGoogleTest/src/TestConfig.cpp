/******************************************************************************
 * @file        TestConfig.cpp
 * @brief       Implements centralized GoogleTest XML configuration loading.
 *
 * @details
 * Parses the checked-in configuration with TinyXML2 and validates it against
 * the test inventory compiled into the central executable.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "TestConfig.hpp"

#include <tinyxml2.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains XML validation helpers private to this translation unit.
 */
namespace
{

/**
 * @brief Finds one suite by exact name.
 *
 * @param[in] suites
 * Suite collection to search.
 *
 * @param[in] name
 * Exact suite name.
 *
 * @return
 * Non-owning suite pointer when found; otherwise `nullptr`.
 */
const xwalk::hal::test::TestSuiteConfig* findSuite(
    const xwalk::hal::test::testsuiteconfigvector& suites, xwalk::hal::stringview name)
{
    for (const xwalk::hal::test::TestSuiteConfig& suite : suites)
    {
        if (suite.name == name)
        {
            return &suite;
        }
    }
    return nullptr;
}

/**
 * @brief Finds one case by exact name.
 *
 * @param[in] suite
 * Suite whose cases are searched.
 *
 * @param[in] name
 * Exact case name.
 *
 * @return
 * Non-owning case pointer when found; otherwise `nullptr`.
 */
const xwalk::hal::test::TestCaseConfig* findCase(
    const xwalk::hal::test::TestSuiteConfig& suite, xwalk::hal::stringview name)
{
    for (const xwalk::hal::test::TestCaseConfig& testCase : suite.cases)
    {
        if (testCase.name == name)
        {
            return &testCase;
        }
    }
    return nullptr;
}

/**
 * @brief Reads one strict Boolean XML attribute.
 *
 * @param[in] element
 * Element containing the attribute.
 *
 * @param[out] enabled
 * Parsed Boolean value.
 *
 * @return
 * `true` only when the attribute is exactly `0` or `1`.
 */
xwalk::hal::boolean readEnabled(const tinyxml2::XMLElement& element, xwalk::hal::boolean& enabled)
{
    const char* value = element.Attribute("enabled");
    if (value == nullptr)
    {
        return false;
    }
    const xwalk::hal::stringview text(value);
    if (text == "0")
    {
        enabled = false;
        return true;
    }
    if (text == "1")
    {
        enabled = true;
        return true;
    }
    return false;
}

} /* namespace */

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

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
boolean TestConfig::load(const filesystempath& path, const testsuiteconfigvector& availableSuites,
    string& error)
{
    suitesValue.clear();
    error.clear();

    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError loadResult = document.LoadFile(path.string().c_str());
    if (loadResult != tinyxml2::XML_SUCCESS)
    {
        error = "cannot load test configuration '" + path.string() + "': " + document.ErrorStr();
        return false;
    }

    const tinyxml2::XMLElement* root = document.FirstChildElement("testConfiguration");
    if ((root == nullptr) || (root != document.RootElement()) ||
        (root->NextSiblingElement() != nullptr))
    {
        error = "test configuration must contain one testConfiguration root element";
        return false;
    }

    orderedmap<string, boolean> seenSuites;
    for (const tinyxml2::XMLElement* suiteElement = root->FirstChildElement();
         suiteElement != nullptr; suiteElement = suiteElement->NextSiblingElement())
    {
        if (stringview(suiteElement->Name()) != "testSuite")
        {
            error = "unexpected XML element under testConfiguration: " +
                string(suiteElement->Name());
            return false;
        }
        const char* suiteNameValue = suiteElement->Attribute("name");
        if ((suiteNameValue == nullptr) || (stringview(suiteNameValue).empty()))
        {
            error = "testSuite requires a non-empty name attribute";
            return false;
        }
        const string suiteName(suiteNameValue);
        if (seenSuites.count(suiteName) != 0U)
        {
            error = "duplicate test suite in XML: " + suiteName;
            return false;
        }
        const TestSuiteConfig* availableSuite = findSuite(availableSuites, suiteName);
        if (availableSuite == nullptr)
        {
            error = "unknown test suite in XML: " + suiteName;
            return false;
        }

        TestSuiteConfig configuredSuite;
        configuredSuite.name = suiteName;
        if (!readEnabled(*suiteElement, configuredSuite.enabled))
        {
            error = "suite enabled attribute must be 0 or 1: " + suiteName;
            return false;
        }

        orderedmap<string, boolean> seenCases;
        for (const tinyxml2::XMLElement* caseElement = suiteElement->FirstChildElement();
             caseElement != nullptr; caseElement = caseElement->NextSiblingElement())
        {
            if (stringview(caseElement->Name()) != "testCase")
            {
                error = "unexpected XML element in suite " + suiteName + ": " + caseElement->Name();
                return false;
            }
            const char* caseNameValue = caseElement->Attribute("name");
            if ((caseNameValue == nullptr) || (stringview(caseNameValue).empty()))
            {
                error = "testCase requires a non-empty name in suite " + suiteName;
                return false;
            }
            const string caseName(caseNameValue);
            if (seenCases.count(caseName) != 0U)
            {
                error = "duplicate test case in XML: " + suiteName + "." + caseName;
                return false;
            }
            if (findCase(*availableSuite, caseName) == nullptr)
            {
                error = "unknown test case in XML: " + suiteName + "." + caseName;
                return false;
            }

            TestCaseConfig configuredCase;
            configuredCase.name = caseName;
            if (!readEnabled(*caseElement, configuredCase.enabled))
            {
                error = "case enabled attribute must be 0 or 1: " + suiteName + "." + caseName;
                return false;
            }
            configuredSuite.cases.push_back(configuredCase);
            seenCases[caseName] = true;
        }

        if (configuredSuite.cases.size() != availableSuite->cases.size())
        {
            error = "suite is missing one or more registered cases: " + suiteName;
            return false;
        }
        suitesValue.push_back(configuredSuite);
        seenSuites[suiteName] = true;
    }

    if (suitesValue.size() != availableSuites.size())
    {
        error = "test configuration is missing one or more registered suites";
        suitesValue.clear();
        return false;
    }
    return true;
}

/**
 * @brief Returns the validated suite configuration.
 *
 * @return
 * Read-only configuration that remains valid for this object's lifetime.
 */
const testsuiteconfigvector& TestConfig::suites() const noexcept
{
    return suitesValue;
}

} /* namespace xwalk::hal::test */
