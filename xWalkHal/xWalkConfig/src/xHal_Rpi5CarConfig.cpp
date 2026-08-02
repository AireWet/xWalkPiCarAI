/******************************************************************************
 * @file        xHal_Rpi5CarConfig.cpp
 * @brief       Implements section-aware configuration editing and persistence.
 *
 * @details
 * Parses configuration sections, manages explicit in-memory changes, preserves
 * comments and unrelated text, and performs same-directory replacement commits.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarConfig.h"

#include <filesystem>
#include <fstream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Parses configuration lines into an owned section map.
 *
 * @param[in] lines
 * Ordered file content without newline characters.
 *
 * @return
 * Parsed sections, including the empty-name default section.
 *
 * @throws std::runtime_error
 * If a section header or option name is malformed.
 */
configsections XWalkConfig::parseSections(const stringvector& lines)
{
    configsections parsedSections;
    string currentSection;
    parsedSections[currentSection] = {};

    for (const string& originalLine : lines)
    {
        const string line = trim(originalLine);
        if (line.empty() || (line.front() == '#'))
        {
            continue;
        }
        if (line.front() == '[')
        {
            currentSection = parseSectionHeader(line);
            parsedSections[currentSection] = {};
            continue;
        }

        const size separator = line.find('=');
        if (separator == string::npos)
        {
            continue;
        }

        const string optionName = trim(stringview(line).substr(0U, separator));
        validateOptionName(optionName);
        parsedSections[currentSection][optionName] =
            trim(stringview(line).substr(separator + 1U));
    }
    return parsedSections;
}

/**
 * @brief Appends options not already represented by existing file lines.
 *
 * @param[in,out] outputLines
 * Merged output receiving serialized options.
 *
 * @param[in,out] remainingSections
 * Writable copy whose completed section is removed.
 *
 * @param[in] sectionName
 * Section whose remaining options are appended.
 */
void XWalkConfig::appendMissingOptions(stringvector& outputLines,
    configsections& remainingSections, const string& sectionName)
{
    const auto sectionIterator = remainingSections.find(sectionName);
    if (sectionIterator == remainingSections.end())
    {
        return;
    }

    for (const auto& option : sectionIterator->second)
    {
        outputLines.push_back(option.first + XHAL_RPI5CAR_CONFIG_ASSIGNMENT_SEPARATOR + option.second);
    }
    remainingSections.erase(sectionIterator);
}

/**
 * @brief Merges the current memory image with existing file lines.
 *
 * @param[in] originalLines
 * Existing ordered file content without newline characters.
 *
 * @return
 * Updated lines retaining comments, blank lines, and unknown content.
 *
 * @throws std::runtime_error
 * If an existing section header is malformed.
 */
stringvector XWalkConfig::mergeLines(const stringvector& originalLines) const
{
    configsections remainingSections = sectionsValue;
    stringvector outputLines;
    string currentSection;

    for (const string& originalLine : originalLines)
    {
        const string line = trim(originalLine);
        if (!line.empty() && (line.front() == '['))
        {
            appendMissingOptions(outputLines, remainingSections, currentSection);
            currentSection = parseSectionHeader(line);
            outputLines.push_back(originalLine);
            continue;
        }

        const size separator = line.find('=');
        const auto sectionIterator = remainingSections.find(currentSection);
        if (!line.empty() && (line.front() != '#') && (separator != string::npos) &&
            (sectionIterator != remainingSections.end()))
        {
            const string optionName = trim(stringview(line).substr(0U, separator));
            const auto optionIterator = sectionIterator->second.find(optionName);
            if (optionIterator != sectionIterator->second.end())
            {
                outputLines.push_back(optionName + XHAL_RPI5CAR_CONFIG_ASSIGNMENT_SEPARATOR +
                    optionIterator->second);
                sectionIterator->second.erase(optionIterator);
                continue;
            }
        }
        outputLines.push_back(originalLine);
    }

    appendMissingOptions(outputLines, remainingSections, currentSection);
    for (const auto& sectionEntry : remainingSections)
    {
        if (!outputLines.empty() && !outputLines.back().empty())
        {
            outputLines.emplace_back();
        }
        if (!sectionEntry.first.empty())
        {
            outputLines.push_back("[" + sectionEntry.first + "]");
        }
        for (const auto& option : sectionEntry.second)
        {
            outputLines.push_back(option.first + XHAL_RPI5CAR_CONFIG_ASSIGNMENT_SEPARATOR +
                option.second);
        }
    }
    return outputLines;
}

