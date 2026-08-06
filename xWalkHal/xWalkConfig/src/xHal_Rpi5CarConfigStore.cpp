/******************************************************************************
 * @file        xHal_Rpi5CarConfigStore.cpp
 * @brief       Implements configuration retrieval and persistence.
 *
 * @details
 * Parses Robot HAT key-value entries, preserves unrelated content, and commits
 * updates through a same-directory replacement file.
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

#include "xHal_Rpi5CarConfigStore.h"

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
 * @brief Reads all configuration lines without newline characters.
 *
 * @return
 * Lines in their original order.
 *
 * @throws std::runtime_error
 * If the configuration file cannot be opened or completely read.
 */
stringvector XWalkConfigStore::readFileLines(const filesystempath& filePath)
{
    inputfilestream configurationFile(filePath);
    const hal::boolean openNotMatched =
        static_cast<hal::boolean>(
            !configurationFile.is_open());
    if (openNotMatched)
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration file open failed");
    }

    stringvector lines;
    string line;
    const hal::boolean processingLoopRequested{true};
    while (processingLoopRequested)
    {
        const hal::boolean readFileLineSucceeded =
            static_cast<hal::boolean>(
                readFileLine(configurationFile, line));
        if (readFileLineSucceeded == false)
        {
            break;
        }
        lines.push_back(line);
    }
    const hal::boolean streamReadFailed =
        static_cast<hal::boolean>(
            configurationFile.bad());
    if (streamReadFailed)
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration file read failed");
    }
    return lines;
}

/**
 * @brief Appends one configuration file and its relative includes.
 *
 * @param[in] filePath Configuration file resolved by its owning file.
 * @param[in,out] activePaths Canonical paths used to detect active recursion.
 * @param[out] lines Flattened non-include lines in declaration order.
 * @param[in] depth Current include depth in the range zero through eight.
 * @throws std::runtime_error If an include is invalid, cyclic, too deep, missing, or unreadable.
 */
void XWalkConfigStore::appendConfigurationLines(const filesystempath& filePath,
    stringvector& activePaths, stringvector& lines, uint32 depth)
{
    constexpr uint32 maximumIncludeDepth{8U};
    if (depth > maximumIncludeDepth)
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration include depth exceeds eight files");
    }

    const filesystempath normalizedPath = filePath.lexically_normal();
    const string activePath = normalizedPath.string();
    const hal::boolean activePathsActivePathDifferent =
        static_cast<hal::boolean>(
            std::find(activePaths.begin(), activePaths.end(), activePath) != activePaths.end());
    if (activePathsActivePathDifferent)
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration include cycle detected");
    }
    const hal::boolean regularFileNotMatched =
        static_cast<hal::boolean>(
            !isRegularFile(normalizedPath));
    if (regularFileNotMatched)
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration include is not a regular file");
    }

    activePaths.push_back(activePath);
    const stringvector fileLines = readFileLines(normalizedPath);
    for (const string& line : fileLines)
    {
        const size separator = line.find('=');
        const hal::boolean separatorTrimLineInvalid =
            static_cast<hal::boolean>(
                (separator == string::npos) ||
            (trim(stringview(line).substr(0U, separator)) != "include"));
        if (separatorTrimLineInvalid)
        {
            lines.push_back(line);
            continue;
        }

        const filesystempath includePath(trim(stringview(line).substr(separator + 1U)));
        const filesystempath relativePath = includePath.lexically_normal();
        const string relativeText = relativePath.generic_string();
        const hal::boolean pathInvalid =
            static_cast<hal::boolean>(
                relativeText.empty() || includePath.is_absolute() ||
            (relativeText == "..") || (relativeText.rfind("../", 0U) == 0U) ||
            (relativePath.extension() != ".conf"));
        if (pathInvalid)
        {
            XHAL_THROW_RUNTIME_ERROR("Configuration include path is invalid");
        }
        appendConfigurationLines(normalizedPath.parent_path() / relativePath,
            activePaths, lines, depth + 1U);
    }
    activePaths.pop_back();
}

/**
 * @brief Reads the primary configuration and expands relative include directives.
 *
 * @return Lines in include-expansion order.
 * @throws std::runtime_error If the primary file or any include cannot be read safely.
 */
stringvector XWalkConfigStore::readLines() const
{
    stringvector activePaths{};
    stringvector lines{};
    appendConfigurationLines(filePathValue, activePaths, lines, 0U);
    return lines;
}

