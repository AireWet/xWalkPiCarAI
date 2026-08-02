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
stringvector XWalkConfigStore::readLines() const
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
        if (line.empty() || (line.front() == '#'))
        {
            continue;
        }

        const size separator = line.find('=');
        if ((separator != string::npos) && (trim(stringview(line).substr(0U, separator)) == name))
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
    stringvector lines = readLines();
    boolean found = false;

    for (string& line : lines)
    {
        if (line.empty() || (line.front() == '#'))
        {
            continue;
        }

        const size separator = line.find('=');
        if ((separator != string::npos) && (trim(stringview(line).substr(0U, separator)) == name))
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