/**
 * @brief Reads all configuration lines without newline characters.
 *
 * @return
 * Lines in their original order.
 *
 * @throws std::runtime_error
 * If the file cannot be opened or completely read.
 */
stringvector XWalkConfig::readLines() const
{
    inputfilestream configurationFile(filePathValue);
    if (!configurationFile.is_open())
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration file open failed");
    }

    stringvector lines;
    string line;
    while (readFileLine(configurationFile, line))
    {
        lines.push_back(line);
    }
    if (configurationFile.bad())
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration file read failed");
    }
    return lines;
}

/**
 * @brief Atomically replaces the configuration file with supplied lines.
 *
 * @param[in] lines
 * Complete ordered file content without newline characters.
 *
 * @post
 * A successful call leaves the configuration path containing `lines`.
 *
 * @throws std::runtime_error
 * If creation, writing, permission copying, or replacement fails.
 */
void XWalkConfig::writeLines(const stringvector& lines) const
{
    filesystempath replacementPath = filePathValue;
    replacementPath += XHAL_RPI5CAR_CONFIG_REPLACEMENT_SUFFIX;

    outputfilestream replacementFile(replacementPath, FILE_OPEN_WRITE_TRUNCATE);
    if (!replacementFile.is_open())
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration replacement file creation failed");
    }
    for (const string& line : lines)
    {
        replacementFile << line << '\n';
    }
    replacementFile.close();
    if (replacementFile.fail())
    {
        errorcode removeError;
        static_cast<void>(removeFilesystemEntry(replacementPath, removeError));
        XHAL_THROW_RUNTIME_ERROR("Configuration replacement file write failed");
    }

    errorcode statusError;
    const filesystemstatus originalStatus = filesystemStatus(filePathValue, statusError);
    if (statusError)
    {
        errorcode removeError;
        static_cast<void>(removeFilesystemEntry(replacementPath, removeError));
        XHAL_THROW_RUNTIME_ERROR("Configuration permission inspection failed");
    }

    errorcode permissionError;
    replaceFilesystemPermissions(replacementPath, originalStatus.permissions(), permissionError);
    if (permissionError)
    {
        errorcode removeError;
        static_cast<void>(removeFilesystemEntry(replacementPath, removeError));
        XHAL_THROW_RUNTIME_ERROR("Configuration replacement permission copy failed");
    }

    errorcode renameError;
    renameFilesystemEntry(replacementPath, filePathValue, renameError);
    if (renameError)
    {
        errorcode removeError;
        static_cast<void>(removeFilesystemEntry(replacementPath, removeError));
        XHAL_THROW_RUNTIME_ERROR("Configuration file replacement failed");
    }
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Reloads all configuration sections from the file.
 *
 * @return
 * Owned copy of the newly loaded section map.
 *
 * @post
 * Unsaved in-memory changes are replaced by current file content.
 *
 * @throws filesystemerror
 * If recreation of a removed file or parent directory fails.
 *
 * @throws std::runtime_error
 * If file creation, reading, or configuration parsing fails.
 */
configsections XWalkConfig::read()
{
    const mutexlock lock(mutexObject);
    ensureFileExists({});
    sectionsValue = parseSections(readLines());
    return sectionsValue;
}

/**
 * @brief Persists the current in-memory configuration.
 *
 * @post
 * Existing entries are updated and newly added entries are serialized.
 *
 * @throws filesystemerror
 * If recreation of a removed file or parent directory fails.
 *
 * @throws std::runtime_error
 * If file reading, parsing, writing, or replacement fails.
 */
void XWalkConfig::write()
{
    const mutexlock lock(mutexObject);
    ensureFileExists({});
    writeLines(mergeLines(readLines()));
}