/**
 * @brief Replaces the configuration file with the supplied lines.
 *
 * @param[in] lines
 * Complete ordered file content without newline characters.
 *
 * @post
 * A successful call leaves the configuration path containing `lines`.
 *
 * @throws std::runtime_error
 * If temporary-file creation, writing, closing, or replacement fails.
 */
void XWalkConfigStore::writeLines(const stringvector& lines) const
{
    filesystempath replacementPath = filePathValue;
    replacementPath += XHAL_RPI5CAR_CONFIG_REPLACEMENT_SUFFIX;

    outputfilestream replacementFile(replacementPath, FILE_OPEN_WRITE_TRUNCATE);
    const hal::boolean replacementFileUnavailable =
        static_cast<hal::boolean>(
            !replacementFile.is_open());
    if (replacementFileUnavailable)
    {
        XHAL_THROW_RUNTIME_ERROR("Configuration replacement file creation failed");
    }

    for (const string& line : lines)
    {
        replacementFile << line << '\n';
    }
    replacementFile.close();
    const hal::boolean replacementWriteFailed =
        static_cast<hal::boolean>(
            replacementFile.fail());
    if (replacementWriteFailed)
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

    const filesystempermissions originalPermissions = originalStatus.permissions();
    errorcode permissionError;
    replaceFilesystemPermissions(replacementPath, originalPermissions, permissionError);
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
 * @brief Retrieves the last stored value associated with a key.
 *
 * @details
 * Comment lines beginning with `#` and malformed lines without `=` are ignored.
 * ASCII spaces are removed from a matched value for Robot HAT compatibility.
 *
 * @param[in] name
 * Non-empty key without leading or trailing whitespace that must not contain
 * `=`, carriage return, or line feed.
 *
 * @param[in] defaultValue
 * Value returned when no matching key exists.
 *
 * @return
 * The last matching normalized value, or an owned copy of `defaultValue`.
 *
 * @throws std::invalid_argument
 * If `name` is not a valid configuration key.
 *
 * @throws std::runtime_error
 * If the configuration file cannot be created, opened, or completely read.
 */
string XWalkConfigStore::get(stringview name, stringview defaultValue) const
{
    validateName(name);
    const mutexlock lock(mutexObject);
    ensureFileExists();
    const stringvector lines = readLines();
    string result(defaultValue);

    for (const string& line : lines)
    {
        const hal::boolean lineInvalid =
            static_cast<hal::boolean>(
                line.empty() || (line.front() == '#'));
        if (lineInvalid)
        {
            continue;
        }

        const size separator = line.find('=');
        const hal::boolean targetOptionMatched =
            static_cast<hal::boolean>(
                (separator != string::npos) && (trim(stringview(line).substr(0U, separator)) == name));
        if (targetOptionMatched)
        {
            result = trim(stringview(line).substr(separator + 1U));
            result.erase(std::remove(result.begin(), result.end(), ' '), result.end());
        }
    }
    return result;
}

/**
 * @brief Updates every matching entry or appends a new configuration entry.
 *
 * @param[in] name
 * Non-empty key without leading or trailing whitespace that must not contain
 * `=`, carriage return, or line feed.
 *
 * @param[in] value
 * Single-line value written after the key.
 *
 * @post
 * Existing matching entries contain `value`, or one new entry has been appended.
 *
 * @throws std::invalid_argument
 * If `name` or `value` cannot be represented safely in the file format.
 *
 * @throws std::runtime_error
 * If the configuration file cannot be read or replaced.
 */
void XWalkConfigStore::set(stringview name, stringview value)
{
    validateName(name);
    validateValue(value);
    const mutexlock lock(mutexObject);
    ensureFileExists();
    stringvector lines = readFileLines(filePathValue);
    boolean found = false;

    for (string& line : lines)
    {
        const hal::boolean ignorableLine =
            static_cast<hal::boolean>(
                line.empty() || (line.front() == '#'));
        if (ignorableLine)
        {
            continue;
        }

        const size separator = line.find('=');
        const hal::boolean targetOptionMatched =
            static_cast<hal::boolean>(
                (separator != string::npos) && (trim(stringview(line).substr(0U, separator)) == name));
        if (targetOptionMatched)
        {
            line = string(name) + " = " + string(value);
            found = true;
        }
    }

    if (!found)
    {
        lines.push_back(string(name) + " = " + string(value));
        lines.emplace_back();
    }
    writeLines(lines);
}

/**
 * @brief Returns the owned configuration-file path as a string.
 *
 * @return
 * Platform-native path representation copied into an owned string.
 */
string XWalkConfigStore::filePath() const
{
    return filePathValue.string();
}

} /* namespace xwalk::hal */
