/******************************************************************************
 * @file        xHal_Rpi5CarConfigLifecycle.cpp
 * @brief       Implements xWalk configuration validation and lifecycle behavior.
 *
 * @details
 * Validates section-aware text, creates missing configuration files, and loads
 * the initial memory image without invoking shell-based permission utilities.
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
 * @brief Validates a named section.
 *
 * @param[in] sectionName
 * Non-empty trimmed section name without brackets or line terminators.
 *
 * @throws std::invalid_argument
 * If the name cannot be represented unambiguously.
 */
void XWalkConfig::validateSectionName(stringview sectionName)
{
    if (sectionName.empty() || (trim(sectionName) != sectionName) ||
        (sectionName.find_first_of("[]\r\n") != stringview::npos))
    {
        XHAL_THROW_INVALID_ARGUMENT("Configuration section name is invalid");
    }
}

/**
 * @brief Validates a configuration option name.
 *
 * @param[in] optionName
 * Non-empty trimmed name without `=`, brackets, or line terminators.
 *
 * @throws std::invalid_argument
 * If the name cannot be represented unambiguously.
 */
void XWalkConfig::validateOptionName(stringview optionName)
{
    if (optionName.empty() || (trim(optionName) != optionName) ||
        (optionName.find_first_of("=[]\r\n") != stringview::npos))
    {
        XHAL_THROW_INVALID_ARGUMENT("Configuration option name is invalid");
    }
}

/**
 * @brief Validates a single-line configuration value.
 *
 * @param[in] value
 * Value that must not contain carriage return or line feed.
 *
 * @throws std::invalid_argument
 * If the value spans more than one line.
 */
void XWalkConfig::validateValue(stringview value)
{
    if (value.find_first_of("\r\n") != stringview::npos)
    {
        XHAL_THROW_INVALID_ARGUMENT("Configuration value must be single-line");
    }
}

/**
 * @brief Removes leading and trailing ASCII spaces and horizontal tabs.
 *
 * @param[in] text
 * Character sequence to normalize.
 *
 * @return
 * Owned normalized text.
 */
string XWalkConfig::trim(stringview text)
{
    const size first = text.find_first_not_of(" \t");
    if (first == stringview::npos)
    {
        return {};
    }

    const size last = text.find_last_not_of(" \t");
    return string(text.substr(first, (last - first) + 1U));
}

/**
 * @brief Extracts and validates a section header.
 *
 * @param[in] line
 * Trimmed line beginning with an opening bracket.
 *
 * @return
 * Validated section name without brackets.
 *
 * @throws std::runtime_error
 * If the line is not a complete unambiguous section header.
 */
string XWalkConfig::parseSectionHeader(stringview line)
{
    if ((line.size() < 3U) || (line.front() != '[') || (line.back() != ']'))
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration section header is malformed");
    }

    const string sectionName = trim(line.substr(1U, line.size() - 2U));
    validateSectionName(sectionName);
    return sectionName;
}

/**
 * @brief Creates the parent directory and initial file when absent.
 *
 * @param[in] description
 * Optional text written as `# ` comment lines only when creating the file.
 *
 * @post
 * The configured path identifies a regular file.
 *
 * @throws std::runtime_error
 * If the path is invalid or initial content cannot be written.
 *
 * @throws filesystemerror
 * If filesystem inspection or parent-directory creation fails.
 */
void XWalkConfig::ensureFileExists(stringview description) const
{
    if (filesystemEntryExists(filePathValue))
    {
        if (!isRegularFile(filePathValue))
        {
            XHAL_THROW_RUNTIME_ERROR("Configuration path is not a regular file");
        }
        return;
    }

    const filesystempath parentPath = filePathValue.parent_path();
    if (!parentPath.empty())
    {
        static_cast<void>(createDirectories(parentPath));
    }

    outputfilestream configurationFile(filePathValue, FILE_OPEN_WRITE_TRUNCATE);
    if (!configurationFile.is_open())
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration file creation failed");
    }

    size lineStart = 0U;
    while (lineStart < description.size())
    {
        const size lineEnd = description.find('\n', lineStart);
        const size lineLength = (lineEnd == stringview::npos)
            ? (description.size() - lineStart)
            : (lineEnd - lineStart);
        string descriptionLine(description.substr(lineStart, lineLength));
        if (!descriptionLine.empty() && (descriptionLine.back() == '\r'))
        {
            descriptionLine.pop_back();
        }
        configurationFile << XHAL_RPI5CAR_CONFIG_COMMENT_PREFIX << descriptionLine << '\n';
        if (lineEnd == stringview::npos)
        {
            break;
        }
        lineStart = lineEnd + 1U;
    }
    if (!description.empty())
    {
        configurationFile << '\n';
    }

    configurationFile.close();
    if (configurationFile.fail())
    {
        XHAL_THROW_RUNTIME_ERROR("Initial configuration write failed");
    }
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs and loads a section-aware configuration file.
 *
 * @param[in] filePath
 * Non-empty filesystem path copied into the object.
 *
 * @param[in] description
 * Optional multiline description used only when a new file is created.
 *
 * @post
 * The file exists and its current sections are loaded into memory.
 *
 * @throws std::invalid_argument
 * If `filePath` is empty.
 *
 * @throws filesystemerror
 * If filesystem inspection or parent-directory creation fails.
 *
 * @throws std::runtime_error
 * If file creation, reading, or configuration parsing fails.
 */
XWalkConfig::XWalkConfig(stringview filePath, stringview description):
    filePathValue(string(filePath))
{
    if (filePath.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("Configuration file path must not be empty");
    }
    ensureFileExists(description);
    sectionsValue = parseSections(readLines());
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the object without deleting or implicitly writing its file.
 */
XWalkConfig::~XWalkConfig() = default;

} /* namespace xwalk::hal */