/**
 * @brief Retrieves one option and inserts its default when absent.
 *
 * @param[in] sectionName
 * Empty for the default section or a valid named section.
 *
 * @param[in] optionName
 * Valid option name to retrieve.
 *
 * @param[in] defaultValue
 * Single-line value copied into memory when the option is absent.
 *
 * @return
 * Stored value or an owned copy of `defaultValue` when newly inserted.
 *
 * @throws std::out_of_range
 * If `sectionName` is not loaded.
 *
 * @throws std::invalid_argument
 * If a name or `defaultValue` cannot be represented safely.
 */
string XWalkConfig::get(stringview sectionName, stringview optionName, stringview defaultValue)
{
    if (!sectionName.empty())
    {
        validateSectionName(sectionName);
    }
    validateOptionName(optionName);
    validateValue(defaultValue);
    const mutexlock lock(mutexObject);

    const auto sectionIterator = sectionsValue.find(string(sectionName));
    if (sectionIterator == sectionsValue.end())
    {
        XHAL_THROW_OUT_OF_RANGE("Configuration section is not loaded");
    }
    auto optionIterator = sectionIterator->second.find(string(optionName));
    if (optionIterator == sectionIterator->second.end())
    {
        optionIterator = sectionIterator->second.emplace(string(optionName), string(defaultValue)).first;
    }
    return optionIterator->second;
}

/**
 * @brief Changes one option in an existing in-memory section.
 *
 * @param[in] sectionName
 * Empty for the default section or a valid named section.
 *
 * @param[in] optionName
 * Valid option name to add or replace.
 *
 * @param[in] value
 * Single-line value copied into memory.
 *
 * @throws std::out_of_range
 * If `sectionName` is not loaded.
 *
 * @throws std::invalid_argument
 * If a name or `value` cannot be represented safely.
 */
void XWalkConfig::set(stringview sectionName, stringview optionName, stringview value)
{
    if (!sectionName.empty())
    {
        validateSectionName(sectionName);
    }
    validateOptionName(optionName);
    validateValue(value);
    const mutexlock lock(mutexObject);

    const auto sectionIterator = sectionsValue.find(string(sectionName));
    if (sectionIterator == sectionsValue.end())
    {
        XHAL_THROW_OUT_OF_RANGE("Configuration section is not loaded");
    }
    sectionIterator->second[string(optionName)] = string(value);
}

/**
 * @brief Returns an owned copy of one loaded section.
 *
 * @param[in] sectionName
 * Empty for the default section or a valid named section.
 *
 * @return
 * Ordered option-value pairs in the requested section.
 *
 * @throws std::out_of_range
 * If `sectionName` is not loaded.
 *
 * @throws std::invalid_argument
 * If a non-empty `sectionName` is invalid.
 */
configsection XWalkConfig::section(stringview sectionName) const
{
    if (!sectionName.empty())
    {
        validateSectionName(sectionName);
    }
    const mutexlock lock(mutexObject);
    const auto sectionIterator = sectionsValue.find(string(sectionName));
    if (sectionIterator == sectionsValue.end())
    {
        XHAL_THROW_OUT_OF_RANGE("Configuration section is not loaded");
    }
    return sectionIterator->second;
}

/**
 * @brief Adds or replaces one complete in-memory section.
 *
 * @param[in] sectionName
 * Empty for the default section or a valid named section.
 *
 * @param[in] sectionValue
 * Valid option-value pairs copied into memory.
 *
 * @throws std::invalid_argument
 * If the section, an option, or a value cannot be represented safely.
 */
void XWalkConfig::setSection(stringview sectionName, const configsection& sectionValue)
{
    if (!sectionName.empty())
    {
        validateSectionName(sectionName);
    }
    for (const auto& option : sectionValue)
    {
        validateOptionName(option.first);
        validateValue(option.second);
    }
    const mutexlock lock(mutexObject);
    sectionsValue[string(sectionName)] = sectionValue;
}

/**
 * @brief Returns the owned configuration-file path as a string.
 *
 * @return
 * Platform-native path representation copied into an owned string.
 */
string XWalkConfig::filePath() const
{
    const mutexlock lock(mutexObject);
    return filePathValue.string();
}

} /* namespace xwalk::hal */
