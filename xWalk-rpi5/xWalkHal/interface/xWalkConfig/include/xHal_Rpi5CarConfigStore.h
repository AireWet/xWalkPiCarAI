/******************************************************************************
 * @file        xHal_Rpi5CarConfigStore.h
 * @brief       Declares the xWalk file-backed configuration store.
 *
 * @details
 * Defines validated string-key persistence for calibration and runtime settings
 * while keeping filesystem ownership outside hardware driver classes.
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

#ifndef XHAL_RPI5CAR_CONFIG_STORE_H
#define XHAL_RPI5CAR_CONFIG_STORE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

#include <filesystem>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkConfigStore
     * @brief Persists named string values in a human-readable configuration file.
     *
     * @details
     * Creates a missing parent directory and file, preserves comments and unrelated
     * entries, and serializes access made through one store instance. Each setting
     * uses the `name = value` format inherited from Robot HAT `filedb.py`.
     *
     * @note
     * Separate instances that address the same file require external synchronization.
     */
    class XWalkConfigStore
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Owned path identifying the configuration file for this store. */
            filesystempath filePathValue;

            /**
             * @brief Protects file operations performed through this store instance.
             *
             * @note
             * Mutable so read-only retrieval can serialize access without changing
             * the logical configuration-store identity.
             */
            mutable mutexhandle mutexObject;

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Validates a configuration key.
             *
             * @param[in] name
             * Non-empty key without leading or trailing whitespace that must not
             * contain `=`, carriage return, or line feed.
             *
             * @throws std::invalid_argument
             * If the key cannot be represented unambiguously in the file format.
             */
            static void validateName(stringview name);

            /**
             * @brief Validates a configuration value.
             *
             * @param[in] value
             * Value that must not contain carriage return or line feed.
             *
             * @throws std::invalid_argument
             * If the value spans more than one line.
             */
            static void validateValue(stringview value);

            /**
             * @brief Removes leading and trailing ASCII spaces and horizontal tabs.
             *
             * @param[in] text
             * Character sequence to normalize.
             *
             * @return
             * Owned normalized text.
             */
            static string trim(stringview text);

            /**
             * @brief Reads one configuration file without expanding includes.
             *
             * @param[in] filePath
             * Readable regular configuration file.
             *
             * @return
             * Lines in their original order.
             *
             * @throws std::runtime_error
             * If the file cannot be opened or completely read.
             */
            static stringvector readFileLines(const filesystempath& filePath);

            /**
             * @brief Appends one configuration file and its relative includes.
             *
             * @param[in] filePath
             * Configuration file resolved by its owning file.
             *
             * @param[in,out] activePaths
             * Canonical path strings used to reject an active include cycle.
             *
             * @param[out] lines
             * Flattened non-include lines in declaration order.
             *
             * @param[in] depth
             * Current include depth in the range zero through eight.
             *
             * @throws std::runtime_error
             * If an include is invalid, cyclic, too deep, missing, or unreadable.
             */
            static void appendConfigurationLines(const filesystempath& filePath,
                                                 stringvector& activePaths,
                                                 stringvector& lines,
                                                 uint32 depth);

            /**
             * @brief Reads all configuration lines without newline characters.
             *
             * @return
             * Lines in include-expansion order. Each `include = relative/path.conf`
             * directive is replaced by the referenced file's lines.
             *
             * @throws std::runtime_error
             * If the configuration file cannot be opened or completely read.
             */
            stringvector readLines() const;

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
            void writeLines(const stringvector& lines) const;

            /**
             * @brief Creates the parent directory and initial configuration file when absent.
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
            void ensureFileExists() const;

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

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
            explicit XWalkConfigStore(stringview filePath);

            /** @brief Destroys the store without deleting its persistent configuration file. */
            ~XWalkConfigStore();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction because the store contains a mutex. */
            XWalkConfigStore(XWalkConfigStore&&) = delete;
            /** @brief Disables copying because the store contains a mutex. */
            XWalkConfigStore(const XWalkConfigStore&) = delete;
            /** @brief Disables move assignment because the store contains a mutex. */
            XWalkConfigStore& operator=(XWalkConfigStore&&) = delete;
            /** @brief Disables copy assignment because the store contains a mutex. */
            XWalkConfigStore& operator=(const XWalkConfigStore&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Retrieves the last stored value associated with a key.
             *
             * @details
             * Comment lines beginning with `#` and malformed lines without `=` are
             * ignored. ASCII spaces are removed from a matched value for compatibility
             * with Robot HAT `filedb.py`.
             *
             * @param[in] name
             * Non-empty key without leading or trailing whitespace that must not
             * contain `=`, carriage return, or line feed.
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
            string get(stringview name, stringview defaultValue = {}) const;

            /**
             * @brief Updates every matching entry or appends a new configuration entry.
             *
             * @param[in] name
             * Non-empty key without leading or trailing whitespace that must not
             * contain `=`, carriage return, or line feed.
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
            void set(stringview name, stringview value);

            /**
             * @brief Returns the owned configuration-file path as a string.
             *
             * @return
             * Platform-native path representation copied into an owned string.
             */
            string filePath() const;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_CONFIG_STORE_H */
