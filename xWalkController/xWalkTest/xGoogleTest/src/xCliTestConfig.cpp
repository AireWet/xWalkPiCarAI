/******************************************************************************
 * @file        xCliTestConfig.cpp
 * @brief       Implements centralized CLI GoogleTest XML configuration.
 *
 * @details
 * Parses the checked-in XML with TinyXML2 and validates it against the exact
 * suite inventory shared by the unit and sequence executables.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xCliTestConfig.h"

#include <tinyxml2.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains XML validation helpers private to this translation unit. */
namespace
{

/**
 * @brief Finds one available suite by exact name.
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
const xwalk::agent::test::CliTestSuiteConfig* findSuite(
    const xwalk::agent::test::clitestsuiteconfigvector& suites,
    ::ctrl::stringview name)
{
    for (const xwalk::agent::test::CliTestSuiteConfig& suite : suites)
    {
        if (suite.name == name)
        {
            return &suite;
        }
    }
    return nullptr;
}

/**
 * @brief Finds one available case by exact name.
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
const xwalk::agent::test::CliTestCaseConfig* findCase(
    const xwalk::agent::test::CliTestSuiteConfig& suite,
    ::ctrl::stringview name)
{
    for (const xwalk::agent::test::CliTestCaseConfig& testCase : suite.cases)
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
::ctrl::boolean readEnabled(
    const tinyxml2::XMLElement& element, ::ctrl::boolean& enabled)
{
    const char* value = element.Attribute("enabled");
    if (value == nullptr)
    {
        return false;
    }
    const ::ctrl::stringview text(value);
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
 * @namespace xwalk::agent::test
 * @brief Contains centralized host verification for the xWalk CLI.
 */
namespace xwalk::agent::test
{

/******************************************************************************
 * Function definitions
 ******************************************************************************/

/**
 * @brief Returns every Controller test grouped by its owning functional group.
 * @return Complete suite and case inventory in stable XML order.
 */
clitestsuiteconfigvector availableCliTests()
{
    return {
        {"XWalkControllerGroup", true,
            {{"Controller", true}, {"ControllerCommands", true}, {"Help", true}}},
        {"XWalkAgentPlatformGroup", true, {{"Doctor", true}}},
        {"XWalkAgentCalibrationGroup", true,
            {{"ServoZeroing", true}, {"Calibration", true}}},
        {"XWalkAgentVehicleGroup", true,
            {{"Move", true}, {"KeyboardControl", true},
             {"ObstacleAvoidance", true}, {"CliffDetection", true},
             {"Turn", true}, {"Sensor", true}, {"LineTracking", true},
             {"SelfDrive", true}}},
        {"XWalkAgentVisionGroup", true,
            {{"ComputerVision", true}, {"FaceTracking", true},
             {"BullFight", true}, {"TreasureHunt", true},
             {"VideoRecording", true}, {"VideoCar", true}, {"Camera", true}}},
        {"XWalkAgentConnectivityGroup", true,
            {{"AppControl", true}, {"Spi", true}}},
        {"XWalkAgentMediaGroup", true,
            {{"SoundBackgroundMusic", true}, {"Sound", true}}},
        {"XWalkAgentVoiceGroup", true,
            {{"VoiceChat", true}, {"VoiceActiveCar", true},
             {"VoiceActiveCarGpt", true}, {"GptCar", true},
             {"VoiceControlledCar", true}, {"VoicePromptCar", true},
             {"StorytellingRobot", true}, {"TextVisionTalk", true},
             {"OnlineLlmTest", true}}}
    };
}

/**
 * @brief Returns every Controller sequence test grouped by functional group.
 * @return Complete sequence suite and case inventory in stable XML order.
 */
clitestsuiteconfigvector availableCliSequenceTests()
{
    clitestsuiteconfigvector suites = availableCliTests();
    suites.front().cases.erase(suites.front().cases.begin());
    return suites;
}

/**
 * @brief Builds a GoogleTest filter from enabled XML suites and cases.
 * @param[in] configuration Complete validated Controller test configuration.
 * @return Colon-separated enabled tests, or the negative-all filter when none are enabled.
 */
::ctrl::string configuredCliTestFilter(const XWalkCliTestConfig& configuration)
{
    ::ctrl::string filter;
    for (const CliTestSuiteConfig& suite : configuration.suites())
    {
        if (!suite.enabled)
        {
            continue;
        }
        for (const CliTestCaseConfig& testCase : suite.cases)
        {
            if (!testCase.enabled)
            {
                continue;
            }
            const ::ctrl::boolean filterAvailable =
                static_cast<::ctrl::boolean>(
                    !filter.empty());
            if (filterAvailable)
            {
                filter += ":";
            }
            filter += suite.name + "." + testCase.name;
        }
    }
    return filter.empty() ? "-*" : filter;
}

/**
 * @brief Reports whether process arguments contain a GoogleTest filter override.
 * @param[in] argumentCount Number of process arguments.
 * @param[in] argumentValues Non-owning process argument array valid for this call.
 * @return `true` when `--gtest_filter` is present; otherwise `false`.
 */
::ctrl::boolean hasGoogleTestFilter(int argumentCount, char* argumentValues[])
{
    for (int index = 1; index < argumentCount; ++index)
    {
        const ::ctrl::stringview argument(argumentValues[index]);
        const ::ctrl::boolean argumentGtestFilterInvalid =
            static_cast<::ctrl::boolean>(
                (argument == "--gtest_filter") ||
            (argument.find("--gtest_filter=") == 0U));
        if (argumentGtestFilterInvalid)
        {
            return true;
        }
    }
    return false;
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

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
::ctrl::boolean XWalkCliTestConfig::load(const ::ctrl::filesystempath& path,
    const clitestsuiteconfigvector& availableSuites, ::ctrl::string& error)
{
    suitesValue.clear();
    error.clear();

    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError loadResult = document.LoadFile(path.string().c_str());
    if (loadResult != tinyxml2::XML_SUCCESS)
    {
        error = "cannot load CLI test configuration '" + path.string() + "': " +
            document.ErrorStr();
        return false;
    }

    const tinyxml2::XMLElement* root = document.FirstChildElement("testConfiguration");
    const ::ctrl::boolean rootDocumentRootElementInvalid =
        static_cast<::ctrl::boolean>(
            (root == nullptr) || (root != document.RootElement()) ||
        (root->NextSiblingElement() != nullptr));
    if (rootDocumentRootElementInvalid)
    {
        error = "CLI test configuration must contain one testConfiguration root element";
        return false;
    }

    ::ctrl::orderedmap<::ctrl::string, ::ctrl::boolean> seenSuites;
    for (const tinyxml2::XMLElement* suiteElement = root->FirstChildElement();
         suiteElement != nullptr; suiteElement = suiteElement->NextSiblingElement())
    {
        const ::ctrl::boolean suiteElementNameTestSuiteDifferent =
            static_cast<::ctrl::boolean>(
                ::ctrl::stringview(suiteElement->Name()) != "testSuite");
        if (suiteElementNameTestSuiteDifferent)
        {
            error = "unexpected XML element under testConfiguration: " +
                ::ctrl::string(suiteElement->Name());
            return false;
        }
        const char* suiteNameValue = suiteElement->Attribute("name");
        const ::ctrl::boolean suiteNameInvalid =
            static_cast<::ctrl::boolean>(
                (suiteNameValue == nullptr) || (::ctrl::stringview(suiteNameValue).empty()));
        if (suiteNameInvalid)
        {
            error = "testSuite requires a non-empty name attribute";
            return false;
        }
        const ::ctrl::string suiteName(suiteNameValue);
        const ::ctrl::boolean seenSuitesCountSuiteNameDifferent =
            static_cast<::ctrl::boolean>(
                seenSuites.count(suiteName) != 0U);
        if (seenSuitesCountSuiteNameDifferent)
        {
            error = "duplicate CLI test suite in XML: " + suiteName;
            return false;
        }
        const CliTestSuiteConfig* availableSuite = findSuite(availableSuites, suiteName);
        if (availableSuite == nullptr)
        {
            error = "unknown CLI test suite in XML: " + suiteName;
            return false;
        }

        CliTestSuiteConfig configuredSuite;
        configuredSuite.name = suiteName;
        const ::ctrl::boolean suiteEnabledRead =
            readEnabled(*suiteElement, configuredSuite.enabled);
        if (suiteEnabledRead == false)
        {
            error = "suite enabled attribute must be 0 or 1: " + suiteName;
            return false;
        }

        ::ctrl::orderedmap<::ctrl::string, ::ctrl::boolean> seenCases;
        for (const tinyxml2::XMLElement* caseElement = suiteElement->FirstChildElement();
             caseElement != nullptr; caseElement = caseElement->NextSiblingElement())
        {
            const ::ctrl::boolean caseElementNameTestCaseDifferent =
                static_cast<::ctrl::boolean>(
                    ::ctrl::stringview(caseElement->Name()) != "testCase");
            if (caseElementNameTestCaseDifferent)
            {
                error = "unexpected XML element in suite " + suiteName + ": " +
                    caseElement->Name();
                return false;
            }
            const char* caseNameValue = caseElement->Attribute("name");
            const ::ctrl::boolean caseNameInvalid =
                static_cast<::ctrl::boolean>(
                    (caseNameValue == nullptr) || (::ctrl::stringview(caseNameValue).empty()));
            if (caseNameInvalid)
            {
                error = "testCase requires a non-empty name in suite " + suiteName;
                return false;
            }
            const ::ctrl::string caseName(caseNameValue);
            const ::ctrl::boolean seenCasesCountCaseNameDifferent =
                static_cast<::ctrl::boolean>(
                    seenCases.count(caseName) != 0U);
            if (seenCasesCountCaseNameDifferent)
            {
                error = "duplicate CLI test case in XML: " + suiteName + "." + caseName;
                return false;
            }
            const ::ctrl::boolean findCaseAvailableSuiteCaseNameMatched =
                static_cast<::ctrl::boolean>(
                    findCase(*availableSuite, caseName) == nullptr);
            if (findCaseAvailableSuiteCaseNameMatched)
            {
                error = "unknown CLI test case in XML: " + suiteName + "." + caseName;
                return false;
            }

            CliTestCaseConfig configuredCase;
            configuredCase.name = caseName;
            const ::ctrl::boolean caseEnabledRead =
                readEnabled(*caseElement, configuredCase.enabled);
            if (caseEnabledRead == false)
            {
                error = "case enabled attribute must be 0 or 1: " + suiteName + "." + caseName;
                return false;
            }
            configuredSuite.cases.push_back(configuredCase);
            seenCases[caseName] = true;
        }

        const ::ctrl::boolean configuredSuiteCasesAvailableSuiteDifferent =
            static_cast<::ctrl::boolean>(
                configuredSuite.cases.size() != availableSuite->cases.size());
        if (configuredSuiteCasesAvailableSuiteDifferent)
        {
            error = "suite is missing one or more registered CLI cases: " + suiteName;
            return false;
        }
        suitesValue.push_back(configuredSuite);
        seenSuites[suiteName] = true;
    }

    const ::ctrl::boolean suitesAvailableSuitesDifferent =
        static_cast<::ctrl::boolean>(
            suitesValue.size() != availableSuites.size());
    if (suitesAvailableSuitesDifferent)
    {
        error = "CLI test configuration is missing one or more registered suites";
        suitesValue.clear();
        return false;
    }
    return true;
}

/**
 * @brief Returns the validated suite configuration.
 *
 * @return
 * Read-only configuration valid for this object's lifetime.
 */
const clitestsuiteconfigvector& XWalkCliTestConfig::suites() const noexcept
{
    return suitesValue;
}

} /* namespace xwalk::agent::test */
