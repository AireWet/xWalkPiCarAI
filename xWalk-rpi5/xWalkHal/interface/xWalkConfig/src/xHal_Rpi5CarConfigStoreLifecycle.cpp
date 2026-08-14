/******************************************************************************
 * @file        xHal_Rpi5CarConfigStoreLifecycle.cpp
 * @brief       Implements configuration-store validation and lifecycle
 *behavior.
 *
 * @details
 * Validates keys and values, binds a store to its owned file path, and creates
 * missing directories and initial configuration content.
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
 * @brief Validates a configuration key.
 *
 * @param[in] name
 * Non-empty key without leading or trailing whitespace that must not contain
 * `=`, carriage return, or line feed.
 *
 * @throws std::invalid_argument
 * If the key cannot be represented unambiguously in the file format.
 */
void XWalkConfigStore::validateName(stringview name) {
  const hal::boolean nameTrimFindFirstOfInvalid = static_cast<hal::boolean>(
      name.empty() || (trim(name) != name) ||
      (name.find_first_of("=\r\n") != stringview::npos));
  if (nameTrimFindFirstOfInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Configuration name is invalid");
  }
}

/**
 * @brief Validates a configuration value.
 *
 * @param[in] value
 * Value that must not contain carriage return or line feed.
 *
 * @throws std::invalid_argument
 * If the value spans more than one line.
 */
void XWalkConfigStore::validateValue(stringview value) {
  const hal::boolean valueFindFirstOfRDifferent = static_cast<hal::boolean>(
      value.find_first_of("\r\n") != stringview::npos);
  if (valueFindFirstOfRDifferent) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Configuration value must be single-line");
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
string XWalkConfigStore::trim(stringview text) {
  const size first = text.find_first_not_of(" \t");
  if (first == stringview::npos) {
    return {};
  }

  const size last = text.find_last_not_of(" \t");
  return string(text.substr(first, (last - first) + 1U));
}

/**
 * @brief Creates the parent directory and initial configuration file when
 * absent.
 *
 * @post
 * The configured path identifies a regular file after successful completion.
 *
 * @throws filesystemerror
 * If filesystem inspection or directory creation fails.
 *
 * @throws std::runtime_error
 * If the path identifies a non-regular file or initial file creation fails.
 */
void XWalkConfigStore::ensureFileExists() const {
  const hal::boolean configurationFileExists =
      static_cast<hal::boolean>(filesystemEntryExists(filePathValue));
  if (configurationFileExists) {
    const hal::boolean regularFileNotMatched =
        static_cast<hal::boolean>(!isRegularFile(filePathValue));
    if (regularFileNotMatched) {
      XWALK_HAL_ERROR(XWALK_RUNTIME,
                      "Configuration path is not a regular file");
    }
    return;
  }

  const filesystempath parentPath = filePathValue.parent_path();
  const hal::boolean parentPathAvailable =
      static_cast<hal::boolean>(!parentPath.empty());
  if (parentPathAvailable) {
    static_cast<void>(createDirectories(parentPath));
  }

  outputfilestream configurationFile(filePathValue, FILE_OPEN_WRITE_TRUNCATE);
  const hal::boolean openNotMatched =
      static_cast<hal::boolean>(!configurationFile.is_open());
  if (openNotMatched) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Configuration file creation failed");
  }
  configurationFile << "# robot-hat config and calibration value of robots\n\n";
  configurationFile.close();
  const hal::boolean configurationWriteFailed =
      static_cast<hal::boolean>(configurationFile.fail());
  if (configurationWriteFailed) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Initial configuration write failed");
  }
  const string ownedFilePath = filePathValue.string();
  XWALK_HAL_TRACE_UID1(RPI .112, "Configuration-store file created: %s",
                       ownedFilePath.c_str());
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a store bound to one configuration file.
 *
 * @param[in] filePath
 * Non-empty filesystem path copied into the store.
 *
 * @post
 * Missing parent directories and the configuration file have been created.
 *
 * @throws std::invalid_argument
 * If `filePath` is empty.
 *
 * @throws filesystemerror
 * If filesystem inspection or directory creation fails.
 *
 * @throws std::runtime_error
 * If the path is not a regular file or initial file creation fails.
 */
XWalkConfigStore::XWalkConfigStore(stringview filePath)
    : filePathValue(string(filePath)) {
  const hal::boolean filePathEmpty =
      static_cast<hal::boolean>(filePath.empty());
  if (filePathEmpty) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Configuration file path must not be empty");
  }
  ensureFileExists();
  const string ownedFilePath = filePathValue.string();
  XWALK_HAL_TRACE_UID1(RPI .105, "Configuration store loaded from %s",
                       ownedFilePath.c_str());
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the store without deleting its persistent configuration file.
 */
XWalkConfigStore::~XWalkConfigStore() = default;

} /* namespace xwalk::hal */
