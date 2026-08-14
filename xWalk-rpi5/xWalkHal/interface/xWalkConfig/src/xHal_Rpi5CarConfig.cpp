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
#include "xHal_Rpi5CarTrace.h"

#include <filesystem>
#include <fstream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

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
configsections XWalkConfig::parseSections(const stringvector &lines) {
  configsections parsedSections;
  string currentSection;
  parsedSections[currentSection] = {};

  for (const string &originalLine : lines) {
    const string line = trim(originalLine);
    const hal::boolean lineInvalid =
        static_cast<hal::boolean>(line.empty() || (line.front() == '#'));
    if (lineInvalid) {
      continue;
    }
    const hal::boolean lineMatched =
        static_cast<hal::boolean>(line.front() == '[');
    if (lineMatched) {
      currentSection = parseSectionHeader(line);
      parsedSections[currentSection] = {};
      continue;
    }

    const size separator = line.find('=');
    if (separator == string::npos) {
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
void XWalkConfig::appendMissingOptions(stringvector &outputLines,
                                       configsections &remainingSections,
                                       const string &sectionName) {
  const auto sectionIterator = remainingSections.find(sectionName);
  const hal::boolean sectionIteratorRemainingSectionsMatched =
      static_cast<hal::boolean>(sectionIterator == remainingSections.end());
  if (sectionIteratorRemainingSectionsMatched) {
    return;
  }

  for (const auto &option : sectionIterator->second) {
    outputLines.push_back(option.first +
                          XHAL_RPI5CAR_CONFIG_ASSIGNMENT_SEPARATOR +
                          option.second);
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
stringvector XWalkConfig::mergeLines(const stringvector &originalLines) const {
  configsections remainingSections = sectionsValue;
  stringvector outputLines;
  string currentSection;

  for (const string &originalLine : originalLines) {
    const string line = trim(originalLine);
    const hal::boolean sectionHeaderFound =
        static_cast<hal::boolean>(!line.empty() && (line.front() == '['));
    if (sectionHeaderFound) {
      appendMissingOptions(outputLines, remainingSections, currentSection);
      currentSection = parseSectionHeader(line);
      outputLines.push_back(originalLine);
      continue;
    }

    const size separator = line.find('=');
    const auto sectionIterator = remainingSections.find(currentSection);
    const hal::boolean knownSectionOption = static_cast<hal::boolean>(
        !line.empty() && (line.front() != '#') && (separator != string::npos) &&
        (sectionIterator != remainingSections.end()));
    if (knownSectionOption) {
      const string optionName = trim(stringview(line).substr(0U, separator));
      const auto optionIterator = sectionIterator->second.find(optionName);
      const hal::boolean optionIteratorSectionIteratorSecondDifferent =
          static_cast<hal::boolean>(optionIterator !=
                                    sectionIterator->second.end());
      if (optionIteratorSectionIteratorSecondDifferent) {
        outputLines.push_back(optionName +
                              XHAL_RPI5CAR_CONFIG_ASSIGNMENT_SEPARATOR +
                              optionIterator->second);
        sectionIterator->second.erase(optionIterator);
        continue;
      }
    }
    outputLines.push_back(originalLine);
  }

  appendMissingOptions(outputLines, remainingSections, currentSection);
  for (const auto &sectionEntry : remainingSections) {
    const hal::boolean trailingBlankLine = static_cast<hal::boolean>(
        !outputLines.empty() && !outputLines.back().empty());
    if (trailingBlankLine) {
      outputLines.emplace_back();
    }
    const hal::boolean firstAvailable =
        static_cast<hal::boolean>(!sectionEntry.first.empty());
    if (firstAvailable) {
      outputLines.push_back("[" + sectionEntry.first + "]");
    }
    for (const auto &option : sectionEntry.second) {
      outputLines.push_back(option.first +
                            XHAL_RPI5CAR_CONFIG_ASSIGNMENT_SEPARATOR +
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
stringvector XWalkConfig::readLines() const {
  inputfilestream configurationFile(filePathValue);
  const hal::boolean openNotMatched =
      static_cast<hal::boolean>(!configurationFile.is_open());
  if (openNotMatched) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Configuration file open failed");
  }

  stringvector lines;
  string line;
  const hal::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const hal::boolean readFileLineSucceeded =
        static_cast<hal::boolean>(readFileLine(configurationFile, line));
    if (readFileLineSucceeded == false) {
      break;
    }
    lines.push_back(line);
  }
  const hal::boolean streamReadFailed =
      static_cast<hal::boolean>(configurationFile.bad());
  if (streamReadFailed) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Configuration file read failed");
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
void XWalkConfig::writeLines(const stringvector &lines) const {
  filesystempath replacementPath = filePathValue;
  replacementPath += XHAL_RPI5CAR_CONFIG_REPLACEMENT_SUFFIX;

  outputfilestream replacementFile(replacementPath, FILE_OPEN_WRITE_TRUNCATE);
  const hal::boolean replacementFileUnavailable =
      static_cast<hal::boolean>(!replacementFile.is_open());
  if (replacementFileUnavailable) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Configuration replacement file creation failed");
  }
  for (const string &line : lines) {
    replacementFile << line << '\n';
  }
  replacementFile.close();
  const hal::boolean replacementWriteFailed =
      static_cast<hal::boolean>(replacementFile.fail());
  if (replacementWriteFailed) {
    errorcode removeError;
    static_cast<void>(removeFilesystemEntry(replacementPath, removeError));
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Configuration replacement file write failed");
  }

  errorcode statusError;
  const filesystemstatus originalStatus =
      filesystemStatus(filePathValue, statusError);
  if (statusError) {
    errorcode removeError;
    static_cast<void>(removeFilesystemEntry(replacementPath, removeError));
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Configuration permission inspection failed");
  }

  errorcode permissionError;
  replaceFilesystemPermissions(replacementPath, originalStatus.permissions(),
                               permissionError);
  if (permissionError) {
    errorcode removeError;
    static_cast<void>(removeFilesystemEntry(replacementPath, removeError));
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Configuration replacement permission copy failed");
  }

  errorcode renameError;
  renameFilesystemEntry(replacementPath, filePathValue, renameError);
  if (renameError) {
    errorcode removeError;
    static_cast<void>(removeFilesystemEntry(replacementPath, removeError));
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Configuration file replacement failed");
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
configsections XWalkConfig::read() {
  const mutexlock lock(mutexObject);
  ensureFileExists({});
  sectionsValue = parseSections(readLines());
  const string ownedFilePath = filePathValue.string();
  XWALK_HAL_TRACE_UID1(RPI .101, "Configuration reloaded from %s",
                       ownedFilePath.c_str());
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
void XWalkConfig::write() {
  const mutexlock lock(mutexObject);
  ensureFileExists({});
  writeLines(mergeLines(readLines()));
  const string ownedFilePath = filePathValue.string();
  XWALK_HAL_TRACE_UID1(RPI .102, "Configuration persisted to %s",
                       ownedFilePath.c_str());
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
string XWalkConfig::get(stringview sectionName, stringview optionName,
                        stringview defaultValue) {
  const hal::boolean sectionNameAvailable =
      static_cast<hal::boolean>(!sectionName.empty());
  if (sectionNameAvailable) {
    validateSectionName(sectionName);
  }
  validateOptionName(optionName);
  validateValue(defaultValue);
  const mutexlock lock(mutexObject);

  const auto sectionIterator = sectionsValue.find(string(sectionName));
  const hal::boolean sectionIteratorSectionsMatched =
      static_cast<hal::boolean>(sectionIterator == sectionsValue.end());
  if (sectionIteratorSectionsMatched) {
    XWALK_HAL_ERROR(XWALK_RANGE, "Configuration section is not loaded");
  }
  auto optionIterator = sectionIterator->second.find(string(optionName));
  const hal::boolean optionIteratorSectionIteratorSecondMatched =
      static_cast<hal::boolean>(optionIterator ==
                                sectionIterator->second.end());
  if (optionIteratorSectionIteratorSecondMatched) {
    optionIterator = sectionIterator->second
                         .emplace(string(optionName), string(defaultValue))
                         .first;
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
void XWalkConfig::set(stringview sectionName, stringview optionName,
                      stringview value) {
  const hal::boolean sectionNameProvided =
      static_cast<hal::boolean>(!sectionName.empty());
  if (sectionNameProvided) {
    validateSectionName(sectionName);
  }
  validateOptionName(optionName);
  validateValue(value);
  const mutexlock lock(mutexObject);

  const auto sectionIterator = sectionsValue.find(string(sectionName));
  const hal::boolean sectionMissing =
      static_cast<hal::boolean>(sectionIterator == sectionsValue.end());
  if (sectionMissing) {
    XWALK_HAL_ERROR(XWALK_RANGE, "Configuration section is not loaded");
  }
  sectionIterator->second[string(optionName)] = string(value);
  const string ownedSectionName(sectionName);
  const string ownedOptionName(optionName);
  XWALK_HAL_TRACE_UID2(RPI .103, "Configuration value updated: [%s] %s",
                       ownedSectionName.c_str(), ownedOptionName.c_str());
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
configsection XWalkConfig::section(stringview sectionName) const {
  const hal::boolean sectionNameProvided =
      static_cast<hal::boolean>(!sectionName.empty());
  if (sectionNameProvided) {
    validateSectionName(sectionName);
  }
  const mutexlock lock(mutexObject);
  const auto sectionIterator = sectionsValue.find(string(sectionName));
  const hal::boolean sectionMissing =
      static_cast<hal::boolean>(sectionIterator == sectionsValue.end());
  if (sectionMissing) {
    XWALK_HAL_ERROR(XWALK_RANGE, "Configuration section is not loaded");
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
void XWalkConfig::setSection(stringview sectionName,
                             const configsection &sectionValue) {
  const hal::boolean sectionNameProvided =
      static_cast<hal::boolean>(!sectionName.empty());
  if (sectionNameProvided) {
    validateSectionName(sectionName);
  }
  for (const auto &option : sectionValue) {
    validateOptionName(option.first);
    validateValue(option.second);
  }
  const mutexlock lock(mutexObject);
  sectionsValue[string(sectionName)] = sectionValue;
  const string ownedSectionName(sectionName);
  XWALK_HAL_TRACE_UID1(RPI .104, "Configuration section updated: [%s]",
                       ownedSectionName.c_str());
}

/**
 * @brief Returns the owned configuration-file path as a string.
 *
 * @return
 * Platform-native path representation copied into an owned string.
 */
string XWalkConfig::filePath() const {
  const mutexlock lock(mutexObject);
  return filePathValue.string();
}

} /* namespace xwalk::hal */
